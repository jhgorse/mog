///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file InputsDialog.hpp
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
#ifndef __INPUTSDIALOG_HPP__
#define __INPUTSDIALOG_HPP__

#include <wx/wx.h>

class AVList;

class InputsDialog : public wxDialog
{
public:
	InputsDialog(const wxString& title);
	
	void OnOk(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
	void OnAVChoose(wxCommandEvent &event);
	
	std::string SelectedVideoInput() const;
	std::string SelectedAudioInput() const;
	
protected:
	enum
	{
		ID_VIDEO_CHOICE = wxID_HIGHEST + 1,
		ID_AUDIO_CHOICE,
	};
	
	DECLARE_EVENT_TABLE()
	
private:
	wxButton* m_OkButton;
	wxChoice* m_VideoChoice;
	wxChoice* m_AudioChoice;
};

#endif // __INPUTSDIALOG_HPP__