///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file InviteParticipantsDialog.hpp
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
/// user to invite participants.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __INVITEPARTICIPANTSDIALOG_HPP__
#define __INVITEPARTICIPANTSDIALOG_HPP__

#include <wx/wx.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
/// InviteParticipantsDialog
///
/// This class is a dialog box used to present a list of participants for a conference.
///////////////////////////////////////////////////////////////////////////////////////////////////
class InviteParticipantsDialog : public wxDialog
{
public:
	/// Constructor.
	InviteParticipantsDialog(const wxString& title);
	
	
	/// Called when "Ok" is clicked.
	void OnOk(wxCommandEvent &event);
	
	
	/// Called when "Cancel" is clicked.
	void OnCancel(wxCommandEvent &event);
	
	
	/// Called when the participant selection is changed.
	void OnSelect(wxCommandEvent &event);
	
	
	/// Configure the list of available participants.
	void SetAvailableParticipants(const wxArrayString& list);
	
	
	/// Get the list of selected participants.
	wxArrayString GetParticipantList() const;
	
	
protected:
	DECLARE_EVENT_TABLE()
	
private:
	/// The available participant list box.
	wxListBox* m_ParticipantListBox;
	
	
	/// The "Ok" button
	wxButton* m_OkButton;
};

#endif // __INVITEPARTICIPANTSDIALOG_HPP__