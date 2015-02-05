///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file PipelineBase.cpp
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
/// @brief This file defines the functions of the PipelineBase class.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>          // for assert
#include "PipelineBase.hpp" // for class declaration


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineBase::Play()
///
/// Put the pipeline in a playing state.
///////////////////////////////////////////////////////////////////////////////////////////////////
void PipelineBase::Play()
{
	assert(gst_element_set_state(m_pPipeline, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineBase::SendEOS()
///
/// Sends an end-of-stream event to the pipeline.
///////////////////////////////////////////////////////////////////////////////////////////////////
void PipelineBase::SendEOS()
{
	gst_element_send_event(m_pPipeline, gst_event_new_eos());
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// PipelineBase::AddBusWatch()
///
/// Adds a bus message watch handler.
///////////////////////////////////////////////////////////////////////////////////////////////////
void PipelineBase::AddBusWatch(BusMessageFunction fnBusMessage, void* data)
{
	GstBus* pBus = gst_pipeline_get_bus(GST_PIPELINE(m_pPipeline));
	assert(pBus != NULL);
	gst_bus_add_watch(pBus, fnBusMessage, data);
	gst_object_unref(pBus);
}