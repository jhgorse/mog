///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file gst_utility.cpp
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
/// @brief This file implements the utility functions declared in gst_utility.hpp.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "gst_utility.hpp" // for utility function declarations


///////////////////////////////////////////////////////////////////////////////////////////////////
/// gst_element_count_sink_pads()
///
/// Count the element's sink pads by using gst_element_iterate_sink_pads.
///////////////////////////////////////////////////////////////////////////////////////////////////
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
} // END gst_element_count_sink_pads()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// gst_element_count_src_pads()
///
/// Count the element's src pads by using gst_element_iterate_src_pads.
///////////////////////////////////////////////////////////////////////////////////////////////////
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
} // END gst_element_count_src_pads()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// gst_element_find_sink_pad_by_name()
///
/// Find an element's sink pad by name by using gst_element_iterate_sink_pads and comparing the
/// element's name with that provided.
///////////////////////////////////////////////////////////////////////////////////////////////////
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
} // END gst_element_find_sink_pad_by_name()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// gst_element_get_first_sink_pad()
///
/// Get an element's first sink pad by using gst_element_iterate_sink_pads. The pad's ref count is
/// NOT incremented.
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPad* gst_element_get_first_sink_pad(GstElement* element)
{
	GstIterator *iter = gst_element_iterate_sink_pads(element);
	GValue vPad = G_VALUE_INIT;
	GstPad* ret = NULL;
	if (gst_iterator_next(iter, &vPad) == GST_ITERATOR_OK)
	{
		ret = GST_PAD(g_value_get_object(&vPad));
	}
	gst_iterator_free(iter);
	return ret;
} // END gst_element_get_first_sink_pad()


///////////////////////////////////////////////////////////////////////////////////////////////////
/// gst_element_get_first_src_pad()
///
/// Get an element's first source pad by using gst_element_iterate_sink_pads. The pad's ref count
/// is NOT incremented.
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPad* gst_element_get_first_src_pad(GstElement* element)
{
	GstIterator *iter = gst_element_iterate_src_pads(element);
	GValue vPad = G_VALUE_INIT;
	GstPad* ret = NULL;
	if (gst_iterator_next(iter, &vPad) == GST_ITERATOR_OK)
	{
		ret = GST_PAD(g_value_get_object(&vPad));
	}
	gst_iterator_free(iter);
	return ret;
} // END gst_element_get_first_src_pad()
