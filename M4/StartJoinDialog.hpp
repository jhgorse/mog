///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file StartJoinDialog.hpp
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
/// @brief This file declares the StartJoinDialog class, which is a dialog for asking the user
/// whether they are starting or joining a call.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __STARTJOINDIALOG_HPP__
#define __STARTJOINDIALOG_HPP__

#include <wx/wx.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
/// This is the world's simplest dialog box, asking the user if they want to start or join a call.
///////////////////////////////////////////////////////////////////////////////////////////////////
class StartJoinDialog : public wxDialog
{
public:
	/// Event IDs.
	enum
	{
		ID_START = wxID_HIGHEST + 1,
		ID_JOIN,
	};
	
	
	/// Constructor.
	StartJoinDialog(const wxString& title);
	
	
	/// Called when "Start" is clicked.
	void OnStart(wxCommandEvent &event);
	
	
	/// Called when "Join" is clicked.
	void OnJoin(wxCommandEvent &event);
	
	
	/// Called when "Cancel" is clicked.
	void OnCancel(wxCommandEvent &event);
	
	
protected:
	DECLARE_EVENT_TABLE()
	
private:
};

#endif // __STARTJOINDIALOG_HPP__