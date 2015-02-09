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
#include "ReceiverPipeline.hpp"
#include "SenderPipeline.hpp"

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
	, protected ReceiverPipeline::IReceiverNotifySink
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
	
	
	/// Called by the receiver pipeline when an SSRC (that we are receiving) becomes active.
	virtual void OnSsrcActivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc);
	
	
	/// Called by the receiver pipeline when an SSRC (that we are receiving) becomes inactive.
	virtual void OnSsrcDeactivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc, SsrcDeactivateReason reason);
	
	
	/// Called by the conference annunciator when a parameter packet arrives.
	virtual void OnParameterPacket(const char* address, const char* pictureParameters, unsigned int videoSsrc, unsigned int audioSsrc);
	
	
private:
	/// Name of directory file
	static const char DIRECTORY_FILENAME[];
	
	
	/// Load the directory of available participants.
	void LoadDirectory();
	
	
	/// Get the address for a participant name.
	const char* GetAddressForParticipant(const char* name);
	
	
	/// Get the name for a participant address.
	const std::string* GetNameForAddress(const std::string& address);
	
	
	/// Get the video panel for a participant address.
	VideoPanel* GetPanelForAddress(const std::string& address);
	
	
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
	
	
	/// A conference participant, complete with parameters.
	struct Participant
	{
		Participant(std::string addr, std::string p, unsigned int v, unsigned int a)
			: address(addr)
			, pictureParameters(p)
			, videoSsrc(v)
			, audioSsrc(a)
		{}

		std::string address;		
		std::string pictureParameters;
		unsigned int videoSsrc;
		unsigned int audioSsrc;
	};
	
	
	/// NOTE: in an ideal world, we should probably use a mutex to protect most of this instance
	/// data, as it could be accessed from multiple threads. For now, however, it seems unlikely
	/// to happen, and this is a prototype.
	
	/// The directory of available participants.
	std::vector<DirectoryEntry> m_Directory;
	
	
	/// "My" address, used to exclude "myself" from various things.
	std::string m_MyAddress;
	
	
	/// A hash of participants by video SSRC
	std::unordered_map<unsigned int, Participant*> m_ParticipantByVideoSsrc;
	
	
	/// A hash of participants by audio SSRC
	std::unordered_map<unsigned int, Participant*> m_ParticipantByAudioSsrc;
	
	
	/// A list of "orphaned" video SSRCs -- video SSRCs that have become active, but for which we
	/// have not (yet) received parameter packets.
	std::vector<unsigned int> m_OrphanedVideoSsrcs;
	
	
	/// A list of "orphaned" audio SSRCs -- audio SSRCs that have become active, but for which we
	/// have not (yet) received parameter packets.
	std::vector<unsigned int> m_OrphanedAudioSsrcs;
	
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
	
	
	/// The receiver pipeline. Not created until the GUI's main thread is started up.
	ReceiverPipeline* m_pReceiverPipeline;
};

#endif // __M4FRAME_HPP__