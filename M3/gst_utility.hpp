///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file gst_utility.hpp
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
/// @brief This file declares a number of functions we *wish* were part of the GStreamer library,
/// but are not.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __GST_UTILITY_HPP__
#define __GST_UTILITY_HPP__

#include <gst/gst.h> // for GStreamer stuff


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Count the number of sink pads on an element.
///
/// @param The element on which to count sink pads.
///
/// @return The number of sink pads this element currently has.
///////////////////////////////////////////////////////////////////////////////////////////////////
gsize gst_element_count_sink_pads(GstElement* element);


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Count the number of src pads on an element.
///
/// @param The element on which to count src pads.
///
/// @return The number of src pads this element currently has.
///////////////////////////////////////////////////////////////////////////////////////////////////
gsize gst_element_count_src_pads(GstElement* element);


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Find a sink pad by name.
///
/// This function locates a sink pad on a given element by name. It does not increment the ref
/// count on any returned pad. If a pad with the given name cannot be found, this function returns
/// NULL.
///
/// @param The element on which to look for the named pad.
///
/// @param The name of the pad for which to look.
///
/// @return A pad with the given name, or NULL.
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPad* gst_element_find_sink_pad_by_name(GstElement* element, const gchar* name);


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Get the first sink pad from an element.
///
/// Returns the first sink pad on a given element. It does not increment the ref count on any
/// returned pad. Returns NULL if the element has no sink pads.
///
/// @param The element from which to get the first sink pad.
///
/// @return The first sink pad, or NULL.
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPad* gst_element_get_first_sink_pad(GstElement* element);


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Get the first src pad from an element.
///
/// Returns the first src pad on a given element. It does not increment the ref count on any
/// returned pad. Returns NULL if the element has no src pads.
///
/// @param The element from which to get the first src pad.
///
/// @return The first src pad, or NULL.
///////////////////////////////////////////////////////////////////////////////////////////////////
GstPad* gst_element_get_first_src_pad(GstElement* element);

#endif // __GST_UTILITY_HPP__
