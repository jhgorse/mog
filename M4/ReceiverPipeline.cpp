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
	"   udpsrc name=vsrc"
	" ! application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96"
	" ! rtpbin.recv_rtp_sink_0"
	"   udpsrc name=vcsrc"
	" ! application/x-rtcp"
	" ! rtpbin.recv_rtcp_sink_0"
	"   udpsrc name=asrc"
	" ! application/x-rtp,media=audio,clock-rate=44100,encoding-name=L16,encoding-params=1,channels=1,payload=96"
	" ! rtpbin.recv_rtp_sink_1"
	"   udpsrc name=acsrc"
	" ! application/x-rtcp"
	" ! rtpbin.recv_rtcp_sink_1"
	"   rtpbin."
	" ! capsfilter name=vfilter caps=\"application/x-rtp,media=video\""
	" ! rtph264depay"
	" ! video/x-h264,stream-format=avc,alignment=au"
	" ! avdec_h264"
	" ! videoconvert"
	" ! osxvideosink name=vsink enable-last-sample=false sync=false"
	"   rtpbin."
	" ! rtpL16depay"
	" ! audioconvert"
	" ! osxaudiosink enable-last-sample=false buffer-time=92880"
;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor.
///
/// Parse the launch string to construct the pipeline; obtain some references; and install a
/// callback function for when pads are added to rtpbin.
///////////////////////////////////////////////////////////////////////////////////////////////////
ReceiverPipeline::ReceiverPipeline(uint16_t basePort, const char* pictureParameters, void* pWindowHandle, IReceiverNotifySink* pNotifySink)
	: PipelineBase(gst_parse_launch(PIPELINE_STRING, NULL))
	, m_pNotifySink(pNotifySink)
	, m_ActiveSsrcs()
	, m_pRtpBin(gst_bin_get_by_name(GST_BIN(Pipeline()), "rtpbin"))
{
	assert(m_pRtpBin != NULL);
	g_signal_connect(m_pRtpBin, "on-bye-ssrc",       G_CALLBACK(StaticOnRtpBinByeSsrc),       this);
	g_signal_connect(m_pRtpBin, "on-bye-timeout",    G_CALLBACK(StaticOnRtpBinByeTimeout),    this);
	g_signal_connect(m_pRtpBin, "on-npt-stop",       G_CALLBACK(StaticOnRtpBinNptStop),       this);
	g_signal_connect(m_pRtpBin, "on-sender-timeout", G_CALLBACK(StaticOnRtpBinSenderTimeout), this);
	g_signal_connect(m_pRtpBin, "on-ssrc-active",    G_CALLBACK(StaticOnRtpBinSsrcActive),    this);
	g_signal_connect(m_pRtpBin, "on-timeout",        G_CALLBACK(StaticOnRtpBinTimeout),       this);
	
	// Set all the udpsrc port properties
	GstElement* e = gst_bin_get_by_name(GST_BIN(Pipeline()), "vsrc");
	assert(e != NULL);
	g_object_set(e, "port", basePort, NULL);
	gst_object_unref(e);
	
	e = gst_bin_get_by_name(GST_BIN(Pipeline()), "vcsrc");
	assert(e != NULL);
	g_object_set(e, "port", basePort + 1, NULL);
	gst_object_unref(e);
	
	e = gst_bin_get_by_name(GST_BIN(Pipeline()), "asrc");
	assert(e != NULL);
	g_object_set(e, "port", basePort + 2, NULL);
	gst_object_unref(e);
	
	e = gst_bin_get_by_name(GST_BIN(Pipeline()), "acsrc");
	assert(e != NULL);
	g_object_set(e, "port", basePort + 3, NULL);
	gst_object_unref(e);
	
	// Set vfilter caps sprop-parameter-sets = pictureParameters
	e = gst_bin_get_by_name(GST_BIN(Pipeline()), "vfilter");
	assert(e != NULL);
	gchar* capsString = new gchar[std::strlen("application/x-rtp,media=video,sprop-parameter-sets=\"\"") + std::strlen(pictureParameters) + 1];
	std::sprintf(capsString, "application/x-rtp,media=video,sprop-parameter-sets=\"%s\"", pictureParameters);
	gst_util_set_object_arg(G_OBJECT(e), "caps", capsString);
	delete[] capsString;
	gst_object_unref(e);
	
	// Set vsink to display on pWindowHandle
	e = gst_bin_get_by_name(GST_BIN(Pipeline()), "vsink");
	assert(e != NULL);
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(e), reinterpret_cast<guintptr>(pWindowHandle));
	gst_object_unref(e);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Destructor.
///
/// Release references obtained in constructor.
///////////////////////////////////////////////////////////////////////////////////////////////////
ReceiverPipeline::~ReceiverPipeline()
{
	Nullify();
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