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
	" ! rtph264depay"
	" ! video/x-h264,stream-format=avc,alignment=au"
	" ! avdec_h264"
	" ! videoconvert"
	" ! osxvideosink enable-last-sample=false sync=false"
	"   rtpbin."
	" ! rtpL16depay"
	" ! audioconvert"
	" ! osxaudiosink enable-last-sample=false buffer-time=30000"
;


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