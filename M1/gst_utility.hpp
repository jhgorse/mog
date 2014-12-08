#include <gst/gst.h>

/* Count the number of sink pads on an element */
gsize gst_element_count_sink_pads(GstElement* element);

/* Count the number of src pads on an element */
gsize gst_element_count_src_pads(GstElement* element);

/* Find a sink pad by name */
GstPad* gst_element_find_sink_pad_by_name(GstElement* element, const gchar* name);

/* Get the first sink pad from an element (transfer: none) */
GstPad* gst_element_get_first_sink_pad(GstElement* element);

/* Get the first src pad from an element (transfer: none) */
GstPad* gst_element_get_first_src_pad(GstElement* element);