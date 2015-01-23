///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file SenderPipeline.hpp
///
/// Copyright (c) 2014, BoxCast, Inc. All rights reserved.
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
/// @brief This file declares the SenderPipeline class, which represents a pipeline transmitting
/// video and audio data to remote endpoints.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __SENDER_PIPELINE_HPP__
#define __SENDER_PIPELINE_HPP__


#include <gst/gst.h>          // for GStreamer stuff
#include <string>             // for vector
#include <vector>             // for vector

#include "PipelineBase.hpp"   // for PipelineBase
#include "PipelineTracer.hpp" // for PipelineTracer


///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class represents the "sender" side of the conference; that is, the captured, encoded, and
/// transmitted video and audio data.
///////////////////////////////////////////////////////////////////////////////////////////////////
class SenderPipeline : public PipelineBase
{
public:
	/// An interface for notifying about new picture parameters
	class IPictureParameterNotifySink
	{
	public:
		virtual ~IPictureParameterNotifySink() {}
		virtual void OnNewPictureParameters(const SenderPipeline& rPipeline, const char* pPictureParameters) = 0;
	};
	
	
	/// Constructor
	explicit SenderPipeline(IPictureParameterNotifySink* pNotifySink = NULL);
	
	
	/// Destructor
	virtual ~SenderPipeline();
	
	
	/// Add the destination IP address or hostname
	void AddDestination(const char* destination);
	
	
	/// Remove the destination IP address or hostname
	void RemoveDestination(const char* destination);
	
	
	/// Set the video bitrate
	void SetBitrate(size_t bitrate);
	
	
	/// Get picture parameters
	inline const char* GetPictureParameters() const { return m_pSpropParameterSets; }
	

protected:


private:
	/// (Static) callback for pad caps property change
	static void StaticPadNotifyCaps(GObject* gobject, GParamSpec* pspec, gpointer user_data)
	{
		reinterpret_cast<SenderPipeline*>(user_data)->PadNotifyCaps(gobject, pspec);
	}
	
	
	/// (Instance) callback for pad caps property change
	void PadNotifyCaps(GObject* gobject, GParamSpec* pspec);
	
	
	/// Internally set the destinations for the pipeline
	void SetDestinations();
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This string is the pipeline created for this milestone. There are a lot of "hard-coded"
	/// things here that should be made more fluid and configurable in future milestones.
	///////////////////////////////////////////////////////////////////////////////////////////////
	static const char PIPELINE_STRING[];

	/// Pointer to video encoder element
	GstElement* const m_pVideoEncoder;
	
	/// Pointer to video RTP sink element
	GstElement* const m_pVideoRtpSink;
	
	/// Pointer to video RTCP sink element
	GstElement* const m_pVideoRtcpSink;
	
	/// Pointer to audio RTP sink element
	GstElement* const m_pAudioRtpSink;
	
	/// Pointer to audio RTCP sink element
	GstElement* const m_pAudioRtcpSink;
	
	/// Vector of destination addresses
	std::vector<const std::string*> m_vDestinations;
	
	/// Vector of notify interfaces
	IPictureParameterNotifySink* const m_pNotifySink;
	
	/// The video sprop parameter sets string
	const char* m_pSpropParameterSets;
	
}; // END class SenderPipeline

#endif // __SENDER_PIPELINE_HPP__