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
	explicit ReceiverPipeline(const char* address);
	
	
	/// Destructor
	virtual ~ReceiverPipeline();
	
	
	/// (Instance) callback for when pads are added to rtpbin
	void OnRtpBinPadAdded(GstElement* element, GstPad* pad);
	

protected:


private:
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This string is the pipeline created for this milestone. There are a lot of "hard-coded"
	/// things here that should be made more fluid and configurable in future milestones.
	///////////////////////////////////////////////////////////////////////////////////////////////
	static const char PIPELINE_STRING[];
	
	
	/// (Static) callback for when pads are added to rtpbin
	static void StaticOnRtpBinPadAdded(GstElement* element, GstPad* pad, gpointer data);
	
	
	/// Get a sink pad by name on an element. Properly unrefs intermediate stuff (for use in ctor).
	static GstPad* GetElementSinkPad(const GstBin* bin, const char* element_name, const char* pad_name);
	
	
	/// Parse the launch string, interpreted with address, into a pipeline.
	static GstElement* ParsePipeline(const char* address);
	
	
	/// Reference to rtpbin element
	GstElement* const m_pRtpBin;
	
	
	/// Reference to sink pad of video depayloader
	GstPad* const m_pVideoDepayloaderSinkPad;
	
	
	/// Reference to sink pad of audio depayloader
	GstPad* const m_pAudioDepayloaderSinkPad;
}; // END class ReceiverPipeline

#endif // __RECEIVER_PIPELINE_HPP__