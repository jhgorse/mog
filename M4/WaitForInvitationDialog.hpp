///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file WaitForInvitationDialog.hpp
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
/// @brief This file declares the WaitForInvitationDialog class, which is a dialog that simply
/// waits to receive an invitation for a meeting.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __WAITFORINVITATIONDIALOG_HPP__
#define __WAITFORINVITATIONDIALOG_HPP__

#include <wx/wx.h>
#include "ConferenceAnnunciator.hpp"

class WaitForInvitationDialog
	: public wxDialog
	, protected ConferenceAnnunciator::ICallPacketListener
{
public:
	explicit WaitForInvitationDialog(ConferenceAnnunciator& annunciator);
	
	inline wxArrayString GetAddressList() { return m_AddressList; }
	
	void OnCancel(wxCommandEvent &event);

protected:
	virtual void OnCallPacket(const char* participantList[], size_t numberOfParticipants);
	
	DECLARE_EVENT_TABLE()

private:
	ConferenceAnnunciator& m_Annunciator;
	wxArrayString m_AddressList;
};

#endif // __WAITFORINVITATIONDIALOG_HPP__