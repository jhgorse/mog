///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file ConferenceAnnunciator.cpp
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
/// @brief This file defines the functions of the ConferenceAnnunciator class, which is responsible
/// for communicating information about a conference between participants.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstring>

#include "clocks.h"
#include "ConferenceAnnunciator.hpp"

ConferenceAnnunciator::ConferenceAnnunciator()
	: m_UdpSocket(CreateSocket())
	, m_WorkerThread()
	, m_pCallPacketListener(NULL)
	, m_pParameterPacketListener(NULL)
	, m_NextXmitTime()
	, m_pParticipantListPacket(NULL)
	, m_ParticipantListPacketLength(0)
	, m_pParameterPacket(NULL)
	, m_ParameterPacketLength(0)
	, m_pDestinationAddresses(NULL)
	, m_NumberOfDestinations(0)
{
	// Set "now" as the next xmit time so that, in theory, we would send right away.
	assert(clock_gettime(CLOCK_MONOTONIC, &m_NextXmitTime) == 0);
	
	// Spawn the worker thread
	assert(pthread_create(&m_WorkerThread, NULL, StaticWorkerFn, this) == 0);
}
	
	
ConferenceAnnunciator::~ConferenceAnnunciator()
{
	pthread_cancel(m_WorkerThread);
	
	if (m_pParticipantListPacket != NULL)
	{
		const char* pParticipantListPacketSave = m_pParticipantListPacket;
		m_pParticipantListPacket = NULL;
		delete[] pParticipantListPacketSave;
	}
	
	if (m_pParameterPacket != NULL)
	{
		const char* pParameterPacketSave = m_pParameterPacket;
		m_pParameterPacket = NULL;
		delete[] pParameterPacketSave;
	}
	
	if (m_pDestinationAddresses != NULL)
	{
		const struct sockaddr_in* pDestinationAddressesSave = m_pDestinationAddresses;
		m_pDestinationAddresses = NULL;
		delete[] pDestinationAddressesSave;
	}
	
	close(m_UdpSocket);
}


void ConferenceAnnunciator::SendParameters(const char* pPictureParameters, unsigned int videoSsrc, unsigned int audioSsrc)
{
	// If we already have a parameter packet, free it.
	if (m_pParameterPacket != NULL)
	{
		const char* pParameterPacketSave = m_pParameterPacket;
		m_pParameterPacket = NULL;
		delete[] pParameterPacketSave;
	}
	
	// Allocate and format a new parameter packet.
	size_t pictureParametersLength = std::strlen(pPictureParameters);
	m_ParameterPacketLength = 4 + pictureParametersLength + 1 + sizeof(videoSsrc) + sizeof(audioSsrc);
	char* pParameterPacket = new char[m_ParameterPacketLength];
	std::memcpy(pParameterPacket, "PARM", 4);
	char* pWorking = &pParameterPacket[4];
	std::strcpy(pWorking, pPictureParameters);
	pWorking[pictureParametersLength] = '\0';
	pWorking += pictureParametersLength + 1;
	*((unsigned int *)pWorking) = videoSsrc;
	pWorking += sizeof(videoSsrc);
	*((unsigned int *)pWorking) = audioSsrc;
	m_pParameterPacket = pParameterPacket;
	
	std::printf("Parameter packet of %u bytes allocated.\n", m_ParameterPacketLength);
}


void ConferenceAnnunciator::SendParticipantList(const char* participantAddresses[], size_t numberOfParticipants)
{
	SetParticipantList(participantAddresses, numberOfParticipants);
	
	// If we already have a participant list packet, free it.
	if (m_pParticipantListPacket != NULL)
	{
		const char* pParticipantListPacketSave = m_pParticipantListPacket;
		m_pParticipantListPacket = NULL;
		delete[] pParticipantListPacketSave;
	}
	
	// Allocate and format a participant list packet
	m_ParticipantListPacketLength = 4; // "CALL"
	for (size_t i = 0; i < numberOfParticipants; ++i)
	{
		m_ParticipantListPacketLength += std::strlen(participantAddresses[i]) + 1;
	}
	char* pParticipantListPacket = new char[m_ParticipantListPacketLength];
	std::memcpy(pParticipantListPacket, "CALL", 4);
	char* pWorking = &pParticipantListPacket[4];
	for (size_t i = 0; i < numberOfParticipants; ++i)
	{
		size_t len = std::strlen(participantAddresses[i]);
		std::strcpy(pWorking, participantAddresses[i]);
		pWorking[len] = '\0';
		pWorking += len + 1;
	}
	m_pParticipantListPacket = pParticipantListPacket;
	
	std::printf("Participant list packet of %u bytes allocated.\n", m_ParticipantListPacketLength);
}


void ConferenceAnnunciator::SetParticipantList(const char* participantAddresses[], size_t numberOfParticipants)
{
	// If we already have a destination list, free it.
	if (m_pDestinationAddresses != NULL)
	{
		const struct sockaddr_in* pDestinationAddressesSave = m_pDestinationAddresses;
		m_pDestinationAddresses = NULL;
		delete[] pDestinationAddressesSave;
	}
	
	// Convert the participant addresses to sockaddr structs and save for posterity.
	struct sockaddr_in* destinationAddresses = new struct sockaddr_in[numberOfParticipants];
	for (size_t i = 0; i < numberOfParticipants; ++i)
	{
		struct sockaddr_in* pAddr = reinterpret_cast<struct sockaddr_in*>(&destinationAddresses[i]);
		pAddr->sin_family = AF_INET;
		pAddr->sin_port = htons(UDP_PORT);
		assert(inet_pton(AF_INET, participantAddresses[i], &pAddr->sin_addr) == 1);
	}
	m_NumberOfDestinations = numberOfParticipants;
	m_pDestinationAddresses = destinationAddresses;
}


int ConferenceAnnunciator::CreateSocket()
{
	// Create the socket itself
	int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(s >= 0);
	
	// Bind it to our port
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(UDP_PORT);
	assert(bind(s, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0);
	
	// Set to non-blocking
	int flags = fcntl(s, F_GETFL, 0);
	assert(flags >= 0);
	assert(fcntl(s, F_SETFL, flags | O_NONBLOCK) == 0);
	
	// Return the socket
	return s;
}


void* ConferenceAnnunciator::StaticWorkerFn(void* pArg)
{
	return reinterpret_cast<ConferenceAnnunciator*>(pArg)->WorkerFn();
}


void ConferenceAnnunciator::HandleCallPacket(const char* packet, size_t packetSize)
{
	// Early return if no one's listening
	if (m_pCallPacketListener == NULL)
	{
		return;
	}
	
	// Count the number of participants listed in the packet
	size_t numParticipants = 0;
	size_t index = 0;
	while (index < packetSize)
	{
		numParticipants++;
		index += std::strlen(&packet[index]) + 1;
	}
	
	// Allocate pointer storage and copy pointers from packet
	const char** participantList = new const char*[numParticipants];
	index = 0;
	for (size_t participantIndex = 0; participantIndex < numParticipants; ++participantIndex)
	{
		participantList[participantIndex] = &packet[index];
		index += std::strlen(&packet[index]) + 1;
	}
	
	// Call listener
	if (m_pCallPacketListener != NULL)
	{
		m_pCallPacketListener->OnCallPacket(participantList, numParticipants);
	}
	
	// Delete storage
	delete[] participantList;
}


void ConferenceAnnunciator::HandleParameterPacket(const char* packet, size_t packetSize, struct sockaddr_in* pSenderAddress)
{
	// Early return if no one's listening
	if (m_pParameterPacketListener == NULL)
	{
		return;
	}
	
	// Take parameters from packet
	const char* pictureParameters = packet;
	packet += std::strlen(pictureParameters) + 1;
	unsigned int videoSsrc = ntohl(*((unsigned int *)packet));
	packet += sizeof(videoSsrc);
	unsigned int audioSsrc = ntohl(*((unsigned int *)packet));
	
	// Get a string for sender address
	char ipAddress[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(reinterpret_cast<struct sockaddr_in*>(pSenderAddress)->sin_addr), ipAddress, INET_ADDRSTRLEN);
	
	// Call listener
	if (m_pParameterPacketListener != NULL)
	{
		m_pParameterPacketListener->OnParameterPacket(ipAddress, pictureParameters, videoSsrc, audioSsrc);
	}
}


void ConferenceAnnunciator::SendParticipantList()
{
	// Early return in case we don't have a destination list or a participant list packet
	if ((m_pDestinationAddresses == NULL) || (m_pParticipantListPacket == NULL))
	{
		return;
	}
	
	std::printf("Sending participant list packet to %u destinations.\n", m_NumberOfDestinations);
	for (size_t i = 0; i < m_NumberOfDestinations; ++i)
	{
		// Purposefully ignoring return value here, in case we would get back an EAGAIN or EWOULDBLOCK
		// (since the socket is non-blocking). It's ok, we'll try again soon enough.
		sendto(m_UdpSocket, m_pParticipantListPacket, m_ParticipantListPacketLength, 0, reinterpret_cast<const struct sockaddr *>(&m_pDestinationAddresses[i]), sizeof(m_pDestinationAddresses[i]));
	}
}


void ConferenceAnnunciator::SendParameters()
{
	// Early return in case we don't have a destination list or a parameter packet
	if ((m_pDestinationAddresses == NULL) || (m_pParameterPacket == NULL))
	{
		return;
	}
	
	std::printf("Sending parameter packet to %u destinations.\n", m_NumberOfDestinations);
	for (size_t i = 0; i < m_NumberOfDestinations; ++i)
	{
		// Purposefully ignoring return value here, in case we would get back an EAGAIN or EWOULDBLOCK
		// (since the socket is non-blocking). It's ok, we'll try again soon enough.
		sendto(m_UdpSocket, m_pParameterPacket, m_ParameterPacketLength, 0, reinterpret_cast<const struct sockaddr *>(&m_pDestinationAddresses[i]), sizeof(m_pDestinationAddresses[i]));
	}
}


/// Returns the difference in struct timespecs IN MICROSECONDS.
static inline unsigned long operator-(const struct timespec& lhs, const struct timespec& rhs)
{
	static const int64_t US_PER_S = static_cast<int64_t>(1000000);
	static const int64_t NS_PER_US = static_cast<int64_t>(1000);
	
	int64_t delta = (static_cast<int64_t>(lhs.tv_sec) * US_PER_S)
		- (static_cast<int64_t>(rhs.tv_sec) * US_PER_S);
	delta += (static_cast<int64_t>(lhs.tv_nsec) / NS_PER_US)
		- (static_cast<int64_t>(rhs.tv_nsec) / NS_PER_US);
	return static_cast<unsigned long>(delta);
}


/// Adds a specified number of MICROSECONDS to a struct timespec.
static inline struct timespec& operator+=(struct timespec& lhs, long addedUs)
{
	static const int64_t US_PER_S = static_cast<int64_t>(1000000);
	static const int64_t NS_PER_S = static_cast<int64_t>(1000000000);
	static const int64_t NS_PER_US = static_cast<int64_t>(1000);
	
	// Add seconds
	lhs.tv_sec += addedUs / US_PER_S;
	addedUs %= US_PER_S;
	
	// Add microseconds (carefully)
	int64_t tv_nsec = static_cast<int64_t>(lhs.tv_nsec);
	tv_nsec += static_cast<int64_t>(addedUs) * NS_PER_US;
	if (tv_nsec > NS_PER_S)
	{
		lhs.tv_sec += tv_nsec / NS_PER_S;
		tv_nsec %= NS_PER_S;
	}
	lhs.tv_nsec = static_cast<long>(tv_nsec);
	
	return lhs;
}


/// The (instance) worker function
void* ConferenceAnnunciator::WorkerFn()
{
	struct timespec now;
	struct timeval timeout;
	fd_set set;
	
	// This task that will periodically wake up to do things like:
	//  - send the participant list (if configured to do so),
	//  - send the sender picture parameters and SSRCs,
	//  - receive the participant list (if configured to do so),
	//  - receive the other participants' picture parameters and SSRCs
	while (true)
	{
		// Figure out how long to wait 
		assert(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
		if (now >= m_NextXmitTime)
		{
			// We're beyond our next transmit time, so don't sleep at all
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;
		}
		else
		{
			unsigned long delayUs = m_NextXmitTime - now;
			timeout.tv_sec = delayUs / 1000000;
			timeout.tv_usec = delayUs % 1000000;
		}
		
		// Use select to wait for the UDP socket to be readable
		FD_ZERO(&set);
		FD_SET(m_UdpSocket, &set);
//		std::printf("select(%d, %d)\n", timeout.tv_sec, timeout.tv_usec);
		int r = select(m_UdpSocket + 1, &set, NULL, NULL, &timeout);
		assert(r >= 0);
		
		if (r > 0)
		{
			char buffer[1500];
			struct sockaddr_in addr;
			socklen_t addrSize = sizeof(addr);
			
			while (true)
			{
				// Read and process packets from the socket until we can't anymore
				ssize_t read = recvfrom(m_UdpSocket, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&addr), &addrSize);
				if ((read == 0) || ((read < 0) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))))
				{
					// Nothing to read
					break;
				}
				assert(read > 0);
				if (std::strncmp(buffer, "CALL", 4) == 0)
				{
					HandleCallPacket(&buffer[4], read - 4);
				}
				else if (std::strncmp(buffer, "PARM", 4) == 0)
				{
					HandleParameterPacket(&buffer[4], read - 4, &addr);
				}
			}
		}
		
		if (m_pDestinationAddresses != NULL)
		{
			// See if it's time to transmit.
			assert(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
			if (now >= m_NextXmitTime)
			{
				// It is time to transmit.
				SendParticipantList();
				SendParameters();
			}
		}
		
		assert(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
		while (m_NextXmitTime <= now)
		{
			m_NextXmitTime += TRANSMIT_INTERVAL_US;
		}
	}
	
	// Shouldn't get here, but keep the compiler happy...
	return NULL;
}