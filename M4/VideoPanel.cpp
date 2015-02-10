///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file VideoPanel.cpp
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
/// @brief This file defines the functions of the VideoPanel class, which is a panel displaying a
/// video player (either for "self" or another participant).
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "VideoPanel.hpp"

/// For now, we hard-code this to a 16:9 aspect ratio, and a size that *should* fit in most cases.
/// The user can always resize the window.
const wxSize VideoPanel::DEFAULT_MEDIA_PANEL_SIZE(128, 72);


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor. Display a label with the participant's name, followed by the display panel.
///////////////////////////////////////////////////////////////////////////////////////////////////
VideoPanel::VideoPanel(wxFrame* parent, const wxString &participantName)
	: wxPanel(parent, wxID_ANY)
	, m_MediaPanel(new wxPanel(this, wxID_ANY, wxDefaultPosition, DEFAULT_MEDIA_PANEL_SIZE))
{
	m_MediaPanel->SetForegroundColour("white");
	m_MediaPanel->SetBackgroundColour("black");
	
	wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
	wxString label = participantName.Clone();
	label << ":";
	wxStaticText* text = new wxStaticText(this, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	s->Add(text, 0, wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL);
	s->Add(m_MediaPanel, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxSHAPED);
	SetSizer(s);
}