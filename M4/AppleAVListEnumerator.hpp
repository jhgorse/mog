///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file AppleAVListEnumerator.hpp
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
/// @brief This file declares the AppleAVListEnumerator class, which proxies for AVList enumeration
/// in Objective-C.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __APPLEAVLISTENUMERATOR_HPP__
#define __APPLEAVLISTENUMERATOR_HPP__

#include "AVList.hpp" // for AVListEnumerator

///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class provides an implementation of AVListEnumerator that enumerates A/V input devices on
/// apple (OS X).
///////////////////////////////////////////////////////////////////////////////////////////////////
class AppleAVListEnumerator : public AVListEnumerator
{
protected:

	/// Enumerate all A/V input devices.
	virtual void Enumerate();
};

#endif // __APPLEAVLISTENUMERATOR_HPP__