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

class M4Frame
	: public wxFrame
	, protected SenderPipeline::ISenderParameterNotifySink
	, protected ReceiverPipeline::IReceiverNotifySink
	, protected ConferenceAnnunciator::IParameterPacketListener
{
public:
	M4Frame();
	virtual ~M4Frame();
	
	void OnIdle(wxIdleEvent& evt);
	
protected:
	virtual void OnNewParameters(const SenderPipeline& rPipeline, const char* pPictureParameters, unsigned int videoSsrc, unsigned int audioSsrc);
	virtual void OnSsrcActivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc);
	virtual void OnSsrcDeactivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc, SsrcDeactivateReason reason);
	virtual void OnParameterPacket(const char* address, const char* pictureParameters, unsigned int videoSsrc, unsigned int audioSsrc);
	
private:
	void LoadDirectory();
	const char* GetAddressForParticipant(const char* name);
	const std::string* GetNameForAddress(const std::string& address);
	VideoPanel* GetPanelForAddress(const std::string& address);
	
	struct DirectoryEntry
	{
		DirectoryEntry(std::string n, std::string a)
			: name(n)
			, address(a)
		{}
		
		std::string name;
		std::string address;
	};
	
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
	
	// TODO: Could probably use a mutex around some of this stuff
	std::vector<DirectoryEntry> m_Directory;
	std::string m_MyAddress;
	std::unordered_map<unsigned int, Participant*> m_ParticipantByVideoSsrc;
	std::unordered_map<unsigned int, Participant*> m_ParticipantByAudioSsrc;
	std::vector<unsigned int> m_OrphanedVideoSsrcs;
	std::vector<unsigned int> m_OrphanedAudioSsrcs;
	std::string videoInputName;
	std::string audioInputName;
	wxArrayString m_ParticipantList;
	ConferenceAnnunciator m_Annunciator;
	VideoPanel* m_VideoPanels[6];
	SenderPipeline* m_pSenderPipeline;
	ReceiverPipeline* m_pReceiverPipeline;
};

#endif // __M4FRAME_HPP__