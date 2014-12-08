#include "gst_utility.hpp"

gsize gst_element_count_sink_pads(GstElement* element)
{
	GstIterator *iter = gst_element_iterate_sink_pads(element);
	GValue vPad = G_VALUE_INIT;
	gsize ret = 0;
	while (gst_iterator_next(iter, &vPad) == GST_ITERATOR_OK)
	{
		ret++;
	}
	gst_iterator_free(iter);
	return ret;
}

gsize gst_element_count_src_pads(GstElement* element)
{
	GstIterator *iter = gst_element_iterate_src_pads(element);
	GValue vPad = G_VALUE_INIT;
	gsize ret = 0;
	while (gst_iterator_next(iter, &vPad) == GST_ITERATOR_OK)
	{
		ret++;
	}
	gst_iterator_free(iter);
	return ret;
}

GstPad* gst_element_find_sink_pad_by_name(GstElement* element, const gchar* name)
{
	GstIterator *iter = gst_element_iterate_sink_pads(element);
	GValue vPad = G_VALUE_INIT;
	GstPad* ret = NULL;
	while (gst_iterator_next(iter, &vPad) == GST_ITERATOR_OK)
	{
		GstPad* pad = GST_PAD(g_value_get_object(&vPad));
		const gchar* pad_name = gst_pad_get_name(pad);
		if (g_strcmp0(pad_name, name) == 0)
		{
			ret = pad;
		}
		g_free(const_cast<gchar*>(pad_name));
		if (ret != NULL)
		{
			break;
		}
	}
	gst_iterator_free(iter);
	return ret;
}

GstPad* gst_element_get_first_sink_pad(GstElement* element)
{
	GstIterator *iter = gst_element_iterate_sink_pads(element);
	GValue vPad = G_VALUE_INIT;
	g_assert(gst_iterator_next(iter, &vPad) == GST_ITERATOR_OK);
	GstPad* ret = GST_PAD(g_value_get_object(&vPad));
	gst_iterator_free(iter);
	return ret;
}

GstPad* gst_element_get_first_src_pad(GstElement* element)
{
	GstIterator *iter = gst_element_iterate_src_pads(element);
	GValue vPad = G_VALUE_INIT;
	g_assert(gst_iterator_next(iter, &vPad) == GST_ITERATOR_OK);
	GstPad* ret = GST_PAD(g_value_get_object(&vPad));
	gst_iterator_free(iter);
	return ret;
}