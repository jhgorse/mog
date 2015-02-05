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

#include <cassert>
#include "AVList.hpp"

#if defined(__APPLE__)
#include "AppleAVListEnumerator.hpp"
#endif

AVListEnumerator* AVListEnumerator::m_pTheInstance = NULL;

AVList::AVList()
	: m_VideoInputList()
	, m_AudioInputList()
{}

void AVList::AddVideoInput(const std::string& name)
{
	m_VideoInputList.push_back(VideoInput(name));
}

void AVList::AddAudioInput(const std::string& name)
{
	m_AudioInputList.push_back(AudioInput(name));
}

void AVList::Clear()
{
	m_VideoInputList.clear();
	m_AudioInputList.clear();
}

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
	
	m_pTheInstance->m_AvList.Clear();
	m_pTheInstance->Enumerate();
	
	return m_pTheInstance->m_AvList;
}

void AVListEnumerator::AddVideoInput(const std::string& name)
{
	m_AvList.AddVideoInput(name);
}

void AVListEnumerator::AddAudioInput(const std::string& name)
{
	m_AvList.AddAudioInput(name);
}