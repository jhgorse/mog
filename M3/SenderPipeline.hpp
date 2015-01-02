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
#include "PipelineBase.hpp"   // for PipelineBase
#include "PipelineTracer.hpp" // for PipelineTracer


///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class represents the "sender" side of the conference; that is, the captured, encoded, and
/// transmitted video and audio data.
///////////////////////////////////////////////////////////////////////////////////////////////////
class SenderPipeline : public PipelineBase
{
public:
	/// Constructor
	SenderPipeline();
	
	
	/// Destructor
	virtual ~SenderPipeline();
	
	
	/// Set the destination IP address or hostname
	void SetDestination(const char* destination);
	

protected:


private:
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This string is the pipeline created for this milestone. There are a lot of "hard-coded"
	/// things here that should be made more fluid and configurable in future milestones.
	///////////////////////////////////////////////////////////////////////////////////////////////
	static const char PIPELINE_STRING[];
	
	/// Pointer to video RTP sink element
	GstElement* const m_pVideoRtpSink;
	
	/// Pointer to video RTCP sink element
	GstElement* const m_pVideoRtcpSink;
	
	/// Pointer to audio RTP sink element
	GstElement* const m_pAudioRtpSink;
	
	/// Pointer to audio RTCP sink element
	GstElement* const m_pAudioRtcpSink;
	
}; // END class SenderPipeline

#endif // __SENDER_PIPELINE_HPP__