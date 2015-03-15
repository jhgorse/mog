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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class is a thin shim around a wxPanel that allows it to display video from GStreamer.
///////////////////////////////////////////////////////////////////////////////////////////////////
class VideoPanel : public wxPanel
{
public:
	/// Constructor.
	VideoPanel(wxFrame* parent, const wxString &participantName);
	
	
	/// Return the native window handle for this panel.
	inline void* GetMediaPanelHandle() const { return m_MediaPanel->GetHandle(); }
	
	
  /// The inner video panel.
  wxPanel* const m_MediaPanel;
  
  // Handle mouse clicks
  void OnClick(wxMouseEvent& event);
  
private:
	/// The default size.
	static const wxSize DEFAULT_MEDIA_PANEL_SIZE;
	
	
};

#endif // __VIDEOPANEL_HPP__