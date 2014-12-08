#include <signal.h>
#include <gst/gst.h>

#include "PipelineTracer.hpp"

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

static GstElement *pipeline;

static void on_sig_int(int sig)
{
	// Send EOS on SIGINT
	gst_element_send_event(pipeline, gst_event_new_eos());
}

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
   
	// Parse message
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
		}
		gst_message_unref(msg);
	}

	// Free resources
	delete pTracer;
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	
	return 0;
}