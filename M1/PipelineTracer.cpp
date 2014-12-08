#include <gst/gst.h>
#include "gst_utility.hpp"
#include "PipelineTracer.hpp"

PipelineTracer::PipelineTracer(GstElement* pPipeline)
	// Reference the pipeline in the ctor
	: m_pPipeline(GST_ELEMENT(gst_object_ref(pPipeline)))
	, m_SourcePads()
	, m_IELVector()
{
	// Iterate through all the elements in the pipeline. We don't do this recursively
	// because at the moment we don't really care about or want to deal with the innards
	// of bins.
	GValue vElement = G_VALUE_INIT;
	GstIterator *iter = gst_bin_iterate_elements(GST_BIN(pPipeline));
	while (gst_iterator_next(iter, &vElement) == GST_ITERATOR_OK) {
		GstElement* element = GST_ELEMENT(g_value_get_object(&vElement));
		
		// If this element is a video or audio source, let's track its source jitter.
		if (ElementIsSource(element))
		{
			GstIterator *iterPads = gst_element_iterate_src_pads(element);
			GValue vPad = G_VALUE_INIT;
			while (gst_iterator_next(iterPads, &vPad) == GST_ITERATOR_OK)
			{
				GstPad* pad = GST_PAD(g_value_get_object(&vPad));
				GstCaps* caps = gst_pad_get_pad_template_caps(pad);
				
				if (gst_caps_get_size(caps) > 0)
				{
					const gchar* caps_name = gst_structure_get_name(gst_caps_get_structure(caps, 0));
					if ((g_strcmp0(caps_name, "video/x-raw") == 0) || (g_strcmp0(caps_name, "audio/x-raw") == 0))
					{
						// Create an entry in our vector
						m_SourcePads.push_back(new SourcePadJitterEntry(pad));
						
						// Add a pad probe for buffers and buffer lists
						gst_pad_add_probe(pad, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticSrcProbe, this, NULL);
					}
				}

				gst_caps_unref(caps);
			}
			gst_iterator_free(iterPads);
		}
		
		// If it's a transform, we want to track its intra-element latency (from sink
		// pad(s) to source pad(s)).
		else if (ElementIsTransform(element))
		{
			if (    ((gst_element_count_sink_pads(element) == 1) && (gst_element_count_src_pads(element) == 1))
			     || (g_strcmp0(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(element)), "GstTee") == 0)
			   )
			{
				// There's only one sink and one source, OR it's a tee element, which has exactly
				// 1 sink, but we only track the latency from the sink to the FIRST source.
				
				// Create an entry in our vector
				GstPad* sink = gst_element_get_first_sink_pad(element);
				GstPad* src = gst_element_get_first_src_pad(element);
				m_IELVector.push_back(new IntraElementLatencyEntry(sink, src));

				// Add pad probes for buffers and buffer lists
				gst_pad_add_probe(sink, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSinkProbe, this, NULL);
				gst_pad_add_probe(src, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSrcProbe, this, NULL);
			}
			else if (g_strcmp0(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(element)), "GstRtpBin") == 0)
			{
				// rtpbin is a special case. We don't really care a lick about the rtcp sinks and
				// sources, because they're only used for timestamping and synchronization, not
				// passed through. The general cases are for:
				//  - send_rtp_sink_%u (request) -> send_rtp_src_%u (sometimes), and
				//  - recv_rtp_sink_%u (request) -> recv_rtp_src_%u_%u_%u (sometimes)
				// The additional difficulty lies in the sometimes pads, which aren't always
				// created by the time we get here.
				
				// Our strategy is this: if source pads are present, we assume all the sometimes
				// pads are already linked, so do our best to create pairs. If no source pads are
				// present, then hook up a "pad-added" signal handler and try to create pairs then.
				if (gst_element_count_src_pads(element) > 0)
				{
					// hook up pairs, looking up sinks by their sources
					GstIterator *iter = gst_element_iterate_src_pads(element);
					GValue vPad = G_VALUE_INIT;
					while (gst_iterator_next(iter, &vPad) == GST_ITERATOR_OK)
					{
						GstPad* src = GST_PAD(g_value_get_object(&vPad));
						GstPad* sink = FindRtpBinSinkBySrc(src);
						if (sink != NULL)
						{
							m_IELVector.push_back(new IntraElementLatencyEntry(sink, src));

							// Add pad probes for buffers and buffer lists
							gst_pad_add_probe(sink, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSinkProbe, this, NULL);
							gst_pad_add_probe(src, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSrcProbe, this, NULL);
						}
					}
					gst_iterator_free(iter);
				}
				else
				{
					// deferring pair hookup until pad-added
					g_signal_connect(element, "pad-added", G_CALLBACK(StaticRtpBinNewPad), this);
				}
			}
			else
			{
				const gchar* element_name = gst_element_get_name(element);
				g_message("Found transform %s (type %s) with %u sink(s) and %u src(s) -- unable to handle.", element_name, G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(element)), gst_element_count_sink_pads(element), gst_element_count_src_pads(element));
				g_free(const_cast<gchar*>(element_name));
			}
		}
	}
	gst_iterator_free(iter);
}

PipelineTracer::~PipelineTracer()
{
	// Release all tracked source pads
	while (m_SourcePads.size() > 0)
	{
		delete m_SourcePads.back();
		m_SourcePads.pop_back();
	}
	
	// Release pipeline reference we took in ctor
	gst_object_ref(m_pPipeline);
}

GstPad* PipelineTracer::FindRtpBinSinkBySrc(const GstPad* pSrcPad)
{
	const gchar* pad_name = gst_pad_get_name(pSrcPad);
	GstPad* ret;
	
	// send_rtcp_src_%u -- no corresponding sink.
	if (strncmp(pad_name, "send_rtcp_src_", sizeof("send_rtcp_src_") - 1) == 0)
	{
		ret = NULL;
	}
	
	// send_rtp_src_%u -- sink is named send_rtp_sink_%u
	else if (strncmp(pad_name, "send_rtp_src_", sizeof("send_rtp_src_") - 1) == 0)
	{
		guint64 idx = g_ascii_strtoull(&pad_name[sizeof("send_rtp_src_") - 1], NULL, 10);
		gchar* sink_pad_name = g_strdup_printf("send_rtp_sink_%lu", idx);
		GstElement* element = gst_pad_get_parent_element(const_cast<GstPad*>(pSrcPad));
		ret = gst_element_find_sink_pad_by_name(element, sink_pad_name);
		gst_object_unref(GST_OBJECT(element));
		g_free(sink_pad_name);
	}
	
	// recv_rtp_src_%u_%u_%u -- sink is named recv_rtp_sink_%u
	else if (strncmp(pad_name, "recv_rtp_src_", sizeof("recv_rtp_src_") - 1) == 0)
	{
		guint64 idx = g_ascii_strtoull(&pad_name[sizeof("recv_rtp_src_") - 1], NULL, 10);
		gchar* sink_pad_name = g_strdup_printf("recv_rtp_sink_%lu", idx);
		GstElement* element = gst_pad_get_parent_element(const_cast<GstPad*>(pSrcPad));
		ret = gst_element_find_sink_pad_by_name(element, sink_pad_name);
		gst_object_unref(GST_OBJECT(element));
		g_free(sink_pad_name);
	}
	
	else
	{
		g_warning("Unexpected src pad name \"%s\" in rtpbin! Could not find corresponding sink.", pad_name);
		ret = NULL;
	}
	
	g_free(const_cast<gchar*>(pad_name));
	return ret;
}

void PipelineTracer::StaticRtpBinNewPad(GstElement* element, GstPad* pad, gpointer user_data)
{
	(reinterpret_cast<PipelineTracer*>(user_data))->RtpBinNewPad(element, pad);
}

GstPadProbeReturn PipelineTracer::StaticSrcProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	return (reinterpret_cast<PipelineTracer*>(user_data))->SrcProbe(pad, info);
}

GstPadProbeReturn PipelineTracer::StaticXfrmSinkProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	return (reinterpret_cast<PipelineTracer*>(user_data))->XfrmSinkProbe(pad, info);
}

GstPadProbeReturn PipelineTracer::StaticXfrmSrcProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	return (reinterpret_cast<PipelineTracer*>(user_data))->XfrmSrcProbe(pad, info);
}

const gchar* PipelineTracer::GetPadParentElementName(const GstPad* pPad)
{
	GstElement* pParentElement = gst_pad_get_parent_element(const_cast<GstPad*>(pPad));
	const gchar* ret = gst_element_get_name(pParentElement);
	gst_object_unref(pParentElement);
	return ret;
}

void PipelineTracer::RtpBinNewPad(GstElement* element, GstPad* pad)
{
	if (    (g_strcmp0(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(element)), "GstRtpBin") == 0)
	     && (GST_PAD_IS_SRC(pad))
	   )
	{
		GstPad* sink = FindRtpBinSinkBySrc(pad);
		if (sink != NULL)
		{
			m_IELVector.push_back(new IntraElementLatencyEntry(sink, pad));

			// Add pad probes for buffers and buffer lists
			gst_pad_add_probe(sink, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSinkProbe, this, NULL);
			gst_pad_add_probe(pad, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSrcProbe, this, NULL);
		}
	}
}

GstPadProbeReturn PipelineTracer::SrcProbe(GstPad* pad, GstPadProbeInfo* info)
{
	// Go looking to see if this pad is one we care about for tracking source pad jitter
	SourcePadJitterEntry *pJE = NULL;
	for (JEVector::const_iterator ci = m_SourcePads.cbegin(); ci != m_SourcePads.cend(); ++ci)
	{
		if (pad == (*ci)->Pad())
		{
			pJE = *ci;
			break;
		}
	}
	if (pJE != NULL)
	{
		// Get the current system clock time
		GstClock* clock = gst_system_clock_obtain();
		GstClockTime time = gst_clock_get_time(clock);
		g_object_unref(clock);
		
		if (pJE->LastSourceTimestamp() != 0)
		{
			uint64_t deltaUs = (time - pJE->LastSourceTimestamp()) / 1000;
			// TODO: Record jitter here...
			g_message("%s.%s jitter = %llu us", pJE->ElementName(), pJE->PadName(), deltaUs);
		}
		
		pJE->LastSourceTimestamp(time);
	}

	return GST_PAD_PROBE_OK;
}

GstPadProbeReturn PipelineTracer::XfrmSinkProbe(GstPad* pad, GstPadProbeInfo* info)
{
	// Go looking to see if this pad is one we care about for tracking intra-element latency
	IntraElementLatencyEntry *pIEL = NULL;
	for (IELVector::const_iterator ci = m_IELVector.cbegin(); ci != m_IELVector.cend(); ++ci)
	{
		if (pad == (*ci)->SinkPad())
		{
			pIEL = *ci;
			break;
		}
	}
	if (pIEL != NULL)
	{
		// Get the current system clock time
		GstClock* clock = gst_system_clock_obtain();
		GstClockTime time = gst_clock_get_time(clock);
		g_object_unref(clock);
		
		// For sink pads, we just record the last timestamp.
		pIEL->LastSinkTimestamp(time);
	}
	
	return GST_PAD_PROBE_OK;
}

GstPadProbeReturn PipelineTracer::XfrmSrcProbe(GstPad* pad, GstPadProbeInfo* info)
{
	// Go looking to see if this pad is one we care about for tracking intra-element latency
	IntraElementLatencyEntry *pIEL = NULL;
	for (IELVector::const_iterator ci = m_IELVector.cbegin(); ci != m_IELVector.cend(); ++ci)
	{
		if (pad == (*ci)->SourcePad())
		{
			pIEL = *ci;
			break;
		}
	}
	if (pIEL != NULL)
	{
		// Get the current system clock time
		GstClock* clock = gst_system_clock_obtain();
		GstClockTime time = gst_clock_get_time(clock);
		g_object_unref(clock);
		
		if (pIEL->LastSinkTimestamp() != 0)
		{
			uint64_t deltaUs = (time - pIEL->LastSinkTimestamp()) / 1000;
			// TODO: Record latency here...
			g_message("%s.%s->%s jitter = %llu us", pIEL->ElementName(), pIEL->SinkPadName(), pIEL->SourcePadName(), deltaUs);
			
			// A transform element may turn N sink buffer (list)(s) into M src buffer (list)(s),
			// where N is not necessarily == M. However, what we're interested in is the time when
			// the element is actually doing the work of transforming, which is the delta between
			// the last sink data and the first source data. We ensure that subsequent source data
			// items are not recorded by marking the last sink timestamp as zero, thus skipping
			// this block until subsequent sink data is received.
			pIEL->LastSinkTimestamp(0);
		}
	}
	
	return GST_PAD_PROBE_OK;
}

PipelineTracer::SourcePadJitterEntry::SourcePadJitterEntry(const GstPad* pSourcePad)
	: m_pPad(GST_PAD(gst_object_ref(GST_OBJECT(pSourcePad))))
	, m_ElementName(GetPadParentElementName(pSourcePad))
	, m_PadName(gst_pad_get_name(pSourcePad))
	, m_LastSourceTimestamp(0)
{
}
		
PipelineTracer::SourcePadJitterEntry::~SourcePadJitterEntry()
{
	g_free(const_cast<gchar*>(m_PadName));
	g_free(const_cast<gchar*>(m_ElementName));
	gst_object_unref(GST_OBJECT(m_pPad));
}

PipelineTracer::IntraElementLatencyEntry::IntraElementLatencyEntry(const GstPad* pSinkPad, const GstPad* pSourcePad)
	: m_pSinkPad(GST_PAD(gst_object_ref(GST_OBJECT(pSinkPad))))
	, m_pSourcePad(GST_PAD(gst_object_ref(GST_OBJECT(pSourcePad))))
	, m_ElementName(GetPadParentElementName(pSinkPad))
	, m_SinkPadName(gst_pad_get_name(pSinkPad))
	, m_SourcePadName(gst_pad_get_name(pSourcePad))
	, m_LastSinkTimestamp(0)
{
	// Sanity-check that these pads have the same parent.
	GstElement* pSinkParentElement = gst_pad_get_parent_element(const_cast<GstPad*>(pSinkPad));
	GstElement* pSourceParentElement = gst_pad_get_parent_element(const_cast<GstPad*>(pSourcePad));
	bool bSameParent = (pSinkParentElement == pSourceParentElement);
	gst_object_unref(pSinkParentElement);
	gst_object_unref(pSourceParentElement);
	g_assert(bSameParent);
}
		
PipelineTracer::IntraElementLatencyEntry::~IntraElementLatencyEntry()
{
	g_free(const_cast<gchar*>(m_SourcePadName));
	g_free(const_cast<gchar*>(m_SinkPadName));
	g_free(const_cast<gchar*>(m_ElementName));
	gst_object_unref(GST_OBJECT(m_pSourcePad));
	gst_object_unref(GST_OBJECT(m_pSinkPad));
}