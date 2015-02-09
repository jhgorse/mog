///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file AVList.cpp
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
/// @brief This file defines the functions of AVList and related classes.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>    // for assert
#include "AVList.hpp" // for AVList and AVListEnumerator

#if defined(__APPLE__)
// Pull in the apple implementation
#include "AppleAVListEnumerator.hpp"
#endif


/// The static (singleton) instance pointer.
AVListEnumerator* AVListEnumerator::m_pTheInstance = NULL;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// AVListEnumerator::List()
///
/// Accessor to return the list of available A/V input devices.
///
/// If the singleton instance is not created, it will be created. So this isn't particularly
/// thread-safe.
///////////////////////////////////////////////////////////////////////////////////////////////////
const AVList& AVListEnumerator::List()
{
	if (m_pTheInstance == NULL)
	{
		m_pTheInstance = 
#if defined(__APPLE__)
			new AppleAVListEnumerator
#else
			NULL
#endif
		;
	}
	assert(m_pTheInstance != NULL);
	
	/// Clear the lists, then ask the implementation to enumerate.
	m_pTheInstance->m_AvList.Clear();
	m_pTheInstance->Enumerate();
	
	return m_pTheInstance->m_AvList;
}