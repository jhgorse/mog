///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file ReceiverPipeline.hpp
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
/// @brief This file declares the ReceiverPipeline class, which represents a pipeline receiving
/// video and audio data from a remote endpoint.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __RECEIVER_PIPELINE_HPP__
#define __RECEIVER_PIPELINE_HPP__


#include <gst/gst.h>          // for GStreamer stuff
#include "PipelineBase.hpp"   // for PipelineBase
#include "PipelineTracer.hpp" // for PipelineTracer


///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class represents the "receiver" side of the conference; that is, the received, decoded,
/// and displayed video and audio data.
///////////////////////////////////////////////////////////////////////////////////////////////////
class ReceiverPipeline : public PipelineBase
{
public:	
	/// Constructor
	ReceiverPipeline(uint16_t basePort, const char* audioDeviceName, const char* pictureParameters, void* pWindowHandle);
	
	
	/// Destructor
	virtual ~ReceiverPipeline();
	
	
	/// Get the display window sink using a native window handle.
	const void* GetWindowSink() const { return m_DisplayWindowHandle; }
		

protected:


private:
	/// The "static" parts of the pipeline, as a string.
	static const char PIPELINE_STRING[];


	/// Reference to rtpbin element
	GstElement* const m_pRtpBin;
	
	
	/// The display window handle
	const void* const m_DisplayWindowHandle;
}; // END class ReceiverPipeline

#endif // __RECEIVER_PIPELINE_HPP__