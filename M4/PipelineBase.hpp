///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file PipelineBase.hpp
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
/// @brief This file declares the PipelineBase class, which is a base class for SenderPipeline and
/// ReceiverPipeline. Its functions are common to both.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __PIPELINE_BASE_HPP__
#define __PIPELINE_BASE_HPP__


#include <gst/gst.h>          // for GStreamer stuff
#include <cassert>            // for assert

// The Pipeline tracer is currently commented out to keep the console output quiet. Uncomment to
// re-include it.
//#include "PipelineTracer.hpp" // for PipelineTracer


///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class represents a base pipeline class with common functions.
///////////////////////////////////////////////////////////////////////////////////////////////////
class PipelineBase
{
public:
	/// Constructor
	inline explicit PipelineBase(GstElement* pPipeline)
		: m_pPipeline(pPipeline)
//		, m_Tracer(m_pPipeline)
	{
		assert(m_pPipeline != NULL);
	}
	
	
	/// Destructor
	virtual ~PipelineBase()
	{
		// Set the pipeline to NULL to stop and free everything
		gst_element_set_state(m_pPipeline, GST_STATE_NULL);
	
		// Now unref the pipeline
		gst_object_unref(m_pPipeline);
	}
	
	
	/// Pipeline accessor
	GstElement* Pipeline() const { return m_pPipeline; }
	

	/// Make the pipeline go to NULL
	void Nullify();

	
	/// Make the pipeline play
	void Play();
	
	
	/// Send an end-of-stream event to the pipeline.
	void SendEOS();
	
	
	/// Bus message callback function typedef.
	typedef gboolean (*BusMessageFunction)(GstBus* bus, GstMessage* msg, gpointer data);
	
	
	/// Add a bus message handler
	void AddBusWatch(BusMessageFunction fnBusMessage, void* data = NULL);


protected:


private:
	/// The one and only GStreamer pipeline.
	GstElement* const m_pPipeline;
	
	/// Pipeline tracer
//	PipelineTracer m_Tracer;
}; // END class PipelineBase

#endif // __PIPELINE_BASE_HPP__