///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file InviteParticipantsDialog.vpp
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
/// @brief This file declares the InviteParticipantsDialog class, which is a dialog for asking the
/// user to invite participants to a call.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "InviteParticipantsDialog.hpp"


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InviteParticipantsDialog::InviteParticipantsDialog
///
/// Constructor. Just builds the GUI.
///////////////////////////////////////////////////////////////////////////////////////////////////
InviteParticipantsDialog::InviteParticipantsDialog(const wxString & title)
	: wxDialog(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400, 600))
	, m_ParticipantListBox(NULL)
	, m_OkButton(NULL)
{
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	
	m_ParticipantListBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(390, 550), 0, NULL, wxLB_MULTIPLE | wxLB_SORT);
	vbox->Add(m_ParticipantListBox, 1, wxALIGN_CENTER | wxTOP | wxBOTTOM);
	
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_OkButton = new wxButton(this, wxID_OK, wxT("Ok"), wxDefaultPosition, wxSize(70, 30));
	m_OkButton->Disable();
	wxButton *cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxSize(70, 30));
	hbox->Add(m_OkButton, 1);
	hbox->Add(cancelButton, 1, wxLEFT);
	
	vbox->Add(hbox, 1, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	SetSizer(vbox);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InviteParticipantsDialog::OnOk
///
/// Close the dialog in a successful way.
///////////////////////////////////////////////////////////////////////////////////////////////////
void InviteParticipantsDialog::OnOk(wxCommandEvent &event)
{
	EndModal(wxID_OK);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InviteParticipantsDialog::OnCancel
///
/// Close the dialog in an unsuccessful way.
///////////////////////////////////////////////////////////////////////////////////////////////////
void InviteParticipantsDialog::OnCancel(wxCommandEvent &event)
{
	EndModal(wxID_CANCEL);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InviteParticipantsDialog::OnSelect
///
/// Called when the selection changes; allows us to conditionally enable the "Ok" button.
///////////////////////////////////////////////////////////////////////////////////////////////////
void InviteParticipantsDialog::OnSelect(wxCommandEvent &event)
{
	for (unsigned int i = 0; i < m_ParticipantListBox->GetCount(); ++i)
	{
		if (m_ParticipantListBox->IsSelected(i))
		{
			m_OkButton->Enable();
			return;
		}
	}
	m_OkButton->Disable();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InviteParticipantsDialog::SetAvailableParticipants
///
/// Set the list of allowed participants in the dialog box. Should be called after construction,
/// but before display.
///////////////////////////////////////////////////////////////////////////////////////////////////
void InviteParticipantsDialog::SetAvailableParticipants(const wxArrayString& list)
{
	for (unsigned int i = 0; i < list.GetCount(); ++i)
	{
		m_ParticipantListBox->Append(wxString(list[i]));
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InviteParticipantsDialog::GetParticipantList
///
/// Get an array of selected participants.
///////////////////////////////////////////////////////////////////////////////////////////////////
wxArrayString InviteParticipantsDialog::GetParticipantList() const
{
	wxArrayString list;
	for (unsigned int i = 0; i < m_ParticipantListBox->GetCount(); ++i)
	{
		if (m_ParticipantListBox->IsSelected(i))
		{
			list.Add(wxString(m_ParticipantListBox->GetString(i)));
		}
	}
	return list;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// The event table.
///////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(InviteParticipantsDialog, wxDialog)
	EVT_BUTTON(wxID_OK,     InviteParticipantsDialog::OnOk)
	EVT_BUTTON(wxID_CANCEL, InviteParticipantsDialog::OnCancel)
	EVT_LISTBOX(wxID_ANY,   InviteParticipantsDialog::OnSelect)
END_EVENT_TABLE()