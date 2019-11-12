
gst-launch-1.0 -v v4l2src device=/dev/video4 do-timestamp=true ! video/x-h264 ! h264parse ! queue ! rtph264pay config-interval=10 pt=96 ! udpsink host=127.0.0.1 port=5600