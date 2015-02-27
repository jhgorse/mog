///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file M4Frame.hpp
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
/// @brief This file declares the M4Frame class, which is the basic frame for the M4 application.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __M4FRAME_HPP__
#define __M4FRAME_HPP__

#include <unordered_map>
#include <vector>
#include <wx/wx.h>

#include "ConferenceAnnunciator.hpp"
#include "SenderPipeline.hpp"

class ReceiverPipeline;
class VideoPanel;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame
///
/// This is the main frame for the M4 application. It uses several "child" dialog boxes to walk the
/// user through the initial configuration workflow.
///////////////////////////////////////////////////////////////////////////////////////////////////
class M4Frame
	: public wxFrame
	, protected SenderPipeline::ISenderParameterNotifySink
	, protected ConferenceAnnunciator::IParameterPacketListener
{
public:
	/// Constructor.
	M4Frame();
	
	
	/// Destructor.
	virtual ~M4Frame();
	
	
	/// Called when the application is idle.
	void OnIdle(wxIdleEvent& evt);
	
	
protected:
	/// Called by the sender pipeline when the sender-side parameters are available.
	virtual void OnNewParameters(const SenderPipeline& rPipeline, const char* pPictureParameters, unsigned int videoSsrc, unsigned int audioSsrc);
	
	
	/// Called by the conference annunciator when a parameter packet arrives.
	virtual void OnParameterPacket(const char* address, const char* pictureParameters, unsigned int videoSsrc, unsigned int audioSsrc);
	
	
private:
	/// Name of directory file
	static const char DIRECTORY_FILENAME[];
	
	
	/// Video bitrate
	static const size_t VIDEO_BITRATE;
	
	
	/// Load the directory of available participants.
	void LoadDirectory();
	
	
	/// Get the address for a participant name.
	const char* GetAddressForParticipant(const char* name);
	
	
	/// Get the name for a participant address.
	const std::string* GetNameForAddress(const std::string& address);
	
	
	/// Get the video panel for a participant address.
	VideoPanel* GetPanelForAddress(const std::string& address, size_t& rIndexOut);
	
	
	/// A name and address entry in the directory.
	struct DirectoryEntry
	{
		DirectoryEntry(std::string n, std::string a)
			: name(n)
			, address(a)
		{}
		
		std::string name;
		std::string address;
	};
	
	
	/// NOTE: in an ideal world, we should probably use a mutex to protect most of this instance
	/// data, as it could be accessed from multiple threads. For now, however, it seems unlikely
	/// to happen, and this is a prototype.
	
	/// The directory of available participants.
	std::vector<DirectoryEntry> m_Directory;
	
	
	/// "My" address, used to exclude "myself" from various things.
	std::string m_MyAddress;
	
	
	/// A hash of receiver pipelines by video SSRC
	std::unordered_map<unsigned int, ReceiverPipeline*> m_ReceiverPipelinesByVideoSsrc;
	
	
	/// The name of the chosen video input
	std::string videoInputName;
	
	
	/// The name of the chosen audio input
	std::string audioInputName;
	
	
	/// An array of participant addresses
	wxArrayString m_ParticipantList;
	
	
	/// The conference annunciator.
	ConferenceAnnunciator m_Annunciator;
	
	
	/// An array of video panels instances
	VideoPanel* m_VideoPanels[6];
	
	
	/// The sender pipeline. Not created until the GUI's main thread is started up.
	SenderPipeline* m_pSenderPipeline;
};

#endif // __M4FRAME_HPP__