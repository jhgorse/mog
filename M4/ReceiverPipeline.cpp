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
#include <cstdio>
#include <gst/video/videooverlay.h>
#include "gst_utility.hpp"      // for gst_element_find_sink_pad_by_name
#include "ReceiverPipeline.hpp" // for class declaration

///////////////////////////////////////////////////////////////////////////////////////////////////
/// These are the mostly "static" parts of the pipeline, represented as a string in
/// gst_parse_launch format.
///////////////////////////////////////////////////////////////////////////////////////////////////
const char ReceiverPipeline::PIPELINE_STRING[] =
	"   rtpbin name=rtpbin latency=10"
	"   udpsrc port=10000"
	" ! application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96"
	" ! rtpbin.recv_rtp_sink_0"
	"   udpsrc port=10001"
	" ! application/x-rtcp"
	" ! rtpbin.recv_rtcp_sink_0"
	"   udpsrc port=10002"
	" ! application/x-rtp,media=audio,clock-rate=44100,encoding-name=L16,encoding-params=1,channels=1,payload=96"
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
/// ReceiverPipeline::ActivateVideoSsrc()
///
/// Activate a video SSRC within the pipeline. This creates the necessary elements to depayload,
/// decode, and display the video data, connecting its videosink to a native window handle.
///
/// @param ssrc  The SSRC to be activated.
///
/// @param pictureParameters  The picture parameters string.
///
/// @param windowHandle  The native window handle.
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::ActivateVideoSsrc(unsigned int ssrc, const char* pictureParameters, void* windowHandle)
{
	// Go looking for a recv rtp src pad with the name corresponding to this ssrc
	char padName[sizeof("recv_rtp_src_0_4294967295_96")];
	std::sprintf(padName, "recv_rtp_src_0_%u_96", ssrc);
	GstPad* ssrcPad = gst_element_find_src_pad_by_name(m_pRtpBin, padName);
	if (ssrcPad != NULL)
	{
		// Hook up a new video display chain here
		GstElement* capsfilter = gst_element_factory_make("capsfilter", NULL);
		GstCaps* caps = gst_caps_from_string("application/x-rtp,sprop-parameter-sets=\"\"");
		GValue valParams = G_VALUE_INIT;
		g_value_init(&valParams, G_TYPE_STRING);
		g_value_set_string(&valParams, pictureParameters);
		gst_structure_set_value(gst_caps_get_structure(caps, 0), "sprop-parameter-sets", &valParams);
	    g_object_set(capsfilter,
	    	"caps", caps,
	    	NULL);
	    gst_caps_unref(caps);
    
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
		gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(videosink), reinterpret_cast<guintptr>(windowHandle));

		gst_bin_add_many(GST_BIN(Pipeline()), capsfilter, depay, filter, decoder, convert, videosink, NULL);
		assert(gst_element_link_many(capsfilter, depay, filter, decoder, convert, videosink, NULL));
		
		GstPad* sink = gst_element_get_static_pad(capsfilter, "sink");
		assert(sink != NULL);
		GstPad *peer = gst_pad_get_peer(ssrcPad);
		gst_pad_unlink(ssrcPad, peer);
		gst_object_unref(peer);
		assert(gst_pad_link(ssrcPad, sink) == GST_PAD_LINK_OK);
		gst_object_unref(sink);
		
		gst_element_sync_state_with_parent(capsfilter);
		gst_element_sync_state_with_parent(depay);
		gst_element_sync_state_with_parent(filter);
		gst_element_sync_state_with_parent(decoder);
		gst_element_sync_state_with_parent(convert);
		gst_element_sync_state_with_parent(videosink);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// ReceiverPipeline::ActivateAudioSsrc()
///
/// Activate an audio SSRC within the pipeline. This creates the necessary elements to depayload,
/// decode, and play back the audio data.
///
/// @param ssrc  The SSRC to be activated.
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::ActivateAudioSsrc(unsigned int ssrc)
{
	// Go looking for a recv rtp src pad with the name corresponding to this ssrc
	char padName[sizeof("recv_rtp_src_1_4294967295_96")];
	std::sprintf(padName, "recv_rtp_src_1_%u_96", ssrc);
	GstPad* ssrcPad = gst_element_find_src_pad_by_name(m_pRtpBin, padName);
	if (ssrcPad != NULL)
	{
		// Hook up rtpbin's src pad to a real sink.
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
		assert(gst_element_link_many(depay, convert, audiosink, NULL));
		
		GstPad* sink = gst_element_get_static_pad(depay, "sink");
		assert(sink != NULL);
		GstPad *peer = gst_pad_get_peer(ssrcPad);
		gst_pad_unlink(ssrcPad, peer);
		gst_object_unref(peer);
		assert(gst_pad_link(ssrcPad, sink) == GST_PAD_LINK_OK);
		gst_object_unref(sink);
		
		gst_element_sync_state_with_parent(depay);
		gst_element_sync_state_with_parent(convert);
		gst_element_sync_state_with_parent(audiosink);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// ReceiverPipeline::DeactivateVideoSsrc()
///
/// Deactivate a video SSRC within the pipeline.
///
/// @param ssrc  The SSRC to be deactivated.
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::DeactivateVideoSsrc(unsigned int ssrc)
{
	// Go looking for a recv rtp src pad with the name corresponding to this ssrc
	char padName[sizeof("recv_rtp_src_0_4294967295_96")];
	std::sprintf(padName, "recv_rtp_src_0_%u_96", ssrc);
	GstPad* ssrcPad = gst_element_find_src_pad_by_name(m_pRtpBin, padName);
	if (ssrcPad != NULL)
	{
		// Connect this pad to a fakesink
		GstElement* fakesink = gst_element_factory_make("fakesink", NULL);
		assert(fakesink != NULL);
		gst_bin_add(GST_BIN(Pipeline()), fakesink);
		
		GstPad* sink = gst_element_get_static_pad(fakesink, "sink");
		assert(sink != NULL);
		GstPad *peer = gst_pad_get_peer(ssrcPad);
		gst_pad_unlink(ssrcPad, peer);
		gst_object_unref(peer);
		assert(gst_pad_link(ssrcPad, sink) == GST_PAD_LINK_OK);
		gst_object_unref(sink);
		
		gst_element_sync_state_with_parent(fakesink);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// ReceiverPipeline::DeactivateAudioSsrc()
///
/// Deactivate an audio SSRC within the pipeline.
///
/// @param ssrc  The SSRC to be deactivated.
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::DeactivateAudioSsrc(unsigned int ssrc)
{
	// Go looking for a recv rtp src pad with the name corresponding to this ssrc
	char padName[sizeof("recv_rtp_src_1_4294967295_96")];
	std::sprintf(padName, "recv_rtp_src_1_%u_96", ssrc);
	GstPad* ssrcPad = gst_element_find_src_pad_by_name(m_pRtpBin, padName);
	if (ssrcPad != NULL)
	{
		// Connect this pad to a fakesink
		GstElement* fakesink = gst_element_factory_make("fakesink", NULL);
		assert(fakesink != NULL);
		gst_bin_add(GST_BIN(Pipeline()), fakesink);
		
		GstPad* sink = gst_element_get_static_pad(fakesink, "sink");
		assert(sink != NULL);
		GstPad *peer = gst_pad_get_peer(ssrcPad);
		gst_pad_unlink(ssrcPad, peer);
		gst_object_unref(peer);
		assert(gst_pad_link(ssrcPad, sink) == GST_PAD_LINK_OK);
		gst_object_unref(sink);
		
		gst_element_sync_state_with_parent(fakesink);
	}
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
	
	if ((g_strcmp0(media_type, "audio") == 0) || (g_strcmp0(media_type, "video") == 0))
	{
		GstElement* fakesink = gst_element_factory_make("fakesink", NULL);
		assert(fakesink != NULL);
		gst_bin_add(GST_BIN(Pipeline()), fakesink);
		
		GstPad* sink = gst_element_get_static_pad(fakesink, "sink");
		assert(sink != NULL);
		assert(gst_pad_link(pad, sink) == GST_PAD_LINK_OK);
		gst_object_unref(sink);
		
		gst_element_sync_state_with_parent(fakesink);
		
		const gchar* pad_name = gst_pad_get_name(pad);
		g_message("Pad \"%s\" added to rtpbin.", pad_name);
		g_free(const_cast<gchar*>(pad_name));
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// ReceiverPipeline::OnRtpBinSsrcActivate
///
/// (Instance) callback for when an SSRC becomes active. Calls the listener (if configured).
///////////////////////////////////////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// ReceiverPipeline::OnRtpBinSsrcActivate
///
/// (Instance) callback for when an SSRC becomes inactive. Calls the listener (if configured).
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::OnRtpBinSsrcDeactivate(ReceiverPipeline::IReceiverNotifySink::SsrcType type, unsigned int ssrc, ReceiverPipeline::IReceiverNotifySink::SsrcDeactivateReason reason)
{
	std::set<unsigned int>::iterator it;
	if ((it = m_ActiveSsrcs.find(ssrc)) != m_ActiveSsrcs.end())
	{
		m_ActiveSsrcs.erase(it);
		m_pNotifySink->OnSsrcDeactivate(*this, type, ssrc, reason);
	}
}