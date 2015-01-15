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
	"   udpsrc address=\"%s\" port=10000"
	" ! application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,sprop-parameter-sets=\"Z3oAH7y0AoAt2AiAAosKgJiWgEeMGVA\\=\\,aM48gA\\=\\=\",payload=96"
	" ! rtpbin.recv_rtp_sink_0"
	"   udpsrc address=\"%s\" port=10001"
	" ! application/x-rtcp"
	" ! rtpbin.recv_rtcp_sink_0"
	"   udpsrc address=\"%s\" port=10002"
	" ! application/x-rtp,media=audio,clock-rate=48000,encoding-name=L16,encoding-params=1,channels=1,payload=96"
	" ! rtpbin.recv_rtp_sink_1"
	"   udpsrc address=\"%s\" port=10003"
	" ! application/x-rtcp"
	" ! rtpbin.recv_rtcp_sink_1"
	"   rtpbin."
	" ! rtph264depay name=vdepay"
	" ! video/x-h264,stream-format=avc,alignment=au"
	" ! avdec_h264"
	" ! videoconvert"
	" ! osxvideosink enable-last-sample=false sync=false"
	"   rtpbin."
	" ! rtpL16depay name=adepay"
	" ! audioconvert"
	" ! osxaudiosink enable-last-sample=false buffer-time=92880"
;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor.
///
/// Parse the launch string to construct the pipeline; obtain some references; and install a
/// callback function for when pads are added to rtpbin.
///////////////////////////////////////////////////////////////////////////////////////////////////
ReceiverPipeline::ReceiverPipeline(const char* address)
	: PipelineBase(ParsePipeline(address))
	, m_pRtpBin(gst_bin_get_by_name(GST_BIN(Pipeline()), "rtpbin"))
	, m_pVideoDepayloaderSinkPad(GetElementSinkPad(GST_BIN(Pipeline()), "vdepay", "sink"))
	, m_pAudioDepayloaderSinkPad(GetElementSinkPad(GST_BIN(Pipeline()), "adepay", "sink"))
{
	assert(m_pRtpBin != NULL);
	assert(m_pVideoDepayloaderSinkPad != NULL);
	assert(m_pAudioDepayloaderSinkPad != NULL);
	g_signal_connect(m_pRtpBin, "pad-added", G_CALLBACK(StaticOnRtpBinPadAdded), this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Destructor.
///
/// Release references obtained in constructor.
///////////////////////////////////////////////////////////////////////////////////////////////////
ReceiverPipeline::~ReceiverPipeline()
{
	gst_object_unref(m_pAudioDepayloaderSinkPad);
	gst_object_unref(m_pVideoDepayloaderSinkPad);
	gst_object_unref(m_pRtpBin);
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
	GstPad* sink_pad = NULL;
	if (g_strcmp0(media_type, "audio") == 0)
	{
		sink_pad = m_pAudioDepayloaderSinkPad;
	}
	else if (g_strcmp0(media_type, "video") == 0)
	{
		sink_pad = m_pVideoDepayloaderSinkPad;
	}
	else
	{
		const gchar* pad_name = gst_pad_get_name(pad);
		const gchar* caps_string = gst_caps_to_string(pad_caps);
		g_message("Pad \"%s\" with caps \"%s\" added to rtpbin! Not a known media type!", pad_name, caps_string);
		g_free(const_cast<gchar*>(caps_string));
		g_free(const_cast<gchar*>(pad_name));
	}
	
	// If we found a sink pad, then get its peer (the current rtpbin src pad), unlink it, and
	// link it to the new src pad.
	if (sink_pad != NULL)
	{
		GstPad* src_pad = gst_pad_get_peer(sink_pad);
		gst_pad_unlink(src_pad, sink_pad);
		gst_pad_link(pad, sink_pad);
		gst_object_unref(src_pad);
	}
	
	gst_caps_unref(pad_caps);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Static callback function for when pads are added to rtpbin. Calls the instance version.
///////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiverPipeline::StaticOnRtpBinPadAdded(GstElement* element, GstPad* pad, gpointer data)
{
	((ReceiverPipeline *)data)->OnRtpBinPadAdded(element, pad);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Obtain an element's sink pad by name. Unrefs the intermediate objects. For use in the ctor's
/// member initialization list.
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPad* ReceiverPipeline::GetElementSinkPad(const GstBin* bin, const char* element_name, const char* pad_name)
{
	GstElement* element = gst_bin_get_by_name(const_cast<GstBin*>(bin), element_name);
	GstPad* ret = gst_element_find_sink_pad_by_name(element, pad_name);
	gst_object_unref(element);
	return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////
/// Parse the launch string, interpreted with address, into a pipeline. This is used in the
/// constructor to properly free data.
///////////////////////////////////////////////////////////////////////////////////////////////
GstElement* ReceiverPipeline::ParsePipeline(const char* address)
{
	const gchar* value = g_strdup_printf(PIPELINE_STRING, address, address, address, address);
	GstElement* ret = gst_parse_launch(value, NULL);
	g_free(const_cast<gchar*>(value));
	return ret;
}