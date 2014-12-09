///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file PipelineTracer.cpp
///
/// Copyright (c) 2014, BoxCast, Inc. All rights reserved.
///
/// This library is free software; you can redistribute it and/or modify it under the terms of the
/// GNU Lesser General Public License as published by the Free Software Foundation; either version
/// 3.0 of the License, or (at your option) any later version.
///
/// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
/// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
/// the GNULesser General Public License for more details.
///
/// You should have received a copy of the GNU Lesser General Public License along with this
/// library; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
/// Boston, MA 02110-1301 USA
///
/// @brief This file defines the functions declared for the PipelineTracer class and its private
/// classes.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <gst/gst.h>          // for GStreamer stuff
#include "gst_utility.hpp"    // for GStreamer utility functions
#include "PipelineTracer.hpp" // for class declaration


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::PipelineTracer()
///
/// The constructor is responsible for finding all of the elements and pads that are relevant for
/// tracking source jitter and intra-element transform latency.
///////////////////////////////////////////////////////////////////////////////////////////////////
PipelineTracer::PipelineTracer(GstElement* pPipeline)
	// Reference the pipeline in the ctor
	: m_pPipeline(GST_ELEMENT(gst_object_ref(pPipeline)))
	, m_SourcePads()
	, m_IELVector()
{
	// Iterate through all the elements in the pipeline. We don't do this recursively because at
	// the moment we don't really care about or want to deal with the innards of bins -- we treat
	// them opaquely.
	GValue vElement = G_VALUE_INIT;
	GstIterator *iter = gst_bin_iterate_elements(GST_BIN(pPipeline));
	while (gst_iterator_next(iter, &vElement) == GST_ITERATOR_OK)
	{
		// Get element pointer from value.
		GstElement* element = GST_ELEMENT(g_value_get_object(&vElement));
		
		// If this is a source element, let's look through its pads to see if it's an audio or
		// video source element.
		if (ElementIsSource(element))
		{
			// Iterate through pads
			GstIterator *iterPads = gst_element_iterate_src_pads(element);
			GValue vPad = G_VALUE_INIT;
			while (gst_iterator_next(iterPads, &vPad) == GST_ITERATOR_OK)
			{
				// Get pad pointer from value and get its caps
				GstPad* pad = GST_PAD(g_value_get_object(&vPad));
				GstCaps* caps = gst_pad_get_pad_template_caps(pad);
				
				// Make sure the caps have actual caps, not just "ANY" or something stupid.
				if (gst_caps_get_size(caps) > 0)
				{
					// Take a look to see if this is a video or audio source based on caps
					const gchar* caps_name = gst_structure_get_name(gst_caps_get_structure(caps, 0));
					if ((g_strcmp0(caps_name, "video/x-raw") == 0) || (g_strcmp0(caps_name, "audio/x-raw") == 0))
					{
						// Create an entry in our vector
						m_SourcePads.push_back(new SourcePadJitterEntry(pad));
						
						// Add a pad probe for buffers and buffer lists
						gst_pad_add_probe(pad, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticSrcProbe, this, NULL);
					} // END if (video or audio source)
				} // END if (caps are real)
				gst_caps_unref(caps);
			} // END while (pad iterator keeps producing)
			gst_iterator_free(iterPads);
		} // END if (element is a source)
		
		// If it's a transform, we want to track its intra-element latency (from sink
		// pad(s) to source pad(s)).
		else if (ElementIsTransform(element))
		{
			// If there's only one sink and one source, OR it's a tee element, which has exactly
			// one sink, but we only track the latency from the sink to the FIRST source, then
			// we track this pad pair.
			if (    ((gst_element_count_sink_pads(element) == 1) && (gst_element_count_src_pads(element) == 1))
			     || (g_strcmp0(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(element)), "GstTee") == 0)
			   )
			{
				// Create an entry in our vector
				GstPad* sink = gst_element_get_first_sink_pad(element);
				GstPad* src = gst_element_get_first_src_pad(element);
				m_IELVector.push_back(new IntraElementLatencyEntry(sink, src));

				// Add pad probes for buffers and buffer lists
				gst_pad_add_probe(sink, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSinkProbe, this, NULL);
				gst_pad_add_probe(src, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSrcProbe, this, NULL);
			} // END if 1 sink and 1 source, or GstTee
			
			// RtpBin is a special case. We don't really care a lick about the rtcp sinks and
			// sources, because they're only used for timestamping and synchronization -- they
			// don't pass data through. The general cases are for:
			//  - send_rtp_sink_%u (request) -> send_rtp_src_%u (sometimes), and
			//  - recv_rtp_sink_%u (request) -> recv_rtp_src_%u_%u_%u (sometimes)
			// The additional difficulty lies in the sometimes pads, which aren't always
			// created by the time we get here.
			else if (g_strcmp0(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(element)), "GstRtpBin") == 0)
			{
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
							// Create an entry in our vector
							m_IELVector.push_back(new IntraElementLatencyEntry(sink, src));

							// Add pad probes for buffers and buffer lists
							gst_pad_add_probe(sink, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSinkProbe, this, NULL);
							gst_pad_add_probe(src, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSrcProbe, this, NULL);
						} // END if (sink)
					} // END while (pad iterator is producing)
					gst_iterator_free(iter);
				} // END if (there are source pads)
				else
				{
					// If there are no source pads, then we defer pair creation until source pads
					// are added. We hook up the pad-added signal handler for this reason.
					g_signal_connect(element, "pad-added", G_CALLBACK(StaticRtpBinNewPad), this);
				} // END else
			} // END else if (GstRtpBin)
			
			// For all other cases, print out a warning message to indicate we didn't know what to
			// do with this transform element.
			else
			{
				const gchar* element_name = gst_element_get_name(element);
				g_warning("Found transform %s (type %s) with %u sink(s) and %u src(s) -- unable to handle.", element_name, G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(element)), gst_element_count_sink_pads(element), gst_element_count_src_pads(element));
				g_free(const_cast<gchar*>(element_name));
			} // END else
		} // END if (element is a transform)
	} // END while (element iterator is producing)
	gst_iterator_free(iter);
} // END PipelineTracer::PipelineTracer()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::~PipelineTracer()
///
/// The only real cleanup to do is release vector entries (for source jitter and latency tracking)
/// and release the pipeline reference.
///
/// It would probably also be a good idea to remove all out pad probes and signal handlers; however
/// the assumption is that the pipeline is going away anyway, so we don't bother to do that.
///////////////////////////////////////////////////////////////////////////////////////////////////
PipelineTracer::~PipelineTracer()
{
	// Release all tracked source pads
	while (m_SourcePads.size() > 0)
	{
		delete m_SourcePads.back();
		m_SourcePads.pop_back();
	}
	
	// Release all tracked latency elements
	while (m_IELVector.size() > 0)
	{
		delete m_IELVector.back();
		m_IELVector.pop_back();
	}
	
	// Release pipeline reference we took in ctor
	gst_object_ref(m_pPipeline);
} // END PipelineTracer::~PipelineTracer()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::FindRtpBinSinkBySrc()
///
/// See the comments above for the strategy about handling intra-element latency measurement in
/// RtpBin elements.
///
/// The general idea here is that we know what the sinks should be named based on the source pad's
/// name. So we do a bit of string manipulation and try to find an appropriately-named pad.
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPad* PipelineTracer::FindRtpBinSinkBySrc(const GstPad* pSrcPad)
{
	const gchar* pad_name = gst_pad_get_name(pSrcPad);
	GstPad* ret;
	
	// send_rtcp_src_%u -- no corresponding sink.
	if (strncmp(pad_name, "send_rtcp_src_", sizeof("send_rtcp_src_") - 1) == 0)
	{
		ret = NULL;
	} // END if (send_rtcp_src_%u)
	
	// send_rtp_src_%u -- sink is named send_rtp_sink_%u
	else if (strncmp(pad_name, "send_rtp_src_", sizeof("send_rtp_src_") - 1) == 0)
	{
		guint64 idx = g_ascii_strtoull(&pad_name[sizeof("send_rtp_src_") - 1], NULL, 10);
		gchar* sink_pad_name = g_strdup_printf("send_rtp_sink_%lu", idx);
		GstElement* element = gst_pad_get_parent_element(const_cast<GstPad*>(pSrcPad));
		ret = gst_element_find_sink_pad_by_name(element, sink_pad_name);
		gst_object_unref(GST_OBJECT(element));
		g_free(sink_pad_name);
	} // END if (send_rtp_src_%u)
	
	// recv_rtp_src_%u_%u_%u -- sink is named recv_rtp_sink_%u
	else if (strncmp(pad_name, "recv_rtp_src_", sizeof("recv_rtp_src_") - 1) == 0)
	{
		guint64 idx = g_ascii_strtoull(&pad_name[sizeof("recv_rtp_src_") - 1], NULL, 10);
		gchar* sink_pad_name = g_strdup_printf("recv_rtp_sink_%lu", idx);
		GstElement* element = gst_pad_get_parent_element(const_cast<GstPad*>(pSrcPad));
		ret = gst_element_find_sink_pad_by_name(element, sink_pad_name);
		gst_object_unref(GST_OBJECT(element));
		g_free(sink_pad_name);
	} // END if (recv_rtp_src_%u_%u_%u)
	
	else
	{
		g_warning("Unexpected src pad name \"%s\" in rtpbin! Could not find corresponding sink.", pad_name);
		ret = NULL;
	} // END else
		
	g_free(const_cast<gchar*>(pad_name));
	return ret;
} // END PipelineTracer::FindRtpBinSinkBySrc()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::StaticRtpBinNewPad()
///
/// Static interface to RtpBinNewPad() (for C callback reasons).
///////////////////////////////////////////////////////////////////////////////////////////////////
void PipelineTracer::StaticRtpBinNewPad(GstElement* element, GstPad* pad, gpointer user_data)
{
	(reinterpret_cast<PipelineTracer*>(user_data))->RtpBinNewPad(element, pad);
} // END PipelineTracer::StaticRtpBinNewPad()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::StaticSrcProbe()
///
/// Static interface to SrcProbe() (for C callback reasons).
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPadProbeReturn PipelineTracer::StaticSrcProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	return (reinterpret_cast<PipelineTracer*>(user_data))->SrcProbe(pad, info);
} // END PipelineTracer::StaticSrcProbe()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::StaticXfrmSinkProbe()
///
/// Static interface to XfrmSinkProbe() (for C callback reasons).
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPadProbeReturn PipelineTracer::StaticXfrmSinkProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	return (reinterpret_cast<PipelineTracer*>(user_data))->XfrmSinkProbe(pad, info);
} // END PipelineTracer::StaticXfrmSinkProbe()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::StaticXfrmSrcProbe()
///
/// Static interface to XfrmSrcProbe() (for C callback reasons).
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPadProbeReturn PipelineTracer::StaticXfrmSrcProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
	return (reinterpret_cast<PipelineTracer*>(user_data))->XfrmSrcProbe(pad, info);
} // END PipelineTracer::StaticXfrmSrcProbe()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::GetPadParentElementName()
///
/// Get a pad's parent element's name. For use in a constructor because gst_pad_get_parent_element
/// increment's the element's reference.
///////////////////////////////////////////////////////////////////////////////////////////////////
const gchar* PipelineTracer::GetPadParentElementName(const GstPad* pPad)
{
	GstElement* pParentElement = gst_pad_get_parent_element(const_cast<GstPad*>(pPad));
	const gchar* ret = gst_element_get_name(pParentElement);
	gst_object_unref(pParentElement);
	return ret;
} // END PipelineTracer::GetPadParentElementName()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::RtpBinNewPad()
///
/// This callback is called when a new pad is created on an RtpBin element; this callback should
/// only be installed when an RtpBin does not have any linked source pads when the constructor is
/// called.
///////////////////////////////////////////////////////////////////////////////////////////////////
void PipelineTracer::RtpBinNewPad(GstElement* element, GstPad* pad)
{
	// Make sure the element is a GstRtpBin and the pad is a source pad; otherwise, we don't care.
	if (    (g_strcmp0(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(element)), "GstRtpBin") == 0)
	     && (GST_PAD_IS_SRC(pad))
	   )
	{
		// Try to find the corresponding sink pad, if possible.
		GstPad* sink = FindRtpBinSinkBySrc(pad);
		if (sink != NULL)
		{
			// Create a new intra-element latency entry on our list
			m_IELVector.push_back(new IntraElementLatencyEntry(sink, pad));

			// Add pad probes for buffers and buffer lists
			gst_pad_add_probe(sink, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSinkProbe, this, NULL);
			gst_pad_add_probe(pad, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), StaticXfrmSrcProbe, this, NULL);
		} // END if (sink)
	} // END if (GstRtpBin and source pad)
} // END PipelineTracer::RtpBinNewPad()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::SrcProbe()
///
/// This function is called by GStreamer when data hits a source pad on a media source element
/// (i.e. audio or video source). We track the jitter of the production of media source data.
///////////////////////////////////////////////////////////////////////////////////////////////////
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
		} // END if (pad matches)
	} // END for each source pad jitter entry
	
	// If we found it, then try to record this entry
	if (pJE != NULL)
	{
		// Get the current system clock time
		GstClock* clock = gst_system_clock_obtain();
		GstClockTime time = gst_clock_get_time(clock);
		g_object_unref(clock);
		
		// If we have a last timestamp, we have jitter to measure.
		if (pJE->LastSourceTimestamp() != 0)
		{
			guint64 deltaUs = (time - pJE->LastSourceTimestamp()) / 1000;
			// TODO: Record jitter here...
			g_message("%s.%s jitter = %llu us", pJE->ElementName(), pJE->PadName(), deltaUs);
		} // END if (last timestamp != 0)
		
		// Mark the last timestamp.
		pJE->LastSourceTimestamp(time);
	} // END if (entry)

	return GST_PAD_PROBE_OK;
} // END PipelineTracer::SrcProbe()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::XfrmSinkProbe()
///
/// This function is called by GStreamer when data hits a sink pad on a transform element. We mark
/// the last sink timestamp for the purpose of comparison with the corresponding source pad
/// timestamp.
///////////////////////////////////////////////////////////////////////////////////////////////////
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
		} // END if (pad matches)
	} // END for each intra-element latency entry
	
	// If we found it, then try to record this entry
	if (pIEL != NULL)
	{
		// Get the current system clock time
		GstClock* clock = gst_system_clock_obtain();
		GstClockTime time = gst_clock_get_time(clock);
		g_object_unref(clock);
		
		// For sink pads, we just record the last timestamp.
		pIEL->LastSinkTimestamp(time);
	} // END if (entry)
	
	return GST_PAD_PROBE_OK;
} // END PipelineTracer::XfrmSinkProbe()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineTracer::XfrmSrcProbe()
///
/// This function is called by GStreamer when data hits a source pad on a transform element. We
/// compare this timestamp with the corresponding sink pad's last timestamp; this is the latency
/// from sink pad to source pad.
///////////////////////////////////////////////////////////////////////////////////////////////////
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
		} // END if (pad matches)
	} // END for each intra-element latency entry
	
	// If we found it, then try to record this entry
	if (pIEL != NULL)
	{
		// Get the current system clock time
		GstClock* clock = gst_system_clock_obtain();
		GstClockTime time = gst_clock_get_time(clock);
		g_object_unref(clock);
		
		// If we have a last timestamp, then we have a latency to measure.
		if (pIEL->LastSinkTimestamp() != 0)
		{
			guint64 deltaUs = (time - pIEL->LastSinkTimestamp()) / 1000;
			// TODO: Record latency here...
			g_message("%s.%s->%s jitter = %llu us", pIEL->ElementName(), pIEL->SinkPadName(), pIEL->SourcePadName(), deltaUs);
			
			// A transform element may turn N sink buffer (list)(s) into M src buffer (list)(s),
			// where N is not necessarily == M. However, what we're interested in is the time when
			// the element is actually doing the work of transforming, which is the delta between
			// the last sink data and the first source data. We ensure that subsequent source data
			// items are not recorded by marking the last sink timestamp as zero, thus skipping
			// this block until subsequent sink data is received.
			pIEL->LastSinkTimestamp(0);
		} // END if (last sink timestamp != 0)
	} // END if (entry)
	
	return GST_PAD_PROBE_OK;
} // END PipelineTracer::XfrmSrcProbe()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SourcePadJitterEntry::SourcePadJitterEntry()
///
/// Constructor for a source pad jitter entry. Nothing surprising here.
///////////////////////////////////////////////////////////////////////////////////////////////////
PipelineTracer::SourcePadJitterEntry::SourcePadJitterEntry(const GstPad* pSourcePad)
	: m_pPad(GST_PAD(gst_object_ref(GST_OBJECT(pSourcePad))))
	, m_ElementName(GetPadParentElementName(pSourcePad))
	, m_PadName(gst_pad_get_name(pSourcePad))
	, m_LastSourceTimestamp(0)
{
} // END SourcePadJitterEntry::SourcePadJitterEntry()()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SourcePadJitterEntry::~SourcePadJitterEntry()
///
/// Destructor for a source pad jitter entry. Free the names we allocated in the constructor, and
/// unref the pad (that was ref'ed in the constructor).
///////////////////////////////////////////////////////////////////////////////////////////////////
PipelineTracer::SourcePadJitterEntry::~SourcePadJitterEntry()
{
	g_free(const_cast<gchar*>(m_PadName));
	g_free(const_cast<gchar*>(m_ElementName));
	gst_object_unref(GST_OBJECT(m_pPad));
} // END SourcePadJitterEntry::~SourcePadJitterEntry()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SourcePadJitterEntry::SourcePadJitterEntry()
///
/// Constructor for an intra-element latency entry. Nothing surprising here.
///////////////////////////////////////////////////////////////////////////////////////////////////
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
} // END IntraElementLatencyEntry::IntraElementLatencyEntry()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// IntraElementLatencyEntry::~IntraElementLatencyEntry()
///
/// Destructor for an intra-element latency entry. Free the names we allocated in the constructor,
/// and unref the pads (that were ref'ed in the constructor).
///////////////////////////////////////////////////////////////////////////////////////////////////
PipelineTracer::IntraElementLatencyEntry::~IntraElementLatencyEntry()
{
	g_free(const_cast<gchar*>(m_SourcePadName));
	g_free(const_cast<gchar*>(m_SinkPadName));
	g_free(const_cast<gchar*>(m_ElementName));
	gst_object_unref(GST_OBJECT(m_pSourcePad));
	gst_object_unref(GST_OBJECT(m_pSinkPad));
} // END IntraElementLatencyEntry::~IntraElementLatencyEntry()
