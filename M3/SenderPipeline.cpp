///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file SenderPipeline.cpp
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
/// @brief This file defines the functions of the SenderPipeline class.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>            // for assert
#include "SenderPipeline.hpp" // for class declaration


const char SenderPipeline::PIPELINE_STRING[] =
	"   rtpbin name=rtpbin latency=10"
	"   avfvideosrc name=videosrc do-timestamp=true device-index=1"
	" ! video/x-raw, width=(int)1920, height=(int)1080, format=(string)UYVY, framerate=(fraction)10000000/333333"
	" ! videoconvert"
	" ! timeoverlay font-desc=\"Sans Bold 36\" valignment=\"bottom\" halignment=\"right\""
	" ! tee name=t"
	"   t. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream silent=true ! videoconvert ! osxvideosink enable-last-sample=false sync=false"
	"   t."
	" ! x264enc bitrate=5000 speed-preset=ultrafast tune=zerolatency"
	" ! rtph264pay"
	" ! rtpbin.send_rtp_sink_0"
	"   osxaudiosrc do-timestamp=true latency-time=23220 buffer-time=92880 device=48"
	" ! audio/x-raw, format=(string)S32LE, layout=(string)interleaved, rate=(int)44100, channels=(int)1"
	" ! audioconvert"
	" ! rtpL16pay buffer-list=true"
	" ! rtpbin.send_rtp_sink_1"
	"   rtpbin.send_rtp_src_0  ! udpsink name=vsink  enable-last-sample=false sync=false async=false"
	"   rtpbin.send_rtcp_src_0 ! udpsink name=vcsink enable-last-sample=false sync=false"
	"   rtpbin.send_rtp_src_1  ! udpsink name=asink  enable-last-sample=false sync=false async=false"
	"   rtpbin.send_rtcp_src_1 ! udpsink name=acsink enable-last-sample=false sync=false"
;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::SenderPipeline()
///
/// Constructor. Create the pipeline from the static string representation.
///////////////////////////////////////////////////////////////////////////////////////////////////
SenderPipeline::SenderPipeline()
	: PipelineBase(gst_parse_launch(PIPELINE_STRING, NULL))
	, m_pVideoRtpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "vsink"))
	, m_pVideoRtcpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "vcsink"))
	, m_pAudioRtpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "asink"))
	, m_pAudioRtcpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "acsink"))
{
	assert(m_pVideoRtpSink != NULL);
	assert(m_pVideoRtcpSink != NULL);
	assert(m_pAudioRtpSink != NULL);
	assert(m_pAudioRtcpSink != NULL);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::~SenderPipeline()
///
/// Destructor. Destroy the pipeline.
///////////////////////////////////////////////////////////////////////////////////////////////////
SenderPipeline::~SenderPipeline()
{
	// Unref everything we ref'ed before
	gst_object_unref(m_pAudioRtcpSink);
	gst_object_unref(m_pAudioRtpSink);
	gst_object_unref(m_pVideoRtcpSink);
	gst_object_unref(m_pVideoRtpSink);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::SetDestination()
///
/// Set the sender pipeline's destination address or hostname.
///////////////////////////////////////////////////////////////////////////////////////////////////
void SenderPipeline::SetDestination(const char* destination)
{
	const gchar* clients_value = g_strdup_printf("%s:10000", destination);
	g_object_set(m_pVideoRtpSink, "clients", clients_value, NULL);
	g_free(const_cast<gchar*>(clients_value));
	
	clients_value = g_strdup_printf("%s:10001", destination);
	g_object_set(m_pVideoRtcpSink, "clients", clients_value, NULL);
	g_free(const_cast<gchar*>(clients_value));
	
	clients_value = g_strdup_printf("%s:10002", destination);
	g_object_set(m_pAudioRtpSink, "clients", clients_value, NULL);
	g_free(const_cast<gchar*>(clients_value));
	
	clients_value = g_strdup_printf("%s:10003", destination);
	g_object_set(m_pAudioRtcpSink, "clients", clients_value, NULL);
	g_free(const_cast<gchar*>(clients_value));
}