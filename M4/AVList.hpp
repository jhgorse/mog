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

#include <string> // for std::string
#include <vector> // for std::vector

///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class is a container class that stores a list of available A/V input devices. Video
/// device names are accessed via the VideoInputs accessor, and audio device names are accessed via
/// the AudioInputs accessor.
///////////////////////////////////////////////////////////////////////////////////////////////////
class AVList
{
public:
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This class represents a video input. Currently this is just a string name, but it could be
	/// extended to include more attributes.
	///////////////////////////////////////////////////////////////////////////////////////////////
	class VideoInput
	{
	public:
		/// Constructor.
		VideoInput(const std::string& name)
			: m_Name(name)
		{}
		
		/// Name accessor.
		const std::string& Name() const { return m_Name; }
		
	protected:
	
	private:
		/// Name member variable.
		std::string m_Name;
	};
	
	
	/// A container for video inputs.
	typedef std::vector<VideoInput> VideoInputList;


	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This class represents an audio input. Currently this is just a string name, but it could be
	/// extended to include more attributes.
	///////////////////////////////////////////////////////////////////////////////////////////////
	class AudioInput
	{
	public:
		/// Constructor.
		AudioInput(const std::string& name)
			: m_Name(name)
		{}
		
		/// Name accessor
		const std::string& Name() const { return m_Name; }
		
	protected:
	
	private:
		/// Name member variable.
		std::string m_Name;
	};
	
	
	/// A container for audio inputs.
	typedef std::vector<AudioInput> AudioInputList;
	
	
	/// Constructor.
	inline AVList()
		: m_VideoInputList()
		, m_AudioInputList()
	{}
	
	
	/// Video input list accessor.
	const VideoInputList& VideoInputs() const { return m_VideoInputList; }
	
	
	/// Audio input list accessor.
	const AudioInputList& AudioInputs() const { return m_AudioInputList; }
	
	
protected:
	/// Allow AVListEnumerator to clear and add inputs.
	friend class AVListEnumerator;
	
	
	/// Clear the container of all video and audio input entries.
	void Clear()
	{
		m_VideoInputList.clear();
		m_AudioInputList.clear();
	}
	
	
	/// Add a video input by name.
	inline void AddVideoInput(const std::string& name)
	{
		m_VideoInputList.push_back(VideoInput(name));
	}
	
	
	/// Add an audio input by name.
	inline void AddAudioInput(const std::string& name)
	{
		m_AudioInputList.push_back(AudioInput(name));
	}

private:
	/// The video input list container.
	VideoInputList m_VideoInputList;
	
	
	/// The audio input list container.
	AudioInputList m_AudioInputList;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class provides a means of enumerating the available A/V input devices that the user can
/// choose to use. It is a private singleton, with a static public interface.
///////////////////////////////////////////////////////////////////////////////////////////////////
class AVListEnumerator
{
public:
	/// Returns an AVList representing the available A/V input devices.
	static const AVList& List();
	
protected:
	/// Constructor.
	AVListEnumerator() {}
	
	
	/// Destructor.
	virtual ~AVListEnumerator() {}
	
	
	/// Virtual function to enumerate devices; concrete implementations implement this for specific
	/// platforms.
	virtual void Enumerate() = 0;
	
	
	/// Add a video input by name.
	inline void AddVideoInput(const std::string& name)
	{
		m_AvList.AddVideoInput(name);
	}
	
	
	/// Add an audio input by name.
	inline void AddAudioInput(const std::string& name)
	{
		m_AvList.AddAudioInput(name);
	}
	
private:
	/// The static (singleton) instance pointer.
	static AVListEnumerator* m_pTheInstance;
	
	
	/// The list of available input devices.
	AVList m_AvList;
};

#endif // __AVLIST_HPP__