///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file InputsDialog.vpp
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
/// @brief This file declares the InputsDialog class, which is a dialog for allowing the user to
/// choose video and audio inputs.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "AVList.hpp"
#include "InputsDialog.hpp"


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor.
///
/// Present the user with a choice of A/V input devices obtained from the AVListEnumerator.
///////////////////////////////////////////////////////////////////////////////////////////////////
InputsDialog::InputsDialog(const wxString & title)
	: wxDialog(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(250, 160))
	, m_OkButton(NULL)
	, m_VideoChoice(NULL)
{
	// Iterate available devices.
	const AVList& rAVList = AVListEnumerator::List();
	
	// Build the GUI.
	wxPanel *panel = new wxPanel(this);
	new wxStaticBox(panel, wxID_ANY, wxT("Choose video and audio inputs:"), wxPoint(5, 5), wxSize(240, 86));
	new wxStaticText(panel, wxID_ANY, wxT("Video:"), wxPoint(15, 30), wxDefaultSize, wxALIGN_LEFT);
	wxArrayString videoChoices;
	for (AVList::VideoInputList::const_iterator cit = rAVList.VideoInputs().cbegin(); cit != rAVList.VideoInputs().cend(); ++cit)
	{
		videoChoices.Add(cit->Name());
	}
	m_VideoChoice = new wxChoice(panel, ID_VIDEO_CHOICE, wxPoint(55, 25), wxSize(180, 25), videoChoices, wxCB_SORT);
	m_VideoChoice->SetSelection(wxNOT_FOUND);
	new wxStaticText(panel, wxID_ANY, wxT("Audio:"), wxPoint(15, 55), wxDefaultSize, wxALIGN_LEFT);
	wxArrayString audioChoices;
	for (AVList::AudioInputList::const_iterator cit = rAVList.AudioInputs().cbegin(); cit != rAVList.AudioInputs().cend(); ++cit)
	{
		audioChoices.Add(cit->Name());
	}
	m_AudioChoice = new wxChoice(panel, ID_AUDIO_CHOICE, wxPoint(55, 50), wxSize(180, 25), audioChoices, wxCB_SORT);
	m_AudioChoice->SetSelection(wxNOT_FOUND);
	
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_OkButton = new wxButton(this, wxID_OK, wxT("Ok"), wxDefaultPosition, wxSize(70, 30));
	m_OkButton->Disable();
	wxButton *cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxSize(70, 30));
	hbox->Add(m_OkButton, 1);
	hbox->Add(cancelButton, 1, wxLEFT, 5);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	vbox->Add(panel, 1);
	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	SetSizer(vbox);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InputsDialog::OnOk
///
/// Close the dialog box in a successful way.
///////////////////////////////////////////////////////////////////////////////////////////////////
void InputsDialog::OnOk(wxCommandEvent &event)
{
	EndModal(wxID_OK);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InputsDialog::OnCancel
///
/// Close the dialog box in an unsuccessful way.
///////////////////////////////////////////////////////////////////////////////////////////////////
void InputsDialog::OnCancel(wxCommandEvent &event)
{
	EndModal(wxID_CANCEL);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InputsDialog::OnAVChoose
///
/// Called when the A/V choices change; allows us to conditionally make the Ok button clickable.
///////////////////////////////////////////////////////////////////////////////////////////////////
void InputsDialog::OnAVChoose(wxCommandEvent &event)
{
	m_OkButton->Enable(
		(m_VideoChoice->GetSelection() != wxNOT_FOUND) &&
		(m_AudioChoice->GetSelection() != wxNOT_FOUND)
	);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InputsDialog::SelectedVideoInput
///
/// Returns a string of the selected video input, or an empty string if none is chosen.
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string InputsDialog::SelectedVideoInput() const
{
	int idx;
	if ((idx = m_VideoChoice->GetSelection()) != wxNOT_FOUND)
	{
		return m_VideoChoice->GetString(idx).ToStdString();
	}
	else
	{
		return "";
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InputsDialog::SelectedAudioInput
///
/// Returns a string of the selected audio input, or an empty string if none is chosen.
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string InputsDialog::SelectedAudioInput() const
{
	int idx;
	if ((idx = m_AudioChoice->GetSelection()) != wxNOT_FOUND)
	{
		return m_AudioChoice->GetString(idx).ToStdString();
	}
	else
	{
		return "";
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// The event table.
///////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(InputsDialog, wxDialog)
	EVT_BUTTON(wxID_OK,         InputsDialog::OnOk)
	EVT_BUTTON(wxID_CANCEL,     InputsDialog::OnCancel)
	EVT_CHOICE(ID_VIDEO_CHOICE, InputsDialog::OnAVChoose)
	EVT_CHOICE(ID_AUDIO_CHOICE, InputsDialog::OnAVChoose)
END_EVENT_TABLE()