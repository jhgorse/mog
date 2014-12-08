#!/bin/sh

export GST_DEBUG=2 #,*QOS*:9
export GST_LAUNCH=/Library/Frameworks/GStreamer.framework/Versions/1.0/bin/gst-launch-1.0

${GST_LAUNCH} -v -e \
  rtpbin name=rtpbin latency=10 \
  udpsrc port=10000 \
! "application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,sprop-parameter-sets=\"Z3oAHry0BQHtgIgAKLCoCYloBHixdQ\\=\\=\\,aM48gA\\=\\=\",payload=96" \
! rtpbin.recv_rtp_sink_0 \
  udpsrc port=10001 \
! "application/x-rtcp" \
! rtpbin.recv_rtcp_sink_0 \
  udpsrc port=10002 \
! "application/x-rtp,media=audio,clock-rate=44100,encoding-name=L16,encoding-params=1,channels=1,payload=96" \
! rtpbin.recv_rtp_sink_1 \
  udpsrc port=10003 \
! "application/x-rtcp" \
! rtpbin.recv_rtcp_sink_1 \
  rtpbin. \
! rtph264depay \
! "video/x-h264,stream-format=avc,alignment=au" \
! avdec_h264 \
! autovideoconvert \
! osxvideosink enable-last-sample=false \
  rtpbin. \
! rtpL16depay \
! audioconvert \
! osxaudiosink enable-last-sample=false buffer-time=20000