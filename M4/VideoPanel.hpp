///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file VideoPanel.hpp
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
/// @brief This file declares the VideoPanel class, which is a panel displaying a video player
/// (either for "self" or another participant).
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __VIDEOPANEL_HPP__
#define __VIDEOPANEL_HPP__

#include <wx/wx.h>

class VideoPanel : public wxPanel
{
public:
	VideoPanel(wxFrame* parent, const wxString &participantName);
	
	inline void* GetMediaPanelHandle() const { return m_MediaPanel->GetHandle(); }
	
private:
	static const wxSize DEFAULT_MEDIA_PANEL_SIZE;
	wxPanel* const m_MediaPanel;
};

#endif // __VIDEOPANEL_HPP__