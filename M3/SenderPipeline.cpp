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
	" ! x264enc name=venc speed-preset=ultrafast tune=zerolatency"
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
	, m_pVideoEncoder(gst_bin_get_by_name(GST_BIN(Pipeline()), "venc"))
	, m_pVideoRtpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "vsink"))
	, m_pVideoRtcpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "vcsink"))
	, m_pAudioRtpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "asink"))
	, m_pAudioRtcpSink(gst_bin_get_by_name(GST_BIN(Pipeline()), "acsink"))
	, m_vDestinations()
	, m_pSpropParameterSets(NULL)
{
	assert(m_pVideoEncoder != NULL);
	assert(m_pVideoRtpSink != NULL);
	assert(m_pVideoRtcpSink != NULL);
	assert(m_pAudioRtpSink != NULL);
	assert(m_pAudioRtcpSink != NULL);
	
	// Receive a callback when the caps property of the vsink:sink pad changes
	GstPad* pad = gst_element_get_static_pad(m_pVideoRtpSink, "sink");
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// SenderPipeline::PadNotifyCaps()
///
/// Callback when a pad's caps change.
///////////////////////////////////////////////////////////////////////////////////////////////////
void SenderPipeline::PadNotifyCaps(GObject* gobject, GParamSpec* pspec)
{
	GstCaps* pad_caps = gst_pad_get_current_caps(reinterpret_cast<GstPad*>(gobject));
	const GValue* val = gst_structure_get_value(gst_caps_get_structure(pad_caps, 0), "sprop-parameter-sets");
	if ((val != NULL) && G_VALUE_HOLDS_STRING(val))
	{
		m_pSpropParameterSets = g_value_get_string(val);
	}
	else
	{
		m_pSpropParameterSets = NULL;
	}
	gst_caps_unref(pad_caps);
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