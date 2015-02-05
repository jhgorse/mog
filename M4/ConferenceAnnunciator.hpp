///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file ConferenceAnnunciator.hpp
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
/// @brief This file declares the ConferenceAnnunciator class, which is responsible for
/// communicating information about a conference between participants.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __CONFERENCEANNUNCIATOR_HPP__
#define __CONFERENCEANNUNCIATOR_HPP__

#include <netinet/in.h>
#include <pthread.h>

#include <cstddef>

class ConferenceAnnunciator
{
public:
	class ICallPacketListener
	{
	public:
		virtual ~ICallPacketListener() {}
		virtual void OnCallPacket(const char* participantList[], size_t numberOfParticipants) = 0;
	};
	
	class IParameterPacketListener
	{
	public:
		virtual ~IParameterPacketListener() {}
		virtual void OnParameterPacket(const char* address, const char* pictureParameters, unsigned int videoSsrc, unsigned int audioSsrc) = 0;
	};
	
	/// Constructor.
	ConferenceAnnunciator();
	
	
	/// Destructor.
	~ConferenceAnnunciator();
	
	// Call packet listener setters
	inline void SetCallPacketListener(ICallPacketListener* listener) { m_pCallPacketListener = listener; }
	inline void ClearCallPacketListener() { m_pCallPacketListener = NULL; }
	
	// Parameter packet listener setters
	inline void SetParameterPacketListener(IParameterPacketListener* listener) { m_pParameterPacketListener = listener; }
	inline void ClearParameterPacketListener() { m_pParameterPacketListener = NULL; }
	
	
	/// Configure the annunciator to send parameters to other participants
	void SendParameters(const char* pPictureParameters, unsigned int videoSsrc, unsigned int audioSsrc);


	/// Configure the annunciator to send the participant list periodically.
	void SendParticipantList(const char* participantAddresses[], size_t numberOfParticipants);
	
	
	/// Configure the annunciator with the participant list to use for sent packets
	void SetParticipantList(const char* participantAddresses[], size_t numberOfParticipants);
	
protected:
	
private:
	/// The port we use for these communications
	static const unsigned short UDP_PORT = 9999;
	
	
	/// How long to wait between transmitting informational packets
	static const long TRANSMIT_INTERVAL_US = 2000000;
	
	
	/// (Static) function to create the UDP socket (for ctor usage)
	static int CreateSocket();
	
	
	/// The (static) worker function.
	static void* StaticWorkerFn(void* pArg);
	
	
	/// This function is called when a "call" packet is received (with the participant list)
	void HandleCallPacket(const char* packet, size_t packetSize);
	
	
	/// This function is called when a "parm" packet is received with sender parameters
	void HandleParameterPacket(const char* packet, size_t packetSize, struct sockaddr_in* pSenderAddress);
	
	
	/// This function sends the participant list
	void SendParticipantList();
	
	
	/// This function sends the parameters
	void SendParameters();
	
	
	/// The (instance) worker function
	void* WorkerFn();
	
	
	/// The UDP socket
	int m_UdpSocket;
	
	/// The worker thread
	pthread_t m_WorkerThread;
	
	// TODO: Could probably use a mutex around all this mutable stuff.

	/// The ICallPacketListener
	ICallPacketListener* m_pCallPacketListener;
	
	/// The IParameterPacketListener
	IParameterPacketListener* m_pParameterPacketListener;
	
	/// The next time we should send packets
	struct timespec m_NextXmitTime;
	
	/// The participant list packet and size (if specified)
	const char* m_pParticipantListPacket;
	size_t m_ParticipantListPacketLength;
	
	/// The parameter packet and size (if specified)
	const char* m_pParameterPacket;
	size_t m_ParameterPacketLength;
	
	/// The destination addresses and number of addresses (if specified)
	const struct sockaddr_in* m_pDestinationAddresses;
	size_t m_NumberOfDestinations;
};

#endif // __CONFERENCEANNUNCIATOR_HPP__