#!/bin/sh

#export GST_DEBUG=2,*QOS*:9
#export GST_DEBUG=9,*REGISTRY*:0,*PLUGIN*:0,*REFCOUNTING*:0,*structure*:0
export GST_DEBUG=2
export GST_LAUNCH=/Library/Frameworks/GStreamer.framework/Versions/1.0/bin/gst-launch-1.0

${GST_LAUNCH} -v -e \
  rtpbin name=rtpbin latency=10 \
  avfvideosrc do-timestamp=true device-index=0 \
! "video/x-raw, format=(string)UYVY, width=(int)640, height=(int)480, framerate=(fraction)10000000/333333" \
! videoconvert \
! timeoverlay font-desc="Sans Bold 36" valignment="bottom" halignment="right" \
! tee name=t \
  t. ! videoconvert ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream silent=true ! osxvideosink enable-last-sample=false \
  t. \
! x264enc bitrate=5000 speed-preset=ultrafast tune=zerolatency \
! rtph264pay \
! rtpbin.send_rtp_sink_0 \
  osxaudiosrc do-timestamp=true buffer-time=30000 \
! "audio/x-raw, format=(string)S32LE, layout=(string)interleaved, rate=(int)48000, channels=(int)1" \
! audioconvert \
! rtpL16pay buffer-list=true \
! rtpbin.send_rtp_sink_1 \
  rtpbin.send_rtp_src_0 \
! udpsink enable-last-sample=false sync=false async=false clients=127.0.0.1:10000 \
  rtpbin.send_rtcp_src_0 \
! udpsink enable-last-sample=false sync=false             clients=127.0.0.1:10001 \
  rtpbin.send_rtp_src_1 \
! udpsink enable-last-sample=false sync=false async=false clients=127.0.0.1:10002 \
  rtpbin.send_rtcp_src_1 \
! udpsink enable-last-sample=false sync=false             clients=127.0.0.1:10003