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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class lists the available A/V input sources, allowing the user to choose inputs.
///////////////////////////////////////////////////////////////////////////////////////////////////
class InputsDialog : public wxDialog
{
public:
	/// Constructor.
	InputsDialog(const wxString& title);
	
	
	/// Called when "Ok" is clicked.
	void OnOk(wxCommandEvent &event);
	
	
	/// Called when "Cancel" is clicked.
	void OnCancel(wxCommandEvent &event);
	
	
	/// Called when the A/V boxes are changed.
	void OnAVChoose(wxCommandEvent &event);
	
	
	/// Accessor for the selected video input name.
	std::string SelectedVideoInput() const;
	
	
	/// Accessor for the selected audio input name.
	std::string SelectedAudioInput() const;
	
protected:

	/// Private event IDs.
	enum
	{
		ID_VIDEO_CHOICE = wxID_HIGHEST + 1,
		ID_AUDIO_CHOICE,
	};
	
	DECLARE_EVENT_TABLE()
	
private:
	/// The Ok button.
	wxButton* m_OkButton;
	
	
	/// The available video input devices.
	wxChoice* m_VideoChoice;
	
	
	/// The available audio input devices.
	wxChoice* m_AudioChoice;
};

#endif // __INPUTSDIALOG_HPP__