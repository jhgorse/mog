///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file M4Application.hpp
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
/// @brief This file declares the M4Application class, which represents the starting point for the
/// basic M4 application.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __M4APPLICATION_HPP__
#define __M4APPLICATION_HPP__

#include <wx/wx.h>

class M4Application : public wxApp
{
public:
	virtual bool OnInit();
};

#endif // __M4APPLICATION_HPP__