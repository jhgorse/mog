///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file m1.cpp
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
/// @brief This file provides the "main" code to implement Milestone 1.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <signal.h>           // for Posix signal-handling
#include <gst/gst.h>          // for GStreamer stuff
#include "PipelineTracer.hpp" // for PipelineTracer


///////////////////////////////////////////////////////////////////////////////////////////////////
/// This string is the pipeline created for this milestone. There are a lot of "hard-coded" things
/// here that should be made more fluid and configurable in future milestones.
///////////////////////////////////////////////////////////////////////////////////////////////////
static const char PIPELINE_STRING[] =
  "   rtpbin name=sendrtpbin latency=10"
  "   avfvideosrc name=videosrc do-timestamp=true device-index=0"
  " ! video/x-raw, format=(string)UYVY, width=(int)640, height=(int)480, framerate=(fraction)10000000/333333"
  " ! videoconvert"
  " ! timeoverlay font-desc=\"Sans Bold 36\" valignment=\"bottom\" halignment=\"right\""
  " ! tee name=t"
  "   t. ! videoconvert ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream silent=true ! osxvideosink enable-last-sample=false"
  "   t."
  " ! x264enc bitrate=5000 speed-preset=ultrafast tune=zerolatency"
  " ! rtph264pay"
  " ! sendrtpbin.send_rtp_sink_0"
  "   osxaudiosrc do-timestamp=true latency-time=21333 buffer-time=21333"
  " ! audio/x-raw, format=(string)S32LE, layout=(string)interleaved, rate=(int)48000, channels=(int)1"
  " ! audioconvert"
  " ! rtpL16pay buffer-list=true"
  " ! sendrtpbin.send_rtp_sink_1"
  "   sendrtpbin.send_rtp_src_0"
  " ! udpsink name=vsink enable-last-sample=false sync=false async=false"
  "   sendrtpbin.send_rtcp_src_0"
  " ! udpsink name=vcsink enable-last-sample=false sync=false"
  "   sendrtpbin.send_rtp_src_1"
  " ! udpsink name=asink enable-last-sample=false sync=false async=false"
  "   sendrtpbin.send_rtcp_src_1"
  " ! udpsink name=acsink enable-last-sample=false sync=false"
  "   rtpbin name=recvrtpbin latency=10"
  "   udpsrc port=10000"
  " ! application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,sprop-parameter-sets=\"Z3oAHry0BQHtgIgAKLCoCYloBHixdQ\\=\\=\\,aM48gA\\=\\=\",payload=96"
  " ! recvrtpbin.recv_rtp_sink_0"
  "   udpsrc port=10001"
  " ! application/x-rtcp"
  " ! recvrtpbin.recv_rtcp_sink_0"
  "   udpsrc port=10002"
  " ! application/x-rtp,media=audio,clock-rate=48000,encoding-name=L16,encoding-params=1,channels=1,payload=96"
  " ! recvrtpbin.recv_rtp_sink_1"
  "   udpsrc port=10003"
  " ! application/x-rtcp"
  " ! recvrtpbin.recv_rtcp_sink_1"
  "   recvrtpbin."
  " ! rtph264depay"
  " ! video/x-h264,stream-format=avc,alignment=au"
  " ! avdec_h264"
  " ! autovideoconvert"
  " ! osxvideosink enable-last-sample=false"
  "   recvrtpbin."
  " ! rtpL16depay"
  " ! audioconvert"
  " ! osxaudiosink enable-last-sample=false buffer-time=30000";


/// The one and only GStreamer pipeline. It is static to this file because it's needed in the
/// signal handler.
static GstElement *pipeline;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// on_sig_int()
///
/// Called on a SIGINT signal. Sends an EOS to the pipeline.
///////////////////////////////////////////////////////////////////////////////////////////////////
static void on_sig_int(int sig)
{
	// Send EOS on SIGINT
	gst_element_send_event(pipeline, gst_event_new_eos());
} // END on_sig_int()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// main()
///
/// The main function. Creates the pipeline and makes it go.
///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	// Initialize GStreamer
	gst_init (&argc, &argv);
	
	// Parse the target hostname/IP
	if (argc != 2)
	{
		g_printerr("Usage: %s [host or ip]\n", argv[0]);
		return -1;
	}
	const gchar* target = argv[1];
	g_assert(target != NULL);

	// Parse the pipeline from the string above
	pipeline = gst_parse_launch(PIPELINE_STRING, NULL);
	if (pipeline == NULL)
	{
		g_printerr("Failed to create pipeline!\n");
		return -1;
	}
	
	// Set the clients property of the UDP sink elements
	GstElement* element = gst_bin_get_by_name(GST_BIN(pipeline), "vsink");
	g_assert(element != NULL);
	const gchar* clients_value = g_strdup_printf("%s:10000", target);
	g_object_set(element, "clients", clients_value, NULL);
	g_free(const_cast<gchar*>(clients_value));
	gst_object_unref(element);
	
	element = gst_bin_get_by_name(GST_BIN(pipeline), "vcsink");
	g_assert(element != NULL);
	clients_value = g_strdup_printf("%s:10001", target);
	g_object_set(element, "clients", clients_value, NULL);
	g_free(const_cast<gchar*>(clients_value));
	gst_object_unref(element);
	
	element = gst_bin_get_by_name(GST_BIN(pipeline), "asink");
	g_assert(element != NULL);
	clients_value = g_strdup_printf("%s:10002", target);
	g_object_set(element, "clients", clients_value, NULL);
	g_free(const_cast<gchar*>(clients_value));
	gst_object_unref(element);
	
	element = gst_bin_get_by_name(GST_BIN(pipeline), "acsink");
	g_assert(element != NULL);
	clients_value = g_strdup_printf("%s:10003", target);
	g_object_set(element, "clients", clients_value, NULL);
	g_free(const_cast<gchar*>(clients_value));
	gst_object_unref(element);

	// Create a pipeline tracer for latency / jitter information
	PipelineTracer* pTracer = new PipelineTracer(pipeline);

	// Put the pipeline in the playing state
	GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(pipeline);
		return -1;
	}
	
	// Dump to dot file (if GST_DEBUG_DUMP_DOT_DIR is set) to ${GST_DEBUG_DUMP_DOT_DIR}/.dot.
	// We wait until the pipeline is playing to make sure pads are linked.
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, argv[0]);
	
	// Assign the SIGINT handler to send EOS
	struct sigaction sigact;
	sigact.sa_handler = on_sig_int;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	g_print("Playing... press Ctrl-C to terminate.\n");
  
	// Wait until error or EOS
	GstBus* bus = gst_element_get_bus(pipeline);
	GstMessage* msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
   
	// Parse message and print stuff about it.
	if (msg != NULL)
	{
		GError *err;
		gchar *debug_info;
		
		switch (GST_MESSAGE_TYPE(msg))
		{
			case GST_MESSAGE_ERROR:
				gst_message_parse_error(msg, &err, &debug_info);
				g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
				g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
				g_clear_error(&err);
				g_free(debug_info);
				break;
				
			case GST_MESSAGE_EOS:
				g_print("End-Of-Stream reached.\n");
				break;
				
			default:
				// We should not reach here because we only asked for ERRORs and EOS
				g_printerr("Unexpected message received.\n");
				break;
		} // END switch(message type)
		gst_message_unref(msg);
	} // END if (message)

	// Free resources
	delete pTracer;
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	
	return 0;
} // END main()
