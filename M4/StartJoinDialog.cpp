///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file StartJoinDialog.cpp
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
/// @brief This file declares the StartJoinDialog class, which is a dialog for asking the user if
/// they're starting or joining a call.
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "StartJoinDialog.hpp"


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor. Just builds the GUI.
///////////////////////////////////////////////////////////////////////////////////////////////////
StartJoinDialog::StartJoinDialog(const wxString & title)
	: wxDialog(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(250, 160))
{
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxButton *startButton = new wxButton(this, ID_START, wxT("Start a new call"), wxDefaultPosition, wxSize(200, 40));
	vbox->Add(startButton, 1, wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL);
	wxButton *joinButton = new wxButton(this, ID_JOIN, wxT("Join an existing call"), wxDefaultPosition, wxSize(200, 40));
	vbox->Add(joinButton, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL);
	wxButton *cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel and exit"), wxDefaultPosition, wxSize(200, 40));
	vbox->Add(cancelButton, 1, wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL);

	SetSizer(vbox);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// StartJoinDialog::OnStart
///
/// End the dialog with the "Start" ID.
///////////////////////////////////////////////////////////////////////////////////////////////////
void StartJoinDialog::OnStart(wxCommandEvent &event)
{
	EndModal(ID_START);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// StartJoinDialog::OnJoin
///
/// End the dialog with the "Join" ID.
///////////////////////////////////////////////////////////////////////////////////////////////////
void StartJoinDialog::OnJoin(wxCommandEvent &event)
{
	EndModal(ID_JOIN);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// StartJoinDialog::OnCancel
///
/// End the dialog with the "Cancel" ID.
///////////////////////////////////////////////////////////////////////////////////////////////////
void StartJoinDialog::OnCancel(wxCommandEvent &event)
{
	EndModal(wxID_CANCEL);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// The event table.
///////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(StartJoinDialog, wxDialog)
	EVT_BUTTON(ID_START,    StartJoinDialog::OnStart)
	EVT_BUTTON(ID_JOIN,     StartJoinDialog::OnJoin)
	EVT_BUTTON(wxID_CANCEL, StartJoinDialog::OnCancel)
END_EVENT_TABLE()