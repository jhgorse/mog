///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file AVList.hpp
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
/// @brief This file declares the AVList and related classes, which allow the application to access
/// a list of A/V input devices.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __AVLIST_HPP__
#define __AVLIST_HPP__

#include <string>
#include <vector>

class AVList
{
public:
	class VideoInput
	{
	public:
		VideoInput(const std::string& name)
			: m_Name(name)
		{}
		
		const std::string& Name() const { return m_Name; }
		
	protected:
	
	private:
		std::string m_Name;
	};
	typedef std::vector<VideoInput> VideoInputList;

	class AudioInput
	{
	public:
		AudioInput(const std::string& name)
			: m_Name(name)
		{}
		
		const std::string& Name() const { return m_Name; }
		
	protected:
	
	private:
		std::string m_Name;
	};
	typedef std::vector<AudioInput> AudioInputList;
	
	AVList();
	
	const VideoInputList& VideoInputs() const { return m_VideoInputList; }
	const AudioInputList& AudioInputs() const { return m_AudioInputList; }
	
	void AddVideoInput(const std::string& name);
	void AddAudioInput(const std::string& name);
	
	void Clear();
	
protected:

private:
	VideoInputList m_VideoInputList;
	AudioInputList m_AudioInputList;
};

class AVListEnumerator
{
public:
	static const AVList& List();
	
protected:
	AVListEnumerator() {}
	virtual ~AVListEnumerator() {}
	virtual void Enumerate() = 0;
	void AddVideoInput(const std::string& name);
	void AddAudioInput(const std::string& name);
	
private:
	static AVListEnumerator* m_pTheInstance;
	AVList m_AvList;
};

#endif // __AVLIST_HPP__