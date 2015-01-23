///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file m3.cpp
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
/// @brief This file provides the "main" code to implement Milestone 3.
///
/// @todo Figure out why sometimes we get stuck waiting for a buffer from the camera.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <signal.h>             // for Posix signal-handling
#include <stdio.h>              // for standard IO
#include <cassert>              // for assert
#include "ReceiverPipeline.hpp" // for SenderPipeline
#include "SenderPipeline.hpp"   // for SenderPipeline


static ReceiverPipeline* s_pReceiverPipeline;
static SenderPipeline* s_pSenderPipeline;
static GMainLoop* s_pLoop;
static const size_t VIDEO_BITRATE = 5000000;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// on_sig_int()
///
/// Called on a SIGINT signal. Sends an EOS to the pipeline.
///////////////////////////////////////////////////////////////////////////////////////////////////
static void on_sig_int(int sig)
{
	// Send EOS on SIGINT
	s_pSenderPipeline->SendEOS();
} // END on_sig_int()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// BusMessage()
///
/// Called when messages arrive on a bus.
///////////////////////////////////////////////////////////////////////////////////////////////////
static gboolean BusMessage(GstBus* bus, GstMessage* msg, gpointer data)
{
	// Parse message and print stuff about it.
	switch (GST_MESSAGE_TYPE(msg))
	{
		case GST_MESSAGE_ERROR:
			GError *err;
			gchar *debug_info;
			gst_message_parse_error(msg, &err, &debug_info);
			g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
			g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
			g_clear_error(&err);
			g_free(debug_info);
			g_main_loop_quit(s_pLoop);
			break;
				
		case GST_MESSAGE_EOS:
			g_print("End-Of-Stream reached.\n");
			g_main_loop_quit(s_pLoop);
			break;
				
		default:
			break;
	} // END switch(message type)
	
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// main()
///
/// The main function. Creates the pipeline and makes it go.
///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	// Initialize GStreamer
	gst_init(&argc, &argv);
	
	// Create a new main loop
	s_pLoop = g_main_loop_new(NULL, FALSE);
	
	// Create pipelines
	s_pSenderPipeline = new SenderPipeline();
	s_pSenderPipeline->AddBusWatch(BusMessage);
	s_pSenderPipeline->SetBitrate(VIDEO_BITRATE);
	for (int i = 1; i < argc; ++i)
	{
		s_pSenderPipeline->AddDestination(argv[i]);
	}
	s_pReceiverPipeline = new ReceiverPipeline();
	s_pReceiverPipeline->AddBusWatch(BusMessage);

	// Put the pipelines in the playing state
	s_pSenderPipeline->Play();
	s_pReceiverPipeline->Play();
	
	// Assign the SIGINT handler to send EOS
	struct sigaction sigact;
	sigact.sa_handler = on_sig_int;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	g_print("Playing... press Ctrl-C to terminate.\n");
	
	// Run the main loop
	g_main_loop_run(s_pLoop);

	// Free resources
	delete s_pSenderPipeline;
	delete s_pReceiverPipeline;
	
	return 0;
} // END main()
