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
#include <cmath>              // for round
#include <cstring>
#include <gst/video/videooverlay.h>
#include "SenderPipeline.hpp" // for class declaration


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::SenderPipeline()
///
/// Constructor. Create the pipeline from the static string representation.
///////////////////////////////////////////////////////////////////////////////////////////////////
SenderPipeline::SenderPipeline(const char* videoInputName, const char* audioInputName, ISenderParameterNotifySink* pNotifySink)
	: PipelineBase(BuildPipeline(videoInputName, audioInputName))
	, m_pVideoEncoder(gst_bin_get_by_name(GST_BIN(Pipeline()), "venc"))
	, m_pVideoRtpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "vsink"))
	, m_pVideoRtcpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "vcsink"))
	, m_pAudioRtpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "asink"))
	, m_pAudioRtcpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "acsink"))
	, m_vDestinations()
	, m_pNotifySink(pNotifySink)
	, m_pSpropParameterSets(NULL)
	, m_VideoSsrc(0)
	, m_AudioSsrc(0)
{
	assert(m_pVideoEncoder != NULL);
	assert(m_pVideoRtpSink != NULL);
	assert(m_pVideoRtcpSink != NULL);
	assert(m_pAudioRtpSink != NULL);
	assert(m_pAudioRtcpSink != NULL);
	
	// Receive a callback when the caps property of the vsink:sink and asink:sink pads change
	GstPad* pad = gst_element_get_static_pad(m_pVideoRtpSink, "sink");
	assert(pad != NULL);
	g_signal_connect(pad, "notify::caps", G_CALLBACK(StaticPadNotifyCaps), this);
	gst_object_unref(pad);
	
	pad = gst_element_get_static_pad(m_pAudioRtpSink, "sink");
	assert(pad != NULL);
	g_signal_connect(pad, "notify::caps", G_CALLBACK(StaticPadNotifyCaps), this);
	gst_object_unref(pad);
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
	gst_object_unref(m_pVideoEncoder);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::AddDestination()
///
/// Add a destination address or hostname to the sender's pipeline.
///////////////////////////////////////////////////////////////////////////////////////////////////
void SenderPipeline::AddDestination(const char* destination)
{
	m_vDestinations.push_back(new std::string(destination));
	SetDestinations();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::RemoveDestination()
///
/// Remove a destination address or hostname to the sender's pipeline.
///////////////////////////////////////////////////////////////////////////////////////////////////
void SenderPipeline::RemoveDestination(const char* destination)
{
	std::vector<const std::string*>::const_iterator it = m_vDestinations.begin();
	while (it != m_vDestinations.end())
	{
		if ((*it)->compare(destination) == 0)
		{
			m_vDestinations.erase(it);
		}
		else
		{
			++it;
		}
	}
	SetDestinations();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::SetBitrate()
///
/// Set the sender pipeline's video bitrate.
///////////////////////////////////////////////////////////////////////////////////////////////////
void SenderPipeline::SetBitrate(size_t bitrate)
{
	// The x264enc's bitrate property is in kbit/sec, so here we divide by 1024 and round to the
	// nearest integer.
	bitrate = (size_t)round(((double)bitrate) / ((double)1024.0));
	g_object_set(m_pVideoEncoder, "bitrate", bitrate, NULL);
}


void SenderPipeline::SetWindowSink(void* handle)
{
	GstElement* videosink = gst_bin_get_by_name(GST_BIN(Pipeline()), "videosink");
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(videosink), reinterpret_cast<guintptr>(handle));
	gst_object_unref(videosink);
}


GstElement* SenderPipeline::BuildPipeline(const char* videoInputName, const char* audioInputName)
{
	// Create a new pipeline
	GstElement* pipeline = gst_pipeline_new(NULL);
	
	// Create an rtpbin
	GstElement* rtpbin = gst_element_factory_make("rtpbin", "rtpbin");
	g_object_set(G_OBJECT(rtpbin),
		"latency", RTP_BIN_LATENCY_MS,
		NULL);
	
	// Get device index and caps for the selected video input
	int avfDeviceIndex = GetVideoDeviceIndex(videoInputName);
	assert((avfDeviceIndex >= 0) && (avfDeviceIndex < 2147483647));
	const char* avfDeviceCaps = GetVideoDeviceCaps(avfDeviceIndex);
	assert(avfDeviceCaps != NULL);
	
	// Build the video part of the pipeline
	GstElement* videosrc = gst_element_factory_make("avfvideosrc", "videosrc");
	g_object_set(G_OBJECT(videosrc),
		"do-timestamp", TRUE,
		"device-index", avfDeviceIndex,
		NULL);
	
	GstElement* srccapsfilter = gst_element_factory_make("capsfilter", NULL);
	GstCaps* caps = gst_caps_from_string(avfDeviceCaps);
	delete[] avfDeviceCaps;
    g_object_set(srccapsfilter,
    	"caps", caps,
    	NULL);
    gst_caps_unref(caps);
    
	GstElement* videoconvert1 = gst_element_factory_make("videoconvert", NULL);
	
	GstElement* t = gst_element_factory_make("tee", "t");
	
	GstElement* queue = gst_element_factory_make("queue", NULL);
	g_object_set(G_OBJECT(queue),
		"max-size-buffers", 1,
		"max-size-bytes",   0,
		"max-size-time",    0,
		"silent",           TRUE,
		NULL);
	gst_util_set_object_arg(G_OBJECT(queue), "leaky", "downstream");
		
	GstElement* videoconvert2 = gst_element_factory_make("videoconvert", NULL);
	
	GstElement* videosink = gst_element_factory_make("osxvideosink", "videosink");
	g_object_set(G_OBJECT(videosink),
		"enable-last-sample", FALSE,
		"sync",               FALSE,
		NULL);
	
	GstElement* venc = gst_element_factory_make("x264enc", "venc");
	gst_util_set_object_arg(G_OBJECT(venc), "speed-preset", "ultrafast");
	gst_util_set_object_arg(G_OBJECT(venc), "tune", "zerolatency");
	
	GstElement* rtph264pay = gst_element_factory_make("rtph264pay", NULL);
	
	GstElement* vsink = gst_element_factory_make("multiudpsink", "vsink");
	g_object_set(G_OBJECT(vsink),
		"enable-last-sample", FALSE,
		"sync",               FALSE,
		"async",              FALSE,
		NULL);
	
	GstElement* vcsink = gst_element_factory_make("multiudpsink", "vcsink");
	g_object_set(G_OBJECT(vcsink),
		"enable-last-sample", FALSE,
		"sync",               FALSE,
		NULL);
	
	gst_bin_add_many(GST_BIN(pipeline), rtpbin, videosrc, srccapsfilter, videoconvert1, t, queue, videoconvert2, videosink, venc, rtph264pay, vsink, vcsink, NULL);
	assert(gst_element_link_many(videosrc, srccapsfilter, videoconvert1, t, queue, videoconvert2, videosink, NULL));
	assert(gst_element_link_many(t, venc, rtph264pay, NULL));
	assert(gst_element_link_pads(rtph264pay, "src", rtpbin, "send_rtp_sink_0"));
	assert(gst_element_link_pads(rtpbin, "send_rtp_src_0", vsink, "sink"));
	assert(gst_element_link_pads(rtpbin, "send_rtcp_src_0", vcsink, "sink"));
	
	// Get device index and caps for the selected video input
	int audioDeviceIndex = GetAudioDeviceIndex(audioInputName);
	assert((audioDeviceIndex >= 0) && (avfDeviceIndex < 2147483647));
	const char* audioDeviceCaps = GetAudioDeviceCaps(audioDeviceIndex);
	assert(audioDeviceCaps != NULL);
	
	// Build the audio part of the pipeline
	GstElement* osxaudiosrc = gst_element_factory_make("osxaudiosrc", NULL);
	g_object_set(G_OBJECT(osxaudiosrc),
		"do-timestamp", TRUE,
		"device",       audioDeviceIndex,
		NULL);
	// TODO: Need to set latency-time=23220, buffer-time=92880?
	
	srccapsfilter = gst_element_factory_make("capsfilter", NULL);
	caps = gst_caps_from_string(audioDeviceCaps);
	delete[] audioDeviceCaps;
    g_object_set(srccapsfilter,
    	"caps", caps,
    	NULL);
    gst_caps_unref(caps);
    
	GstElement* audioconvert = gst_element_factory_make("audioconvert", NULL);
	
	GstElement* rtpL16pay = gst_element_factory_make("rtpL16pay", NULL);
    g_object_set(rtpL16pay,
    	"buffer-list", TRUE,
    	NULL);
    
	GstElement* asink = gst_element_factory_make("multiudpsink", "asink");
	g_object_set(G_OBJECT(asink),
		"enable-last-sample", FALSE,
		"sync",               FALSE,
		"async",              FALSE,
		NULL);
	
	GstElement* acsink = gst_element_factory_make("multiudpsink", "acsink");
	g_object_set(G_OBJECT(acsink),
		"enable-last-sample", FALSE,
		"sync",               FALSE,
		NULL);
    	
	gst_bin_add_many(GST_BIN(pipeline), osxaudiosrc, srccapsfilter, audioconvert, rtpL16pay, asink, acsink, NULL);
	assert(gst_element_link_many(osxaudiosrc, srccapsfilter, audioconvert, rtpL16pay, NULL));
	assert(gst_element_link_pads(rtpL16pay, "src", rtpbin, "send_rtp_sink_1"));
	assert(gst_element_link_pads(rtpbin, "send_rtp_src_1", asink, "sink"));
	assert(gst_element_link_pads(rtpbin, "send_rtcp_src_1", acsink, "sink"));

	return pipeline;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::PadNotifyCaps()
///
/// Callback when a pad's caps change.
///////////////////////////////////////////////////////////////////////////////////////////////////
void SenderPipeline::PadNotifyCaps(GObject* gobject, GParamSpec* pspec)
{
	GstCaps* pad_caps = gst_pad_get_current_caps(GST_PAD(gobject));
	if (pad_caps != NULL)
	{
		GstStructure* s = gst_caps_get_structure(pad_caps, 0);
		
		const GValue* val = gst_structure_get_value(s, "media");
		if ((val != NULL) && G_VALUE_HOLDS_STRING(val))
		{
			if (g_strcmp0(g_value_get_string(val), "video") == 0)
			{
				val = gst_structure_get_value(s, "sprop-parameter-sets");
				if ((val != NULL) && G_VALUE_HOLDS_STRING(val))
				{
					m_pSpropParameterSets = g_value_get_string(val);
				}
				else
				{
					m_pSpropParameterSets = NULL;
				}
				
				guint ssrc = 0;
				if (gst_structure_get_uint(s, "ssrc", &ssrc))
				{
					m_VideoSsrc = ssrc;
				}
			}
			else if (g_strcmp0(g_value_get_string(val), "audio") == 0)
			{
				guint ssrc = 0;
				if (gst_structure_get_uint(s, "ssrc", &ssrc))
				{
					m_AudioSsrc = ssrc;
				}
			}
		}
		
		gst_caps_unref(pad_caps);
	}
	else
	{
		m_pSpropParameterSets = NULL;
	}
	if ((m_pNotifySink != NULL) && (m_pSpropParameterSets != NULL) && (m_VideoSsrc != 0) && (m_AudioSsrc))
	{
		m_pNotifySink->OnNewParameters(*this, m_pSpropParameterSets, m_VideoSsrc, m_AudioSsrc);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::SetDestinations()
///
/// Internally set the destinations for the pipeline.
///////////////////////////////////////////////////////////////////////////////////////////////////
void SenderPipeline::SetDestinations()
{
	static const struct
	{
		GstElement* pElement;
		const char pPortString[6];
	}
	pairs[] = 
	{
		{m_pVideoRtpSink,  "10000"},
		{m_pVideoRtcpSink, "10001"},
		{m_pAudioRtpSink,  "10002"},
		{m_pAudioRtcpSink, "10003"},
	};
	
	for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++)
	{
		std::string clients;
		for (std::vector<const std::string*>::const_iterator it = m_vDestinations.begin(); it != m_vDestinations.end(); ++it)
		{
			if (!clients.empty())
			{
				clients += ",";
			}
			clients += **it;
			clients += ":";
			clients += pairs[i].pPortString;
		}
		g_object_set(pairs[i].pElement, "clients", clients.c_str(), NULL);
	}
}