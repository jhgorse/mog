///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file ReceiverPipeline.cpp
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
/// @brief This file defines the functions of the ReceiverPipeline class.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>              // for assert
#include "gst_utility.hpp"      // for gst_element_find_sink_pad_by_name
#include "ReceiverPipeline.hpp" // for class declaration

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A note about this pipeline: this pipeline utilizes sometimes pads for the "downstream" links
/// from rtpbin. So the sometimes pads get linked up when the bin first receives data from an SSRC.
/// However, if the sender gets restarted and a different SSRC is chosen, the bin will make new
/// ghost pads and inner bin elements, but the downstream sometimes pads will not be linked, and
/// the stream will stop. This needs to be dealt with in a proper app either by:
///   (1) Sensibly choosing the SSRC so it's known a priori and not randomly chosen, or
///   (2) Dynamically dealing with pad linkages and unlinkages in callback functions.
///////////////////////////////////////////////////////////////////////////////////////////////////
const char ReceiverPipeline::PIPELINE_STRING[] =
	"   rtpbin name=rtpbin latency=10"
	"   udpsrc port=10000"
	" ! application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,sprop-parameter-sets=\"Z3oAH7y0AoAt2AiAAosKgJiWgEeMGVA\\=\\,aM48gA\\=\\=\",payload=96"
	" ! rtpbin.recv_rtp_sink_0"
	"   udpsrc port=10001"
	" ! application/x-rtcp"
	" ! rtpbin.recv_rtcp_sink_0"
	"   udpsrc port=10002"
	" ! application/x-rtp,media=audio,clock-rate=48000,encoding-name=L16,encoding-params=1,channels=1,payload=96"
	" ! rtpbin.recv_rtp_sink_1"
	"   udpsrc port=10003"
	" ! application/x-rtcp"
	" ! rtpbin.recv_rtcp_sink_1"
;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor.
///
/// Parse the launch string to construct the pipeline; obtain some references; and install a
/// callback function for when pads are added to rtpbin.
///////////////////////////////////////////////////////////////////////////////////////////////////
ReceiverPipeline::ReceiverPipeline(IReceiverNotifySink* pNotifySink)
	: PipelineBase(gst_parse_launch(PIPELINE_STRING, NULL))
	, m_pNotifySink(pNotifySink)
	, m_ActiveSsrcs()
	, m_pRtpBin(gst_bin_get_by_name(GST_BIN(Pipeline()), "rtpbin"))
{
	assert(m_pRtpBin != NULL);
	g_signal_connect(m_pRtpBin, "on-bye-ssrc",       G_CALLBACK(StaticOnRtpBinByeSsrc),       this);
	g_signal_connect(m_pRtpBin, "on-bye-timeout",    G_CALLBACK(StaticOnRtpBinByeTimeout),    this);
	g_signal_connect(m_pRtpBin, "on-npt-stop",       G_CALLBACK(StaticOnRtpBinNptStop),       this);
	g_signal_connect(m_pRtpBin, "pad-added",         G_CALLBACK(StaticOnRtpBinPadAdded),      this);
	g_signal_connect(m_pRtpBin, "on-sender-timeout", G_CALLBACK(StaticOnRtpBinSenderTimeout), this);
	g_signal_connect(m_pRtpBin, "on-ssrc-active",    G_CALLBACK(StaticOnRtpBinSsrcActive),    this);
	g_signal_connect(m_pRtpBin, "on-timeout",        G_CALLBACK(StaticOnRtpBinTimeout),       this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Destructor.
///
/// Release references obtained in constructor.
///////////////////////////////////////////////////////////////////////////////////////////////////
ReceiverPipeline::~ReceiverPipeline()
{
	gst_object_unref(m_pRtpBin);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// (Static) callback for when an SSRC leaves (says bye)
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::StaticOnRtpBinByeSsrc(GstElement* element, guint session, guint ssrc, gpointer data)
{
	((ReceiverPipeline *)data)->OnRtpBinSsrcDeactivate(((session == 0) ? IReceiverNotifySink::SSRC_TYPE_VIDEO : IReceiverNotifySink::SSRC_TYPE_AUDIO), ssrc, IReceiverNotifySink::SSRC_DEACTIVATE_REASON_BYE);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// (Static) callback for when an SSRC leaves (bye times out)
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::StaticOnRtpBinByeTimeout(GstElement* element, guint session, guint ssrc, gpointer data)
{
	((ReceiverPipeline *)data)->OnRtpBinSsrcDeactivate(((session == 0) ? IReceiverNotifySink::SSRC_TYPE_VIDEO : IReceiverNotifySink::SSRC_TYPE_AUDIO), ssrc, IReceiverNotifySink::SSRC_DEACTIVATE_REASON_TIMEOUT);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// (Static) callback for when an SSRC leaves due to its stop time
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::StaticOnRtpBinNptStop(GstElement* element, guint session, guint ssrc, gpointer data)
{
	((ReceiverPipeline *)data)->OnRtpBinSsrcDeactivate(((session == 0) ? IReceiverNotifySink::SSRC_TYPE_VIDEO : IReceiverNotifySink::SSRC_TYPE_AUDIO), ssrc, IReceiverNotifySink::SSRC_DEACTIVATE_REASON_STOP);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Static callback function for when pads are added to rtpbin. Calls the instance version.
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::StaticOnRtpBinPadAdded(GstElement* element, GstPad* pad, gpointer data)
{
	((ReceiverPipeline *)data)->OnRtpBinPadAdded(element, pad);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// (Static) callback for when an SSRC times out
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::StaticOnRtpBinSenderTimeout(GstElement* element, guint session, guint ssrc, gpointer data)
{
	((ReceiverPipeline *)data)->OnRtpBinSsrcDeactivate(((session == 0) ? IReceiverNotifySink::SSRC_TYPE_VIDEO : IReceiverNotifySink::SSRC_TYPE_AUDIO), ssrc, IReceiverNotifySink::SSRC_DEACTIVATE_REASON_TIMEOUT);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// (Static) callback for when an SSRC becomes active
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::StaticOnRtpBinSsrcActive(GstElement* element, guint session, guint ssrc, gpointer data)
{
	((ReceiverPipeline *)data)->OnRtpBinSsrcActivate(((session == 0) ? IReceiverNotifySink::SSRC_TYPE_VIDEO : IReceiverNotifySink::SSRC_TYPE_AUDIO), ssrc);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// (Static) callback for when an SSRC times out
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::StaticOnRtpBinTimeout(GstElement* element, guint session, guint ssrc, gpointer data)
{
	((ReceiverPipeline *)data)->OnRtpBinSsrcDeactivate(((session == 0) ? IReceiverNotifySink::SSRC_TYPE_VIDEO : IReceiverNotifySink::SSRC_TYPE_AUDIO), ssrc, IReceiverNotifySink::SSRC_DEACTIVATE_REASON_TIMEOUT);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Callback function for when pads are added to rtpbin.
///
/// Because the (sender) RtpBins randomly create new SSRCs, if another entity was stopped and
/// relaunched, the (receiver) rtpbin element will create a new dynamic src pad for the new ssrc.
/// In this callback, we disconnect any existing link to the appropriate payloader and connect the
/// new pad (so there's only one at a time).
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::OnRtpBinPadAdded(GstElement* element, GstPad* pad)
{
	assert(element == m_pRtpBin);
	
	// Check the media member of the caps to see what we've received. Point sink_pad at the
	// sink pad for that media type.
	GstCaps* pad_caps = gst_pad_get_current_caps(pad);
	const gchar* media_type = g_value_get_string(gst_structure_get_value(gst_caps_get_structure(pad_caps, 0), "media"));
	
	if (g_strcmp0(media_type, "audio") == 0)
	{
		g_message("on-pad-added (audio)");
		
		GstElement* depay = gst_element_factory_make("rtpL16depay", NULL);
		assert(depay != NULL);
		
		GstElement* convert = gst_element_factory_make("audioconvert", NULL);
		assert(convert != NULL);
		
		GstElement* audiosink = gst_element_factory_make("osxaudiosink", NULL);
		assert(audiosink != NULL);
		g_object_set(audiosink,
			"enable-last-sample", FALSE,
			"buffer-time", 92880,
			NULL
		);
		
		gst_bin_add_many(GST_BIN(Pipeline()), depay, convert, audiosink, NULL);
		assert (gst_element_link_many(depay, convert, audiosink, NULL));
		
		GstPad* sink = gst_element_get_static_pad(depay, "sink");
		assert(sink != NULL);
		gst_pad_link(pad, sink);
		gst_object_unref(sink);
		
		gst_element_sync_state_with_parent(depay);
		gst_element_sync_state_with_parent(convert);
		gst_element_sync_state_with_parent(audiosink);
	}
	else if (g_strcmp0(media_type, "video") == 0)
	{
		g_message("on-pad-added (video)");
		
		GstElement* depay = gst_element_factory_make("rtph264depay", NULL);
		assert(depay != NULL);
		
		GstElement* filter = gst_element_factory_make("capsfilter", NULL);
		gst_util_set_object_arg(G_OBJECT(filter), "caps", "video/x-h264,stream-format=avc,alignment=au");
		assert(filter != NULL);
		
		GstElement* decoder = gst_element_factory_make("avdec_h264", NULL);
		assert(decoder != NULL);
		
		GstElement* convert = gst_element_factory_make("videoconvert", NULL);
		assert(convert != NULL);
		
		GstElement* videosink = gst_element_factory_make("osxvideosink", NULL);
		assert(videosink != NULL);
		g_object_set(videosink,
			"enable-last-sample", FALSE,
			"sync", FALSE,
			NULL
		);

		gst_bin_add_many(GST_BIN(Pipeline()), depay, filter, decoder, convert, videosink, NULL);
		assert (gst_element_link_many(depay, filter, decoder, convert, videosink, NULL));
		
		GstPad* sink = gst_element_get_static_pad(depay, "sink");
		assert(sink != NULL);
		assert (gst_pad_link(pad, sink) == GST_PAD_LINK_OK);
		gst_object_unref(sink);
		
		gst_element_sync_state_with_parent(depay);
		gst_element_sync_state_with_parent(filter);
		gst_element_sync_state_with_parent(decoder);
		gst_element_sync_state_with_parent(convert);
		gst_element_sync_state_with_parent(videosink);
	}
	else
	{
		const gchar* pad_name = gst_pad_get_name(pad);
		const gchar* caps_string = gst_caps_to_string(pad_caps);
		g_message("Pad \"%s\" with caps \"%s\" added to rtpbin! Not a known media type!", pad_name, caps_string);
		g_free(const_cast<gchar*>(caps_string));
		g_free(const_cast<gchar*>(pad_name));
	}
	
	gst_caps_unref(pad_caps);
}


/// (Instance) callback for when an SSRC becomes active
void ReceiverPipeline::OnRtpBinSsrcActivate(ReceiverPipeline::IReceiverNotifySink::SsrcType type, unsigned int ssrc)
{
	if (m_ActiveSsrcs.find(ssrc) == m_ActiveSsrcs.end())
	{
		m_ActiveSsrcs.insert(ssrc);
		if (m_pNotifySink != NULL)
		{
			m_pNotifySink->OnSsrcActivate(*this, type, ssrc);
		}
	}
}
	
	
/// (Instance) callback for when an SSRC becomes inactive
void ReceiverPipeline::OnRtpBinSsrcDeactivate(ReceiverPipeline::IReceiverNotifySink::SsrcType type, unsigned int ssrc, ReceiverPipeline::IReceiverNotifySink::SsrcDeactivateReason reason)
{
	std::set<unsigned int>::iterator it;
	if ((it = m_ActiveSsrcs.find(ssrc)) != m_ActiveSsrcs.end())
	{
		m_ActiveSsrcs.erase(it);
		m_pNotifySink->OnSsrcDeactivate(*this, type, ssrc, reason);
	}
}