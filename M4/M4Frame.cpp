///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file M4Frame.cpp
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
/// @brief This file defines the functions of the M4Frame class.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <cstdio>
#include <gst/gst.h>
#include "M4Frame.hpp"
#include "InputsDialog.hpp"
#include "InviteParticipantsDialog.hpp"
#include "StartJoinDialog.hpp"
#include "VideoPanel.hpp"
#include "WaitForInvitationDialog.hpp"

M4Frame::M4Frame()
	: wxFrame(NULL, wxID_ANY, wxT("Video Conferencing System"), wxDefaultPosition, wxSize(400, 150))
	, m_Directory()
	, m_MyAddress("")
	, m_ParticipantByVideoSsrc()
	, m_ParticipantByAudioSsrc()
	, m_OrphanedVideoSsrcs()
	, m_OrphanedAudioSsrcs()
	, videoInputName()
	, audioInputName()
	, m_ParticipantList()
	, m_Annunciator()
	, m_VideoPanels()
	, m_pSenderPipeline(NULL)
	, m_pReceiverPipeline(NULL)
{
	// First, show a dialog to choose video & audio inputs
	InputsDialog* inputs = new InputsDialog(wxT("Choose Inputs"));
	inputs->Centre();
	if (inputs->ShowModal() == wxID_CANCEL)
	{
		wxExit();
	}
	videoInputName = inputs->SelectedVideoInput();
	audioInputName = inputs->SelectedAudioInput();
	inputs->Destroy();
	
	// Next, show a dialog to ask whether we're starting or joining a meeting
	StartJoinDialog* startJoin = new StartJoinDialog(wxT("Start or Join"));
	startJoin->Centre();
	int startOrJoin = startJoin->ShowModal();
	if (startOrJoin == wxID_CANCEL)
	{
		wxExit();
	}
	startJoin->Destroy();
	
	LoadDirectory();
	
	if (startOrJoin == StartJoinDialog::ID_START)
	{
		InviteParticipantsDialog* inviteParticipants = new InviteParticipantsDialog(wxT("Select Participants to Invite"));
		
		wxArrayString availableParticipants;
		for (unsigned int i = 0; i < m_Directory.size(); ++i)
		{
			if (m_Directory[i].address.compare(m_MyAddress) != 0)
			{
				availableParticipants.Add(m_Directory[i].name);
			}
		}
		
		inviteParticipants->SetAvailableParticipants(availableParticipants);
		inviteParticipants->Centre();
		if (inviteParticipants->ShowModal() == wxID_CANCEL)
		{
			wxExit();
		}
		m_ParticipantList = inviteParticipants->GetParticipantList();
		inviteParticipants->Destroy();
		
		// Configure the annunciator to send the participant list, which acts as an invitation
		const char** participantAddresses = new const char*[m_ParticipantList.GetCount() + 1];
		for (size_t i = 0; i < m_ParticipantList.GetCount(); ++i)
		{
			participantAddresses[i] = GetAddressForParticipant(m_ParticipantList[i].c_str());
		}
		participantAddresses[m_ParticipantList.GetCount()] = m_MyAddress.c_str();
		m_Annunciator.SendParticipantList(participantAddresses, m_ParticipantList.GetCount() + 1);
		delete[] participantAddresses;
	}
	else
	{
		// Wait to be invited; get and populate participant list.
		WaitForInvitationDialog* d = new WaitForInvitationDialog(m_Annunciator);
		d->Centre();
		if (d->ShowModal() == wxID_CANCEL)
		{
			wxExit();
		}
		wxArrayString addressList = d->GetAddressList();
		d->Destroy();
		
		// Convert addressList into m_ParticipantList
		m_ParticipantList.Clear();
		for (size_t i = 0; i < addressList.GetCount(); ++i)
		{
			if (strcmp(addressList[i].c_str(), m_MyAddress.c_str()) != 0)
			{
				const std::string* name = GetNameForAddress(std::string(addressList[i].c_str()));
				if (name != NULL)
				{
					m_ParticipantList.Add(*name);
				}
			}
		}
		
		// Configure the annunciator with the participant list
		const char** participantAddresses = new const char*[m_ParticipantList.GetCount()];
		for (size_t i = 0; i < m_ParticipantList.GetCount(); ++i)
		{
			participantAddresses[i] = GetAddressForParticipant(m_ParticipantList[i].c_str());
		}
		m_Annunciator.SetParticipantList(participantAddresses, m_ParticipantList.GetCount());
		delete[] participantAddresses;
	}
	
	// Connect ourselves as the parameter listener
	m_Annunciator.SetParameterPacketListener(this);

	wxBoxSizer* v = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* h1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* h2 = new wxBoxSizer(wxHORIZONTAL);
	
	m_VideoPanels[0] = new VideoPanel(this, "Me");
	h1->Add(m_VideoPanels[0], 1, wxALL | wxEXPAND, 5);
	
	for (size_t i = 0; i < std::min(m_ParticipantList.GetCount(), (size_t)5); ++i)
	{
		m_VideoPanels[i+1] = new VideoPanel(this, m_ParticipantList[i]);
		(((i % 2) != 0) ? h1 : h2)->Add(m_VideoPanels[i+1], 1, wxALL | wxEXPAND, 5);
	}
	
	v->Add(h1, 1, wxALL | wxEXPAND);
	v->Add(h2, 1, wxALL | wxEXPAND);

	SetSizer(v);
	
	// We can't start GStreamer stuff until the main loop is running; so we hook up an
	// idle handler here and do the GStreamer creation the first time that handler is
	// called (from the main loop).
	Connect(wxEVT_IDLE, wxIdleEventHandler(M4Frame::OnIdle));
}

M4Frame::~M4Frame()
{
	// TODO: Deallocate stuff
}

void M4Frame::OnIdle(wxIdleEvent& evt)
{
	if (m_pSenderPipeline == NULL)
	{
		// Create a new sender pipeline with the chosen video and audio inputs and make it play.
		m_pSenderPipeline = new SenderPipeline(videoInputName.c_str(), audioInputName.c_str(), this);
		m_pSenderPipeline->SetBitrate(1000000);
		m_pSenderPipeline->SetWindowSink(m_VideoPanels[0]->GetMediaPanelHandle());
		for (size_t i = 0; i < std::min(m_ParticipantList.GetCount(), (size_t)5); ++i)
		{
			const char* address = GetAddressForParticipant(m_ParticipantList[i]);
			assert(address != NULL);
			m_pSenderPipeline->AddDestination(address);
		}
		
		m_pSenderPipeline->Play();
		
		// Create a new receiver pipeline and make it play; we will use its callbacks to hook
		// up actual video.
		m_pReceiverPipeline = new ReceiverPipeline(this);
		m_pReceiverPipeline->Play();
		
		// Disconnect the idle handler, as we're done now.
		Disconnect(wxEVT_IDLE, wxIdleEventHandler(M4Frame::OnIdle));
	}
}

void M4Frame::OnNewParameters(const SenderPipeline& rPipeline, const char* pPictureParameters, unsigned int videoSsrc, unsigned int audioSsrc)
{
	// Configure conference annunciator to send my parameters to the other participants
	m_Annunciator.SendParameters(pPictureParameters, videoSsrc, audioSsrc);
}

void M4Frame::OnSsrcActivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc)
{
	std::printf("%s SSRC %u activated.\n", (type == ReceiverPipeline::IReceiverNotifySink::SSRC_TYPE_VIDEO) ? "video" : "audio", ssrc);
	
	// Look up the participant by SSRC.
	Participant* p = NULL;
	if (type == ReceiverPipeline::IReceiverNotifySink::SSRC_TYPE_VIDEO)
	{
		p = m_ParticipantByVideoSsrc[ssrc];
	}
	else
	{
		p = m_ParticipantByAudioSsrc[ssrc];
	}
	
	if (p == NULL)
	{
		// Need to keep track of active SSRCs for which we haven't yet received parameters
		// and look them up when we receive parameter packets.
		if (type == ReceiverPipeline::IReceiverNotifySink::SSRC_TYPE_VIDEO)
		{
			m_OrphanedVideoSsrcs.push_back(ssrc);
		}
		else
		{
			m_OrphanedAudioSsrcs.push_back(ssrc);
		}
	}
	else
	{
		if (type == ReceiverPipeline::IReceiverNotifySink::SSRC_TYPE_VIDEO)
		{
			VideoPanel* panel = GetPanelForAddress(p->address);
			assert(panel != NULL);
			rPipeline.ActivateVideoSsrc(ssrc, p->pictureParameters.c_str(), panel->GetMediaPanelHandle());
		}
		else
		{
			rPipeline.ActivateAudioSsrc(ssrc);
		}
	}
}

void M4Frame::OnSsrcDeactivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc, SsrcDeactivateReason reason)
{
	std::printf("%s SSRC %u deactivated due to %s.\n", (type == ReceiverPipeline::IReceiverNotifySink::SSRC_TYPE_VIDEO) ? "video" : "audio", ssrc, (reason == ReceiverPipeline::IReceiverNotifySink::SSRC_DEACTIVATE_REASON_BYE) ? "bye" : ((reason == ReceiverPipeline::IReceiverNotifySink::SSRC_DEACTIVATE_REASON_STOP) ? "stop" : "timeout"));
	if (type == ReceiverPipeline::IReceiverNotifySink::SSRC_TYPE_VIDEO)
	{
		rPipeline.DeactivateVideoSsrc(ssrc);
	}
	else
	{
		rPipeline.DeactivateAudioSsrc(ssrc);
	}
}

void M4Frame::OnParameterPacket(const char* address, const char* pictureParameters, unsigned int videoSsrc, unsigned int audioSsrc)
{
	if (std::strcmp(address, m_MyAddress.c_str()) == 0)
	{
		return;
	}
	
	if (m_ParticipantByVideoSsrc[videoSsrc] != NULL)
	{
		return;
	}
	
	std::printf("Received picture parameters \"%s\", video SSRC %u, audio SSRC %u from %s\n", pictureParameters, videoSsrc, audioSsrc, address);
	
	// Enter this participant in our dictionaries
	Participant* p = new Participant(address, pictureParameters, videoSsrc, audioSsrc);
	m_ParticipantByVideoSsrc[videoSsrc] = p;
	m_ParticipantByAudioSsrc[audioSsrc] = p;
	
	// Check for any ssrcs that have been activated already that we need to turn on
	for (std::vector<unsigned int>::const_iterator cit = m_OrphanedVideoSsrcs.begin(); cit != m_OrphanedVideoSsrcs.end(); ++cit)
	{
		if ((*cit) == videoSsrc)
		{
			VideoPanel* panel = GetPanelForAddress(address);
			assert(panel != NULL);
			m_pReceiverPipeline->ActivateVideoSsrc(videoSsrc, pictureParameters, panel->GetMediaPanelHandle());
			break;
		}
	}
	for (std::vector<unsigned int>::const_iterator cit = m_OrphanedAudioSsrcs.begin(); cit != m_OrphanedAudioSsrcs.end(); ++cit)
	{
		if ((*cit) == audioSsrc)
		{
			m_pReceiverPipeline->ActivateAudioSsrc(audioSsrc);
			break;
		}
	}
}
 
void M4Frame::LoadDirectory()
{
	// TODO: Get these hard-coded values out of here
	m_Directory.push_back(DirectoryEntry("James",   "192.168.117.227"));
//	m_Directory.push_back(DirectoryEntry("Jessica", "192.168.117.133"));
//	m_Directory.push_back(DirectoryEntry("Jimmy",   "192.168.117.134"));
//	m_Directory.push_back(DirectoryEntry("Jocelyn", "192.168.117.135"));
	m_Directory.push_back(DirectoryEntry("Justin",  "192.168.117.205"));
	m_MyAddress = "192.168.117.205";
}

const char* M4Frame::GetAddressForParticipant(const char* name)
{
	std::string needle(name);
	for (std::vector<DirectoryEntry>::const_iterator cit = m_Directory.cbegin(); cit != m_Directory.cend(); ++cit)
	{
		if ((*cit).name.compare(needle) == 0)
		{
			return (*cit).address.c_str();
		}
	}
	return NULL;
}

const std::string* M4Frame::GetNameForAddress(const std::string& address)
{
	for (std::vector<DirectoryEntry>::const_iterator cit = m_Directory.cbegin(); cit != m_Directory.cend(); ++cit)
	{
		if ((*cit).address.compare(address) == 0)
		{
			return &(*cit).name;
		}
	}
	return NULL;
}

VideoPanel* M4Frame::GetPanelForAddress(const std::string& address)
{
	const std::string* pName = GetNameForAddress(address);
	if (pName != NULL)
	{
		for (size_t i = 0; i < m_ParticipantList.GetCount(); ++i)
		{
			if (std::strcmp(pName->c_str(), m_ParticipantList[i].c_str()) == 0)
			{
				return m_VideoPanels[i + 1];
			}
		}
	}
	return NULL;
}