///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file WaitForInvitationDialog.cpp
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
/// @brief This file defines the functions of the WaitForInvitationDialog class, which is a dialog
/// that simply waits to receive an invitation for a meeting.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "WaitForInvitationDialog.hpp"


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor. Build a very basic GUI and wait for a call packet from the annunciator.
///////////////////////////////////////////////////////////////////////////////////////////////////
WaitForInvitationDialog::WaitForInvitationDialog(ConferenceAnnunciator& annunciator)
	: wxDialog(NULL, wxID_ANY, wxT("Waiting for meeting invitation..."), wxDefaultPosition, wxDefaultSize)
	, m_Annunciator(annunciator)
	, m_AddressList()
{
	wxButton* cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel and Exit"), wxDefaultPosition, wxDefaultSize);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(cancelButton, 1, wxALIGN_CENTER);
	SetSizer(hbox);
	
	m_Annunciator.SetCallPacketListener(this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// WaitForInvitationDialog::OnCancel
///
/// Called if the user clicks "Cancel"; dismissed the dialog unsuccessfully.
///////////////////////////////////////////////////////////////////////////////////////////////////
void WaitForInvitationDialog::OnCancel(wxCommandEvent &event)
{
	m_Annunciator.ClearCallPacketListener();
	EndModal(wxID_CANCEL);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// WaitForInvitationDialog::OnCallPacket
///
/// Called when the annunciator receives a call packet; saves the list of participant addresses and
/// dismisses the dialog successfully.
///////////////////////////////////////////////////////////////////////////////////////////////////
void WaitForInvitationDialog::OnCallPacket(const char* participantList[], size_t numberOfParticipants)
{
	m_AddressList.Clear();
	for (size_t i = 0; i < numberOfParticipants; ++i)
	{
		m_AddressList.Add(participantList[i]);
	}
	
	m_Annunciator.ClearCallPacketListener();
	EndModal(wxID_OK);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// The event table.
///////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(WaitForInvitationDialog, wxDialog)
	EVT_BUTTON(wxID_CANCEL, WaitForInvitationDialog::OnCancel)
END_EVENT_TABLE()