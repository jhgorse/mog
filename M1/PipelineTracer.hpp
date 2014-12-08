#include <vector>

class PipelineTracer
{
public:
	// Constructor
	explicit PipelineTracer(GstElement* pPipeline);
	
	// Destructor
	~PipelineTracer();
	
protected:

private:
	// Whether or not this element is a sink element.
	static inline bool ElementIsSink(const GstElement* e)
	{
		return GST_OBJECT_FLAG_IS_SET(e, GST_ELEMENT_FLAG_SINK);
	}
	
	// Whether or not this element is a source element.
	static inline bool ElementIsSource(const GstElement* e)
	{
		return GST_OBJECT_FLAG_IS_SET(e, GST_ELEMENT_FLAG_SOURCE);
	}
	
	// Whether or not this element is a transform element.
	static inline bool ElementIsTransform(const GstElement* e)
	{
		return (!ElementIsSink(e) && !ElementIsSource(e));
	}
	
	// Find an RtpBin's sink pad (for the purposes of tracking intra-element latency) by
	// the requested src pad. Returns NULL if none exists or it doesn't make sense (e.g.
	// the RTCP src pads). The pad is not referenced.
	static GstPad* FindRtpBinSinkBySrc(const GstPad* pSrcPad);

	// The static probe functions that redirect to the instance ones.
	static void StaticRtpBinNewPad(GstElement* element, GstPad* pad, gpointer user_data);
	static GstPadProbeReturn StaticSrcProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data);
	static GstPadProbeReturn StaticXfrmSinkProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data);
	static GstPadProbeReturn StaticXfrmSrcProbe(GstPad* pad, GstPadProbeInfo* info, gpointer user_data);
	
	// Utility function to get pad parent element name, properly unref'ing things.
	static const gchar* GetPadParentElementName(const GstPad* pPad);
	
	// Called when a new pad is added to an RtpBin element
	void RtpBinNewPad(GstElement* element, GstPad* pad);
	
	// The instance probe function for source elements/pads
	GstPadProbeReturn SrcProbe(GstPad* pad, GstPadProbeInfo* info);
	
	// The instance probe functions for transform elements/pads
	GstPadProbeReturn XfrmSinkProbe(GstPad* pad, GstPadProbeInfo* info);
	GstPadProbeReturn XfrmSrcProbe(GstPad* pad, GstPadProbeInfo* info);
	
	// Pointer to pipeline we are tracing
	GstElement* const m_pPipeline;
	
	// A structure for tracking source pad jitter
	struct SourcePadJitterEntry
	{
	public:
		// Constructor
		explicit SourcePadJitterEntry(const GstPad* pSourcePad);
		
		// Destructor
		~SourcePadJitterEntry();
		
		// SourcePad accessor
		inline const GstPad* Pad() const { return m_pPad; }
		
		// ElementName accessor
		inline const gchar* ElementName() const { return m_ElementName; }
		
		// PadName accessor
		inline const gchar* PadName() const { return m_PadName; }
		
		// LastSourceTimestamp accessors
		inline uint64_t LastSourceTimestamp() const { return m_LastSourceTimestamp; }
		inline void LastSourceTimestamp(uint64_t lastSourceTimestamp) { m_LastSourceTimestamp = lastSourceTimestamp; }
	
	private:
		// Member data
		const GstPad* const		m_pPad;
		const gchar* const		m_ElementName;
		const gchar* const		m_PadName;
		uint64_t				m_LastSourceTimestamp;
	};
	
	// Vector of source pads on which to monitor jitter
	typedef std::vector<SourcePadJitterEntry*> JEVector;
	JEVector m_SourcePads;
	
	// A structure for tracking the latency of a single element from a sink to a source
	struct IntraElementLatencyEntry
	{
	public:
		// Constructor
		explicit IntraElementLatencyEntry(const GstPad* pSinkPad, const GstPad* pSourcePad);
		
		// Destructor
		~IntraElementLatencyEntry();
		
		// SinkPad accessor
		inline const GstPad* SinkPad() const { return m_pSinkPad; }
		
		// SourcePad accessor
		inline const GstPad* SourcePad() const { return m_pSourcePad; }
		
		// ElementName accessor
		inline const gchar* ElementName() const { return m_ElementName; }
		
		// SinkPadName accessor
		inline const gchar* SinkPadName() const { return m_SinkPadName; }
		
		// SourcePadName accessor
		inline const gchar* SourcePadName() const { return m_SourcePadName; }
		
		// LastSourceTimestamp accessors
		inline uint64_t LastSinkTimestamp() const { return m_LastSinkTimestamp; }
		inline void LastSinkTimestamp(uint64_t lastSinkTimestamp) { m_LastSinkTimestamp = lastSinkTimestamp; }
	
	private:
		// Member data
		const GstPad* const		m_pSinkPad;
		const GstPad* const		m_pSourcePad;
		const gchar* const		m_ElementName;
		const gchar* const		m_SinkPadName;
		const gchar* const		m_SourcePadName;
		uint64_t				m_LastSinkTimestamp;
	};
	
	// Vector of elements on which to monitor intra-element latency
	typedef std::vector<IntraElementLatencyEntry*> IELVector;
	IELVector m_IELVector;
};