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

class InviteParticipantsDialog : public wxDialog
{
public:
	InviteParticipantsDialog(const wxString& title);
	
	void OnOk(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
	void OnSelect(wxCommandEvent &event);
	
	void SetAvailableParticipants(const wxArrayString& list);
	wxArrayString GetParticipantList() const;
	
protected:
	DECLARE_EVENT_TABLE()
	
private:
	wxListBox* m_ParticipantListBox;
	wxButton* m_OkButton;
};

#endif // __INVITEPARTICIPANTSDIALOG_HPP__