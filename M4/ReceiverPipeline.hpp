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


#include <set>                // for std::set
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
	/// An interface for notifying about sender sessions
	class IReceiverNotifySink
	{
	public:
		enum SsrcType
		{
			SSRC_TYPE_VIDEO,
			SSRC_TYPE_AUDIO
		};
		enum SsrcDeactivateReason
		{
			SSRC_DEACTIVATE_REASON_BYE,
			SSRC_DEACTIVATE_REASON_STOP,
			SSRC_DEACTIVATE_REASON_TIMEOUT,
		};
		
		virtual ~IReceiverNotifySink() {}
		virtual void OnSsrcActivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc) = 0;
		virtual void OnSsrcDeactivate(ReceiverPipeline& rPipeline, SsrcType type, unsigned int ssrc, SsrcDeactivateReason reason) = 0;
	};
	
	
	/// Constructor
	explicit ReceiverPipeline(IReceiverNotifySink* pNotifySink = NULL);
	
	
	/// Destructor
	virtual ~ReceiverPipeline();
	
	
	void ActivateVideoSsrc(unsigned int ssrc, const char* pictureParameters, void* windowHandle);
	void ActivateAudioSsrc(unsigned int ssrc);
	void DeactivateVideoSsrc(unsigned int ssrc);
	void DeactivateAudioSsrc(unsigned int ssrc);
		

protected:


private:
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This string is the pipeline created for this milestone. There are a lot of "hard-coded"
	/// things here that should be made more fluid and configurable in future milestones.
	///////////////////////////////////////////////////////////////////////////////////////////////
	static const char PIPELINE_STRING[];
	

	/// (Static) callback for when an SSRC leaves (says bye)
	static void StaticOnRtpBinByeSsrc(GstElement* element, guint session, guint ssrc, gpointer data);
	
	
	/// (Static) callback for when an SSRC leaves (bye times out)
	static void StaticOnRtpBinByeTimeout(GstElement* element, guint session, guint ssrc, gpointer data);
	
	
	/// (Static) callback for when an SSRC leaves due to its stop time
	static void StaticOnRtpBinNptStop(GstElement* element, guint session, guint ssrc, gpointer data);
	
	
	/// (Static) callback for when pads are added to rtpbin
	static void StaticOnRtpBinPadAdded(GstElement* element, GstPad* pad, gpointer data);
	

	/// (Static) callback for when an SSRC times out
	static void StaticOnRtpBinSenderTimeout(GstElement* element, guint session, guint ssrc, gpointer data);
	
	
	/// (Static) callback for when an SSRC becomes active
	static void StaticOnRtpBinSsrcActive(GstElement* element, guint session, guint ssrc, gpointer data);
	
	
	/// (Static) callback for when an SSRC times out
	static void StaticOnRtpBinTimeout(GstElement* element, guint session, guint ssrc, gpointer data);
	
	
	/// (Instance) callback for when pads are added to rtpbin
	void OnRtpBinPadAdded(GstElement* element, GstPad* pad);
	
	
	/// (Instance) callback for when an SSRC becomes active
	void OnRtpBinSsrcActivate(IReceiverNotifySink::SsrcType type, unsigned int ssrc);
	
	
	/// (Instance) callback for when an SSRC becomes inactive
	void OnRtpBinSsrcDeactivate(IReceiverNotifySink::SsrcType type, unsigned int ssrc, IReceiverNotifySink::SsrcDeactivateReason reason);
	
	
	/// Notify pointer
	IReceiverNotifySink* const m_pNotifySink;
	
	
	/// Set of active SSRCs
	std::set<unsigned int> m_ActiveSsrcs;
	
	
	/// Reference to rtpbin element
	GstElement* const m_pRtpBin;
}; // END class ReceiverPipeline

#endif // __RECEIVER_PIPELINE_HPP__