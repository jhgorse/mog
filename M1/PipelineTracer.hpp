///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file PipelineTracer.hpp
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
/// @brief This file declares the PipelineTracer class, which traces jitter and latency of a
/// GStreamer pipeline.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __PIPELINE_TRACER_HPP__
#define __PIPELINE_TRACER_HPP__

#include <vector> // for std::vector

///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class traces metrics from a GStreamer pipeline. Specifically, it traces:
///  - The jitter of the generation of buffers from media sources (specifically, video and audio
///	sources), and
///  - The intra-element latency of transform elements (i.e. how long it takes for a transform
///	element to do its job and push buffers out the other side).
///
/// These metrics can be used to study a pipeline's performance and to set up expectations for
/// total pipeline latency.
///
/// This class's sole public code interface is the constructor, which receives a pointer to the
/// pipeline to be measured.
///////////////////////////////////////////////////////////////////////////////////////////////////
class PipelineTracer
{
public:
	/// Constructor
	explicit PipelineTracer(GstElement* pPipeline);
	
	
	/// Destructor
	~PipelineTracer();
	
	
protected:


private:
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This function returns whether or not the given element is a sink element.
	///
	/// @param The element to determine whether or not it's a sink.
	///
	/// @return true if the element is a sink, false if it is not.
	///////////////////////////////////////////////////////////////////////////////////////////////
	static inline bool ElementIsSink(const GstElement* e)
	{
		return GST_OBJECT_FLAG_IS_SET(e, GST_ELEMENT_FLAG_SINK);
	}

	
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This function returns whether or not the given element is a source element.
	///
	/// @param The element to determine whether or not it's a source.
	///
	/// @return true if the element is a source, false if it is not.
	///////////////////////////////////////////////////////////////////////////////////////////////
	static inline bool ElementIsSource(const GstElement* e)
	{
		return GST_OBJECT_FLAG_IS_SET(e, GST_ELEMENT_FLAG_SOURCE);
	}
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This function returns whether or not the given element is a transform element.
	///
	/// @param The element to determine whether or not it's a transform.
	///
	/// @return true if the element is a transform, false if it is not.
	///////////////////////////////////////////////////////////////////////////////////////////////
	static inline bool ElementIsTransform(const GstElement* e)
	{
		return (!ElementIsSink(e) && !ElementIsSource(e));
	}
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// Find the corresponding sink pad for the given source pad on an RtpBin element.
	///
	/// This function finds the sink pad from which data appears on a given source pad from an
	/// RtpBin element. This is used in tracking intra-element latency, as an RtpBin has multiple
	/// sink and source pads; we have to find which pairs go together in order to track the
	/// separate paths through the element.
	///
	/// The ref count is not incremented on any pad returned from this function.
	///
	/// @param The source pad for which to look for a corresponding sink pad.
	///
	/// @return The corresponding sink pad, or NULL if a corresponding sink pad could not be found
	/// or doesn't exist (e.g. for RTCP pads).
	///////////////////////////////////////////////////////////////////////////////////////////////
	static GstPad* FindRtpBinSinkBySrc(const GstPad* pSrcPad);


	///////////////////////////////////////////////////////////////////////////////////////////////
	/// Handle the creation of a new pad on an RtpBin element.
	///
	/// This is the static version that redirects to an instance version; see RtpBinNewPad() for
	/// details.
	///////////////////////////////////////////////////////////////////////////////////////////////
	static void StaticRtpBinNewPad(GstElement* element, GstPad* pad, gpointer user_data)
	{
		(reinterpret_cast<PipelineTracer*>(user_data))->RtpBinNewPad(element, pad);
	}
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// Returns the name of a pad's parent element.
	///
	/// This function is used in the member initialization list of constructor(s), since calling
	/// gst_pad_get_parent_element() requires the returned value to be unref'ed.
	///
	/// @param The pad for which to return the parent element's name.
	///
	/// @return The name of the pad's parent element. Must be freed with g_free.
	///////////////////////////////////////////////////////////////////////////////////////////////
	static const gchar* GetPadParentElementName(const GstPad* pPad);
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// Handles the creation of a new pad on an RtpBin element.
	///
	/// This function is called by GStreamer in response to the "pad-added" signal on an RtpBin
	/// element. Since the RtpBin element has sometimes pads, we need to handle the addition of
	/// pads in order to check for new sink->source pad links that need to have their intra-element
	/// latency tracked.
	///
	/// @param The RtpBin element to which a pad has been added.
	///
	/// @param The pad that has been added to the element.
	///////////////////////////////////////////////////////////////////////////////////////////////
	void RtpBinNewPad(GstElement* element, GstPad* pad);
	
	
	/// A pointer to pipeline we are tracing.
	GstElement* const m_pPipeline;
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This class represents a source pad on which we wish to track source jitter and its
	/// associated metadata. 
	///////////////////////////////////////////////////////////////////////////////////////////////
	class SourcePadJitterEntry
	{
	public:
		///////////////////////////////////////////////////////////////////////////////////////////
		/// Handle a data probe on a source pad of a source element.
		///
		/// This is the static version that redirects to an instance version; see SrcProbe() for
		/// details.
		///////////////////////////////////////////////////////////////////////////////////////////
		static GstPadProbeReturn StaticSrcProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
		{
			return (reinterpret_cast<SourcePadJitterEntry*>(user_data))->SrcProbe(pad, info);
		}
		
		
		/// Constructor.
		inline explicit SourcePadJitterEntry(const GstPad* pSourcePad)
			: m_pPad(GST_PAD(gst_object_ref(GST_OBJECT(pSourcePad))))
			, m_ElementName(GetPadParentElementName(pSourcePad))
			, m_PadName(gst_pad_get_name(pSourcePad))
			, m_LastSourceTimestamp(0)
		{}
		
		
		/// Destructor.
		~SourcePadJitterEntry();
		
		
		///////////////////////////////////////////////////////////////////////////////////////////
		/// Called by GStreamer to handle a data probe on the source pad of a source element.
		///
		/// This function traces jitter on the production of data from the source pad of a media
		/// source element (specifically, audio or video).
		///
		/// @param The pad that the data has traversed.
		///
		/// @param The information about the data probe.
		///
		/// @return The return value to give to GStreamer about how to proceed.
		///////////////////////////////////////////////////////////////////////////////////////////
		GstPadProbeReturn SrcProbe(GstPad* pad, GstPadProbeInfo* info);
		
		
		/// Returns the pad being tracked for jitter.
		inline const GstPad* Pad() const { return m_pPad; }
		
		
		/// Returns the name of the element being tracked for jitter.
		inline const gchar* ElementName() const { return m_ElementName; }
		
		
		/// Returns the name of the pad being tracked for jitter.
		inline const gchar* PadName() const { return m_PadName; }
		
		
		/// Returns the timestamp of the last time data left this source pad.
		inline guint64 LastSourceTimestamp() const { return m_LastSourceTimestamp; }
		
		
		/// Sets the timestamp of the last time data left this source pad.
		inline void LastSourceTimestamp(guint64 lastSourceTimestamp) { m_LastSourceTimestamp = lastSourceTimestamp; }
	
	
	private:
	
		/// The pad being tracked.
		const GstPad* const		m_pPad;
		
		
		/// The name of the element being tracked.
		const gchar* const		m_ElementName;
		
		
		/// The name of the pad being tracked.
		const gchar* const		m_PadName;
		
		
		/// The timestamp of the last time data left the source pad.
		guint64					m_LastSourceTimestamp;
	};
	
	
	/// A type definition representing a vector of SourcePadJitterEntry's.
	typedef std::vector<SourcePadJitterEntry*> JEVector;
	
	
	/// Vector of source pads on which to monitor jitter.
	JEVector m_SourcePads;
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	/// This class represents the data needed to track the intra-element latency of transform
	/// elements. It keeps references to the sink and source pads along which data passes, plus
	/// other metadata.
	///////////////////////////////////////////////////////////////////////////////////////////////
	class IntraElementLatencyEntry
	{
	public:
		///////////////////////////////////////////////////////////////////////////////////////////
		/// Handle a data probe on a sink pad of a transform element.
		///
		/// This is the static version that redirects to an instance version; see XfrmSinkProbe()
		/// for details.
		///////////////////////////////////////////////////////////////////////////////////////////
		static GstPadProbeReturn StaticXfrmSinkProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
		{
			return (reinterpret_cast<IntraElementLatencyEntry*>(user_data))->XfrmSinkProbe(pad, info);
		}
		
		
		///////////////////////////////////////////////////////////////////////////////////////////
		/// Handle a data probe on a source pad of a transform element.
		///
		/// This is the static version that redirects to an instance version; see XfrmSrcProbe()
		/// for details.
		///////////////////////////////////////////////////////////////////////////////////////////
		static GstPadProbeReturn StaticXfrmSrcProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
		{
			return (reinterpret_cast<IntraElementLatencyEntry*>(user_data))->XfrmSrcProbe(pad, info);
		}
		
		
		/// Constructor.
		explicit IntraElementLatencyEntry(const GstPad* pSinkPad, const GstPad* pSourcePad);
		
		
		/// Destructor.
		~IntraElementLatencyEntry();
		
		
		/// Returns the sink pad on which latency is tracked.
		inline const GstPad* SinkPad() const { return m_pSinkPad; }
		
		
		/// Returns the source pad on which latency is tracked.
		inline const GstPad* SourcePad() const { return m_pSourcePad; }
		
		
		/// Returns the name of the element being tracked for latency.
		inline const gchar* ElementName() const { return m_ElementName; }
		
		
		/// Returns the name of the sink pad being tracked for latency.
		inline const gchar* SinkPadName() const { return m_SinkPadName; }
		
		
		/// Returns the name of the source pad being tracked for latency.
		inline const gchar* SourcePadName() const { return m_SourcePadName; }
		
		
		/// Returns the timestamp of the last time data arrived on the sink pad.
		inline guint64 LastSinkTimestamp() const { return m_LastSinkTimestamp; }
		
		
		/// Sets the timestamp of the last time data arrived on the sink pad.
		inline void LastSinkTimestamp(guint64 lastSinkTimestamp) { m_LastSinkTimestamp = lastSinkTimestamp; }
		
		
		///////////////////////////////////////////////////////////////////////////////////////////
		/// Called by GStreamer to handle a data probe on the sink pad of a transform element.
		///
		/// This function is used to trace intra-element latency of transform elements.
		///
		/// @param The pad that the data has traversed.
		///
		/// @param The information about the data probe.
		///
		/// @return The return value to give to GStreamer about how to proceed.
		///////////////////////////////////////////////////////////////////////////////////////////
		GstPadProbeReturn XfrmSinkProbe(GstPad* pad, GstPadProbeInfo* info);
	
	
		///////////////////////////////////////////////////////////////////////////////////////////
		/// Called by GStreamer to handle a data probe on the source pad of a transform element.
		///
		/// This function is used to trace intra-element latency of transform elements.
		///
		/// @param The pad that the data has traversed.
		///
		/// @param The information about the data probe.
		///
		/// @return The return value to give to GStreamer about how to proceed.
		///////////////////////////////////////////////////////////////////////////////////////////
		GstPadProbeReturn XfrmSrcProbe(GstPad* pad, GstPadProbeInfo* info);
	
	
	private:
	
		/// The sink pad being tracked.
		const GstPad* const		m_pSinkPad;
		
		
		/// The source pad being tracked.
		const GstPad* const		m_pSourcePad;
		
		
		/// The name of the element being tracked.
		const gchar* const		m_ElementName;
		
		
		/// The name of the sink pad being tracked.
		const gchar* const		m_SinkPadName;
		
		
		/// The name of the source pad being tracked.
		const gchar* const		m_SourcePadName;
		
		
		/// The timestamp of when data last arrived on the sink pad.
		guint64					m_LastSinkTimestamp;
	};
	
	
	/// A type definition representing a vector of IntraElementLatencyEntry's.
	typedef std::vector<IntraElementLatencyEntry*> IELVector;
	
	
	/// Vector of entries on which to monitor intra-element latency.
	IELVector m_IELVector;
};

#endif // #define __PIPELINE_TRACER_HPP__
