///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file M4Application.cpp
///
/// Copyright (c) 2015, BoxCast, Inc. All rights reserved.
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
/// @brief This file defines the functions of the M4Application class.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <gst/gst.h>
#include "M4Application.hpp"
#include "M4Frame.hpp"
 
IMPLEMENT_APP(M4Application)
 
bool M4Application::OnInit()
{
	// Initialize GStreamer
	int argc = 0;
	gst_init(&argc, NULL);
	
	M4Frame *frame = new M4Frame();
 	frame->Centre();
	frame->Show(TRUE);
	SetTopWindow(frame);
	return TRUE;
}