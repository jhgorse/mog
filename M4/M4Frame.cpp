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

#include <gst/gst.h>
#include <fstream>

#include "rapidjson/document.h"

#include "M4Frame.hpp"
#include "InputsDialog.hpp"
#include "InviteParticipantsDialog.hpp"
#include "StartJoinDialog.hpp"
#include "VideoPanel.hpp"
#include "WaitForInvitationDialog.hpp"


const char M4Frame::DIRECTORY_FILENAME[] = "directory.json";

const size_t M4Frame::VIDEO_BITRATE = 10000000;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::M4Frame
///
/// Constructor. Walks through the startup workflow (via various dialogs) before displaying the
/// main frame.
///////////////////////////////////////////////////////////////////////////////////////////////////
M4Frame::M4Frame()
	: wxFrame(NULL, wxID_ANY, wxT("Video Conferencing System"), wxDefaultPosition, wxSize(400, 600))
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
	std::memset(m_VideoPanels, 0, sizeof(m_VideoPanels));
	
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
	
	// Load up the participant directory.
	LoadDirectory();
	
	if (startOrJoin == StartJoinDialog::ID_START)
	{
		// If we're starting a meeting, create a dialog to allow the selection of participants.
		InviteParticipantsDialog* inviteParticipants = new InviteParticipantsDialog(wxT("Select Participants to Invite"));
		
		// Configure the dialog with everyone in the directory except "me".
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::~M4Frame
///
/// Destructor. Free allocated memory/stuff.
///////////////////////////////////////////////////////////////////////////////////////////////////
M4Frame::~M4Frame()
{
	// Deallocate stuff
	for (size_t i = 0; i < (sizeof(m_VideoPanels) / sizeof(m_VideoPanels[0])); ++i)
	{
		delete m_VideoPanels[i];
	}

	for (std::unordered_map<unsigned int, Participant*>::iterator it = m_ParticipantByVideoSsrc.begin(); it != m_ParticipantByVideoSsrc.end(); ++it)
	{
		delete it->second;
	}

	if (m_pSenderPipeline != NULL)
	{
		delete m_pSenderPipeline;
		m_pSenderPipeline = NULL;
	}
	
	if (m_pReceiverPipeline != NULL)
	{
		delete m_pReceiverPipeline;
		m_pReceiverPipeline = NULL;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::OnIdle
///
/// Called when there is idle time from the main GUI thread.
///
/// If we try to start GStreamer threads before the application's "main" event loop thread is
/// running, then GStreamer will try to start its own "main" thread, and things won't work right.
/// Hence, we do all the GStreamer startup from the "main" event loop idle time.
///////////////////////////////////////////////////////////////////////////////////////////////////
void M4Frame::OnIdle(wxIdleEvent& evt)
{
	assert(m_pSenderPipeline == NULL);

	// Create a new sender pipeline with the chosen video and audio inputs and make it play.
	m_pSenderPipeline = new SenderPipeline(videoInputName.c_str(), audioInputName.c_str(), this);
	m_pSenderPipeline->SetBitrate(VIDEO_BITRATE);
	m_pSenderPipeline->SetWindowSink(m_VideoPanels[0]->GetMediaPanelHandle());
	for (size_t i = 0; i < std::min(m_ParticipantList.GetCount(), (size_t)5); ++i)
	{
		const char* address = GetAddressForParticipant(m_ParticipantList[i]);
		assert(address != NULL);
		m_pSenderPipeline->AddDestination(address);
	}
	m_pSenderPipeline->Play();
	
	assert(m_pReceiverPipeline == NULL);

	// Create a new receiver pipeline and make it play; we will use its callbacks to hook
	// up actual video.
	m_pReceiverPipeline = new ReceiverPipeline(this);
	m_pReceiverPipeline->Play();
	
	// Disconnect the idle handler, as we're done now.
	Disconnect(wxEVT_IDLE, wxIdleEventHandler(M4Frame::OnIdle));
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::OnNewParameters
///
/// Called by the sender pipeline when the sender parameters are available. Configure the
/// annunciator to transmit these parameters to other participants.
///
/// @param rPipeline  The sender pipeline.
///
/// @param pPictureParameters  Picture parameters string.
///
/// @param videoSsrc  The video SSRC.
///
/// @param audioSsrc  The audio SSRC.
///////////////////////////////////////////////////////////////////////////////////////////////////
void M4Frame::OnNewParameters(const SenderPipeline& rPipeline, const char* pPictureParameters, unsigned int videoSsrc, unsigned int audioSsrc)
{
	m_Annunciator.SendParameters(pPictureParameters, videoSsrc, audioSsrc);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::OnSsrcActivate
///
/// Called by the receiver pipeline when an SSRC becomes active.
///
/// @param rPipeline  The receiver pipeline.
///
/// @param type  The SSRC type (video or audio).
///
/// @param ssrc  The SSRC.
///////////////////////////////////////////////////////////////////////////////////////////////////
void M4Frame::OnSsrcActivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc)
{
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
		// Already received parameters, so activate in the receiver pipeline.
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::OnSsrcDeactivate
///
/// Called by the receiver pipeline when an SSRC becomes inactive.
///
/// @param rPipeline  The receiver pipeline.
///
/// @param type  The SSRC type (video or audio).
///
/// @param ssrc  The SSRC.
///
/// @param reason  The reason the SSRC became inactive.
///////////////////////////////////////////////////////////////////////////////////////////////////
void M4Frame::OnSsrcDeactivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc, SsrcDeactivateReason reason)
{
	Participant* p = NULL;
	if (type == ReceiverPipeline::IReceiverNotifySink::SSRC_TYPE_VIDEO)
	{
		p = m_ParticipantByVideoSsrc[ssrc];
	}
	else
	{
		p = m_ParticipantByAudioSsrc[ssrc];
	}
	if (p != NULL)
	{
		rPipeline.DeactivateVideoSsrc(p->videoSsrc);
		rPipeline.DeactivateAudioSsrc(p->audioSsrc);
		
		m_ParticipantByVideoSsrc.erase(p->videoSsrc);
		m_ParticipantByAudioSsrc.erase(p->audioSsrc);
		
		delete p;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::OnParameterPacket
///
/// Called by the annunciator when a parameter packet is received.
///
/// @param address  The address from which the parameters were received.
///
/// @param pictureParameters  The picture parameters string.
///
/// @param videoSsrc  The video SSRC.
///
/// @param audioSsrc  The audio SSRC.
///////////////////////////////////////////////////////////////////////////////////////////////////
void M4Frame::OnParameterPacket(const char* address, const char* pictureParameters, unsigned int videoSsrc, unsigned int audioSsrc)
{
	// Ignore if I got this from myself.
	if (std::strcmp(address, m_MyAddress.c_str()) == 0)
	{
		return;
	}
	
	// Ignore if we already have an active partipant with this SSRC.
	if (m_ParticipantByVideoSsrc[videoSsrc] != NULL)
	{
		return;
	}
	
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::LoadDirectory
///
/// Load up the directory of participants.
///////////////////////////////////////////////////////////////////////////////////////////////////
void M4Frame::LoadDirectory()
{
	// Read the directory file into memory
	std::ifstream s;
	size_t length;
	s.open(DIRECTORY_FILENAME);
	s.seekg(0, std::ios::end);
	length = s.tellg();
	s.seekg(0, std::ios::beg);
	char* buffer = new char[length + 1];
	s.read(buffer, length);
	s.close();
	buffer[length] = '\0';

	// Parse the JSON data in the directory
	rapidjson::Document d;
	d.Parse(buffer);
	
	// Set my address
	m_MyAddress = std::string(d["me"].GetString());
	
	// Add participants from list
	for (size_t i = 0; i < d["participants"].Size(); ++i)
	{
		m_Directory.push_back(DirectoryEntry(std::string(d["participants"][i]["name"].GetString()), std::string(d["participants"][i]["address"].GetString())));
	}
	
	delete[] buffer;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::GetAddressForParticipant
///
/// Get the address for a participant by name.
///
/// @param name  The name of the participant whose address should be looked up.
///
/// @return  A NULL-terminated address, or NULL if the name could not be found.
///////////////////////////////////////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::GetNameForAddress
///
/// Get the name for a participant's address.
///
/// @param address  The address of the participant whose name should be looked up.
///
/// @return  A std::string pointer, or NULL if the address could not be found.
///////////////////////////////////////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::GetPanelForAddress
///
/// Get the video panel for a participant's address.
///
/// @param address  The address of the participant whose video panel should be looked located.
///
/// @return  A VideoPanel pointer, or NULL if the address could not be found.
///////////////////////////////////////////////////////////////////////////////////////////////////
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