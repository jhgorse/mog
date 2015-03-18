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
#include "ReceiverPipeline.hpp"
#include "StartJoinDialog.hpp"
#include "VideoPanel.hpp"
#include "WaitForInvitationDialog.hpp"


const char M4Frame::DIRECTORY_FILENAME[] = "directory.json";

const size_t M4Frame::VIDEO_BITRATE = 50000000;


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
	, m_ReceiverPipelinesByVideoSsrc()
	, videoInputName()
	, audioInputName()
	, m_ParticipantList()
	, m_Annunciator()
	, m_VideoPanels()
	, m_pSenderPipeline(NULL)
  , v()
  , h1()
  , h2()
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
		m_ParticipantList.Add(GetNameForAddress(m_MyAddress)->c_str());
		
		// Configure the annunciator to send the participant list, which acts as an invitation
		const char** participantAddresses = new const char*[m_ParticipantList.GetCount()];
		for (size_t i = 0; i < m_ParticipantList.GetCount(); ++i)
		{
			participantAddresses[i] = GetAddressForParticipant(m_ParticipantList[i].c_str());
		}
		m_Annunciator.SendParticipantList(participantAddresses, m_ParticipantList.GetCount());
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
			const std::string* name = GetNameForAddress(std::string(addressList[i].c_str()));
			if (name != NULL)
			{
				m_ParticipantList.Add(*name);
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

  // Init GUI variables
  v = new wxBoxSizer(wxHORIZONTAL);
  h1 = new wxBoxSizer(wxVERTICAL);
  h2 = new wxBoxSizer(wxVERTICAL);
  selectedMediaPanelId = 0;
  
	m_VideoPanels[0] = new VideoPanel(this, "Me");
  m_VideoPanels[0]->m_MediaPanel->Bind(wxEVT_LEFT_UP, &M4Frame::OnClick, this);
  //std::cout  << "m_VideoPanels Id " << 0 << " " << m_VideoPanels[0]->m_MediaPanel->GetId() << std::endl;
	
	const std::string* myName = GetNameForAddress(m_MyAddress);
	for (size_t i = 1, j=0; j < std::min(m_ParticipantList.GetCount(), 6UL); ++j)
	{
		if (myName->compare(m_ParticipantList[j]) != 0)
		{
			m_VideoPanels[i] = new VideoPanel(this, m_ParticipantList[j]);
      m_VideoPanels[i]->m_MediaPanel->Bind(wxEVT_LEFT_UP, &M4Frame::OnClick, this);
      //std::cout  << "m_VideoPanels Id " << i << " " << m_VideoPanels[i]->m_MediaPanel->GetId() << std::endl;
			++i;
		}
	}
	
  SetView(0);
	SetSizer(v);
	
	// We can't start GStreamer stuff until the main loop is running; so we hook up an
	// idle handler here and do the GStreamer creation the first time that handler is
	// called (from the main loop).
	Connect(wxEVT_IDLE, wxIdleEventHandler(M4Frame::OnIdle));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::OnClick
///
/// Handle mouse click release
///
/// @param event  wxCommandEvent passed from child
///////////////////////////////////////////////////////////////////////////////////////////////////

void M4Frame::OnClick(wxMouseEvent& event)
{
  //std::cout  << "OnClick reached M4Frame " << event.GetId() << std::endl;
  SetView(event.GetId());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::SetView
///
/// Handle layout of the video panels
///
/// @param mediaPanelId  Selected "m_MediaPanel" Id. Set to 0 for no selection.
///////////////////////////////////////////////////////////////////////////////////////////////////

void M4Frame::SetView(int mediaPanelId)
{
  //std::cout  << "SetView  mediaPanelId: " << mediaPanelId
  //           << "  selectedMediaPanelId: " << selectedMediaPanelId << std::endl;
  
  // Detach all children from sizers
  h1->Clear(false);
  h2->Clear(false);
  v->Detach(h1);
  v->Detach(h2);
  
  if ((mediaPanelId == selectedMediaPanelId)
      || (mediaPanelId == 0))
  {
    // Set view to two rows, equal spaced
    v->SetOrientation(wxVERTICAL);
    h1->SetOrientation(wxHORIZONTAL);
    h2->SetOrientation(wxHORIZONTAL);
    
    for (size_t i = 0; i < std::min(m_ParticipantList.GetCount(), 6UL); ++i)
    {
      (((i % 2) == 0) ? h1 : h2)->Add(m_VideoPanels[i], 1, wxALL | wxEXPAND, 5);
    }
    v->Add(h1, 1, wxALL | wxEXPAND);
    v->Add(h2, 1, wxALL | wxEXPAND);
    
    selectedMediaPanelId = 0;
  }
  else
  {
    // Maximize the selected media panel
    v->SetOrientation(wxHORIZONTAL);
    h1->SetOrientation(wxVERTICAL);
    h2->SetOrientation(wxVERTICAL);
    
    for (size_t i = 0; i < std::min(m_ParticipantList.GetCount(), 6UL); ++i)
    {
      if (mediaPanelId == m_VideoPanels[i]->m_MediaPanel->GetId()) // Maximize this Id
      {
        h2->Add(m_VideoPanels[i], 1, wxALL | wxEXPAND, 5);
      }
      else // Iconize these Ids
      {
        h1->Add(m_VideoPanels[i], 1, wxALL | wxEXPAND, 5);
      }
    }
    v->Add(h1, 1, wxLEFT | wxTOP | wxBOTTOM | wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    v->Add(h2, 4, wxALL | wxEXPAND);
    
    selectedMediaPanelId = mediaPanelId;
  }

  Layout(); // Redraw
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// M4Frame::~M4Frame
///
/// Destructor. Free allocated memory/stuff.
///////////////////////////////////////////////////////////////////////////////////////////////////
M4Frame::~M4Frame()
{
	if (m_pSenderPipeline != NULL)
	{
		delete m_pSenderPipeline;
	}
	
	for (std::unordered_map<unsigned int, ReceiverPipeline*>::iterator it = m_ReceiverPipelinesByVideoSsrc.begin(); it != m_ReceiverPipelinesByVideoSsrc.end(); ++it)
	{
		delete it->second;
	}
	
	for (size_t i = 0; i < (sizeof(m_VideoPanels) / sizeof(m_VideoPanels[0])); ++i)
	{
		delete m_VideoPanels[i];
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
	
	// Find "my" index in the participant list; this is the index we use for the port offset for sending.
	const std::string* myName = GetNameForAddress(m_MyAddress);
	size_t myIndex = 0;
	for (size_t i = 0; i < m_ParticipantList.GetCount(); ++i)
	{
		if (myName->compare(m_ParticipantList[i]) == 0)
		{
			myIndex = i;
			break;
		}
	}
	
	// Now add destinations for each of the participants.
	for (size_t i = 0; i < m_ParticipantList.GetCount(); ++i)
	{
		if (myName->compare(m_ParticipantList[i]) != 0)
		{
			const char* address = GetAddressForParticipant(m_ParticipantList[i]);
			assert(address != NULL);
			m_pSenderPipeline->AddDestination(address, 10000 + 4 * myIndex);
		}
	}
	m_pSenderPipeline->Play();
	
	// Connect ourselves as the parameter listener
	m_Annunciator.SetParameterPacketListener(this);
	
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
	
	// Ignore if we already have a receiver pipeline
	if (m_ReceiverPipelinesByVideoSsrc.find(videoSsrc) != m_ReceiverPipelinesByVideoSsrc.end())
	{
		return;
	}
	
	// See if this is in our dictionary; ignore if not.
	size_t index;
	VideoPanel* pPanel = GetPanelForAddress(address, index);
	if (pPanel == NULL)
	{
		return;
	}
	
	// We might have had a receiver pipeline from a previous SSRC from the same sender. Look
	// through the pipelines to see if any match the video sink, and if so, delete them.
	for (std::unordered_map<unsigned int, ReceiverPipeline*>::iterator it = m_ReceiverPipelinesByVideoSsrc.begin(); it != m_ReceiverPipelinesByVideoSsrc.end(); ++it)
	{
		if (it->second->GetWindowSink() == pPanel->GetMediaPanelHandle())
		{
			delete it->second;
			m_ReceiverPipelinesByVideoSsrc.erase(it);
			break;
		}
	}
	
	// If we get here, then there is an entry for this sender in the directory, but no receiver
	// pipeline yet. Create it now.
	ReceiverPipeline* pPipeline = new ReceiverPipeline(10000 + 4 * index, audioInputName.c_str(), pictureParameters, pPanel->GetMediaPanelHandle());
	m_ReceiverPipelinesByVideoSsrc[videoSsrc] = pPipeline;
	pPipeline->Play();
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
VideoPanel* M4Frame::GetPanelForAddress(const std::string& address, size_t& rIndexOut)
{
	const std::string* pName = GetNameForAddress(address);
	if (pName != NULL)
	{
		const std::string* myName = GetNameForAddress(m_MyAddress);
		for (size_t i = 0, j = 1; i < m_ParticipantList.GetCount(); ++i)
		{
			if (myName->compare(m_ParticipantList[i]) != 0)
			{
				if (std::strcmp(pName->c_str(), m_ParticipantList[i].c_str()) == 0)
				{
					rIndexOut = i;
					return m_VideoPanels[j];
				}
				j++;
			}
		}
	}
	return NULL;
}