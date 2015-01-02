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
/// @todo Does the receiver pipeline die if a sender starts sending after a long period of time?
/// @todo osxaudiosrc not actually using microphone sound? Need a different device?
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <signal.h>             // for Posix signal-handling
#include <stdio.h>              // for standard IO
#include <cassert>              // for assert
#include <vector>               // for std::vector
#include "ReceiverPipeline.hpp" // for SenderPipeline
#include "SenderPipeline.hpp"   // for SenderPipeline


static std::vector<ReceiverPipeline*> s_vReceivers;
static SenderPipeline* s_pSenderPipeline;
static GMainLoop* s_pLoop;


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
	// Parse the target hostname/IP
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s [send host or ip] [receive host or ip 1] [receive host or ip 2]...\n", argv[0]);
		return -1;
	}
	const char* target = argv[1];
	assert(target != NULL);
	
	// Initialize GStreamer
	gst_init(&argc, &argv);
	
	// Create a new main loop
	s_pLoop = g_main_loop_new(NULL, FALSE);
	
	// Create pipelines
	s_pSenderPipeline = new SenderPipeline();
	s_pSenderPipeline->AddBusWatch(BusMessage);
	s_pSenderPipeline->SetDestination(target);
	for (int i = 2; i < argc; ++i)
	{
		ReceiverPipeline *p = new ReceiverPipeline(argv[i]);
		p->AddBusWatch(BusMessage);
		s_vReceivers.push_back(p);
	}

	// Put the pipelines in the playing state
	s_pSenderPipeline->Play();
	for (std::vector<ReceiverPipeline*>::iterator it = s_vReceivers.begin(); it != s_vReceivers.end(); ++it)
	{
		(*it)->Play();
	}
	
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
	while (s_vReceivers.size() > 0)
	{
		delete s_vReceivers.back();
		s_vReceivers.pop_back();
	}
	
	return 0;
} // END main()
