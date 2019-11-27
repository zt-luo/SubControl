#include "VideoReceiver.h"

class
    SetPlaying : public QRunnable
{
public:
    SetPlaying(GstElement *);
    ~SetPlaying();

    void run();

private:
    GstElement *pipeline_;
};

SetPlaying::SetPlaying(GstElement *pipeline)
{
    pipeline_ = pipeline ? static_cast<GstElement *>(gst_object_ref(pipeline)) : nullptr;
}

SetPlaying::~SetPlaying()
{
    // if (pipeline_)
    // gst_object_unref(pipeline_);
}

void SetPlaying::run()
{
    if (pipeline_)
    {
        gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    }
}

class
    StopCvJob : public QRunnable
{
public:
    StopCvJob(VideoReceiver *);
    ~StopCvJob();

    void run();

private:
    VideoReceiver *_pVideoReceiver;
};

StopCvJob::StopCvJob(VideoReceiver *pVideoReceiver)
{
    _pVideoReceiver = pVideoReceiver ? pVideoReceiver : nullptr;
}

StopCvJob::~StopCvJob()
{
}

void StopCvJob::run()
{
    if (_pVideoReceiver)
    {
        if (_pVideoReceiver->_cvStoping)
        {
            gst_bin_remove_many(GST_BIN(_pVideoReceiver->_cvElement->pipelineStopCV),
                                _pVideoReceiver->_cvElement->queue,
                                _pVideoReceiver->_cvElement->sink,
                                nullptr);

            gst_element_set_state(_pVideoReceiver->_cvElement->pipelineStopCV, GST_STATE_NULL);
            gst_object_unref(_pVideoReceiver->_cvElement->pipelineStopCV);
            _pVideoReceiver->_cvElement->pipelineStopCV = nullptr;

            //TODO: fix this
            gst_element_set_state(_pVideoReceiver->_cvElement->sink, GST_STATE_NULL);
            gst_element_set_state(_pVideoReceiver->_cvElement->queue, GST_STATE_NULL);

            gst_object_unref(_pVideoReceiver->_cvElement->queue);
            gst_object_unref(_pVideoReceiver->_cvElement->sink);

            _pVideoReceiver->_cvRunning = false;

            qDebug() << "CV Stopped.";
        }
    }
}


VideoReceiver::VideoReceiver(QObject *parent)
    : QObject(parent),
      _recordingElement(new RecordingElement),
      _cvElement(new CvElement),
      _starting(false),
      _playing(false),
      _pausing(false),
      _recStarting(false),
      _recording(false),
      _recStoping(false),
      _cvRunning(false),
      _cvStoping(false)
{
    _pipeline = gst_pipeline_new("receiver");
    _teeRecording = gst_element_factory_make("tee", "tee-recording");
    _teeCV = gst_element_factory_make("tee", "tee-opencv");

    g_assert(_pipeline && _teeRecording && _teeCV);

    connect(this, &VideoReceiver::pipelineEOS, this, &VideoReceiver::onPipelineEOS);
}

VideoReceiver::~VideoReceiver()
{
    delete _recordingElement;

    g_object_unref(_pipeline);
    g_object_unref(_teeRecording);
}

void VideoReceiver::start(QQuickWidget *quickWidget)
{
    if (_starting)
    {
        qDebug() << "staring";
    }

    GstElement *src = gst_element_factory_make("udpsrc", "udpsrc");
    GstElement *demux = gst_element_factory_make("rtph264depay", "rtp-h264-depacketizer");
    GstElement *parser = gst_element_factory_make("h264parse", "h264-parser");
    GstElement *queue = gst_element_factory_make("queue", "queue-recording-main");
    GstElement *decoder = gst_element_factory_make("avdec_h264", "h264-decoder");
    GstElement *videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    GstElement *queue2 = gst_element_factory_make("queue", "queue-cv-main");
    GstElement *glupload = gst_element_factory_make("glupload", "glupload");
    GstElement *glcolorconvert = gst_element_factory_make("glcolorconvert", "glcolorconvert");
    GstCaps *caps = nullptr;

    g_assert(src && demux && parser && queue && decoder && videoconvert &&
             queue2 && glupload && glcolorconvert);

    if ((caps = gst_caps_from_string(
             "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264")) == nullptr)
    {
        qCritical() << "Error with gst_caps_from_string()";
    }

    g_object_set(G_OBJECT(src), "caps", caps, nullptr);

    /* the plugin must be loaded before loading the qml file to register the
     * GstGLVideoItem qml item */
    GstElement *qmlsink = gst_element_factory_make("qmlglsink", "qmlsink");

    g_object_set(G_OBJECT(src), "port", 5600, nullptr);

    g_assert(src && glupload && qmlsink);

    gst_bin_add_many(GST_BIN(_pipeline), src, demux, parser, _teeRecording, queue, decoder,
                     videoconvert, _teeCV, queue2, glupload, glcolorconvert, qmlsink,
                     nullptr);
    if (!gst_element_link_many(src, demux, parser, _teeRecording, queue, decoder,
                               videoconvert, _teeCV, queue2, glupload, glcolorconvert, qmlsink,
                               nullptr))
    {
        qCritical() << "failed to link elements";
    }

    assert(quickWidget);
    quickWidget->rootContext()->setContextProperty("videoReceiver", this);
    QUrl qmlSource("qrc:/assets/video.qml");
    quickWidget->setSource(qmlSource);

    QQuickItem *videoItem;
    QQuickWindow *qQuickWindows;

    /* find and set the videoItem on the sink */
    videoItem = quickWidget->rootObject()->findChild<QQuickItem *>("videoItem");
    assert(videoItem);
    g_object_set(qmlsink, "widget", videoItem, nullptr);

    qQuickWindows = videoItem->window();
    assert(qQuickWindows);
    qQuickWindows->scheduleRenderJob(new SetPlaying(_pipeline),
                                     QQuickWindow::BeforeSynchronizingStage);

    // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");

    // Add handler for EOS event
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));
    gst_bus_enable_sync_message_emission(bus);
    g_signal_connect(bus, "sync-message", G_CALLBACK(_onBusMessage),
                     this);
    gst_object_unref(bus);
    bus = nullptr;

    _starting = true;
}

void VideoReceiver::play()
{
    if (_pausing)
    {
        gst_element_set_state(_pipeline, GST_STATE_PLAYING);
    }
}

void VideoReceiver::pause()
{
    if (_playing)
    {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
    }

    if (_recording)
    {
    }

    _pausing = true;
}

bool VideoReceiver::startRecording()
{
    if (!_playing)
    {
        qDebug() << "no video stream!";
        return false;
    }

    if (_recording)
    {
        qDebug() << "in recording processing!";
        return false;
    }

    if (_recStarting)
    {
        qDebug() << "in recStarting processing!";
        return false;
    }

    _recStarting = true;

    _pipelineStopRec = gst_pipeline_new("pipelineStopRec");

    GstElement *queue = gst_element_factory_make("queue", "queue-recording");
    GstElement *parse = gst_element_factory_make("h264parse", "h264-parser-recording");
    GstElement *mux = gst_element_factory_make("matroskamux", "matroska-mux");
    GstElement *filesink = gst_element_factory_make("filesink", "mkv-filesink");

    g_assert(_pipelineStopRec && queue && parse && mux && filesink);

    g_object_set(static_cast<gpointer>(mux), "streamable", false, nullptr);
    g_object_set(static_cast<gpointer>(mux), "writing-app", qPrintable("SubControl"), nullptr);

    QString videoFile = "";
    videoFile += QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss") + ".mkv";

    g_object_set(static_cast<gpointer>(filesink), "location", qPrintable(videoFile), nullptr);

    gst_bin_add_many(GST_BIN(_pipeline), queue, parse, mux, nullptr);
    gst_element_link_many(queue, parse, mux, nullptr);

    gst_element_sync_state_with_parent(queue);
    gst_element_sync_state_with_parent(parse);
    gst_element_sync_state_with_parent(mux);

    _recordingElement->pThis = this;
    _recordingElement->pipeline = _pipeline;
    _recordingElement->pipelineStopRec = _pipelineStopRec;
    _recordingElement->tee = _teeRecording;
    _recordingElement->queue = queue;
    _recordingElement->parse = parse;
    _recordingElement->mux = mux;
    _recordingElement->sink = filesink;
    _recordingElement->removing = false;

    // Install a probe on the recording branch to drop buffers until we hit our first keyframe.
    // When we hit our first keyframe,
    // we can offset the timestamps appropriately according to the first keyframe time
    // This will ensure the first frame is a keyframe at t=0,
    // and decoding can begin immediately on playback
    // Once we have this valid frame, we attach the filesink.
    // Attaching it here would cause the filesink to fail to preroll
    // and to stall the pipeline for a few seconds.
    GstPad *probepad = gst_element_get_static_pad(queue, "src");
    gst_pad_add_probe(probepad,
                      (GstPadProbeType)(GST_PAD_PROBE_TYPE_BUFFER),
                      /* | GST_PAD_PROBE_TYPE_BLOCK */
                      _keyframeWatch, _recordingElement, nullptr);
    // to drop the buffer or to block the buffer?
    gst_object_unref(probepad);

    // Link the recording branch to the pipeline
    GstPad *teepad = gst_element_get_request_pad(_teeRecording, "src_%u");
    GstPad *sinkpad = gst_element_get_static_pad(queue, "sink");
    g_assert(teepad && sinkpad);

    if (gst_pad_link(teepad, sinkpad) != GST_PAD_LINK_OK)
    {
        qCritical() << "link failed!";
    }

    gst_object_unref(sinkpad);

    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline-recording");


    return true;;
}

void VideoReceiver::stopRecording()
{
    // exit immediately if we are not recording
    if (!_playing || !_recording)
    {
        qDebug() << "Not recording!";
        return;
    }

    if (_recStoping)
    {
        qDebug() << "in recStoping processing!";
        return;
    }

    _recStoping = true;

    // Wait for data block before unlinking
    gst_pad_add_probe(gst_element_get_request_pad(_teeRecording, "src_%u"),
                      GST_PAD_PROBE_TYPE_IDLE, _unlinkCallBack, _recordingElement, nullptr);
    // _recording = false;
    // emit onRecordingChanged();
}

GstPadProbeReturn VideoReceiver::_keyframeWatch(GstPad *pad,
                                                GstPadProbeInfo *info,
                                                gpointer user_data)
{
    Q_UNUSED(pad);
    if (info != nullptr && user_data != nullptr)
    {
        GstBuffer *buf = gst_pad_probe_info_get_buffer(info);
        if (GST_BUFFER_FLAG_IS_SET(buf, GST_BUFFER_FLAG_DELTA_UNIT))
        { // wait for a keyframe
            return GST_PAD_PROBE_DROP;
        }
        else
        {
            struct RecordingElement *recordingElement = (struct RecordingElement *)user_data;
            // reset the clock
            GstClock *clock = gst_pipeline_get_clock(GST_PIPELINE(recordingElement->pipeline));
            GstClockTime time = gst_clock_get_time(clock);
            gst_object_unref(clock);
            gst_element_set_base_time(recordingElement->pipeline, time);
            // offset pipeline timestamps to start at zero again
            buf->dts = 0;
            // The offset will not apply to this current buffer, our first frame, timestamp is zero
            buf->pts = 0;

            // Add the filesink once we have a valid I-frame
            gst_bin_add_many(GST_BIN(recordingElement->pipeline), recordingElement->sink, nullptr);
            gst_element_link_many(recordingElement->mux, recordingElement->sink, nullptr);
            gst_element_sync_state_with_parent(recordingElement->sink);

            recordingElement->pThis->_recording = true;
            recordingElement->pThis->_recStarting = false;
            emit recordingElement->pThis->onRecordingChanged();

            qDebug() << "Got keyframe, stop dropping buffers";
        }
    }

    return GST_PAD_PROBE_REMOVE;
}

GstPadProbeReturn
VideoReceiver::_unlinkCallBack(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    Q_UNUSED(pad);

    if (info != nullptr && user_data != nullptr)
    {
        RecordingElement *recordingElement = (RecordingElement *)user_data;
        // We will only act once
        if (g_atomic_int_compare_and_exchange(&recordingElement->removing, false, true))
        {
            // unlinks
            gst_bin_remove_many(GST_BIN(recordingElement->pipeline),
                                recordingElement->queue,
                                recordingElement->parse, recordingElement->mux,
                                recordingElement->sink, nullptr);

            // Give tee its pad back
            GstPad *teepad = gst_element_get_request_pad(recordingElement->tee, "src_%u");
            gst_element_release_request_pad(recordingElement->tee, teepad);
            gst_object_unref(teepad);
            teepad = nullptr;

            // Create temporary pipeline
            recordingElement->pipelineStopRec = gst_pipeline_new("pipeStopRec");

            // Put our elements from the recording branch into the temporary pipeline
            gst_bin_add_many(GST_BIN(recordingElement->pipelineStopRec),
                             recordingElement->queue,
                             recordingElement->parse,
                             recordingElement->mux,
                             recordingElement->sink, nullptr);
            gst_element_link_many(recordingElement->queue,
                                  recordingElement->parse,
                                  recordingElement->mux,
                                  recordingElement->sink, nullptr);

            // Add handler for EOS event
            GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(recordingElement->pipelineStopRec));
            gst_bus_enable_sync_message_emission(bus);
            g_signal_connect(bus, "sync-message", G_CALLBACK(_onBusMessage),
                             recordingElement->pThis);
            gst_object_unref(bus);
            bus = nullptr;

            if (gst_element_set_state(recordingElement->pipelineStopRec, GST_STATE_PLAYING) ==
                GST_STATE_CHANGE_FAILURE)
            {
                qDebug() << "problem starting _pipelineStopRec";
            }

            // Send EOS at the beginning of the pipeline
            GstPad *sinkpad = gst_element_get_static_pad(recordingElement->queue, "sink");
            gst_pad_send_event(sinkpad, gst_event_new_eos());
            gst_object_unref(sinkpad);
            sinkpad = nullptr;
            qDebug() << "Recording branch unlinked";
        }
    }
    return GST_PAD_PROBE_REMOVE;
}

gboolean VideoReceiver::_onBusMessage(GstBus *bus, GstMessage *msg, gpointer data)
{
    Q_UNUSED(bus)
    Q_ASSERT(msg != nullptr && data != nullptr);
    VideoReceiver *pThis = (VideoReceiver *)data;

    switch (GST_MESSAGE_TYPE(msg))
    {

    case (GST_MESSAGE_ERROR):
    {
        gchar *debug;
        GError *error;
        gst_message_parse_error(msg, &error, &debug);
        g_free(debug);
        qCritical() << error->message;
        g_error_free(error);
        qDebug() << "GStreamer error!";
        // pThis->stop();
    }
    break;

    case (GST_MESSAGE_EOS):

        // do clean work in slot
        emit pThis->pipelineEOS();

        break;

    case (GST_MESSAGE_STATE_CHANGED):

        if (pThis->_pipeline &&
            pThis->_playing != (GST_STATE(pThis->_pipeline) == GST_STATE_PLAYING))
        {
            pThis->_playing = GST_STATE(pThis->_pipeline) == GST_STATE_PLAYING;
            qDebug() << "State changed, _playing:" << pThis->_playing;
        }
        break;

    default:
        break;
    }

    return TRUE;
}

void VideoReceiver::setRecordingHightlight(bool hightlight)
{
    _mouseOverRecordingButton = hightlight;
    emit onMouseOverRecordingButtonChanged();
}

void VideoReceiver::onPipelineEOS()
{
    if (_recStoping)
    {
        gst_bin_remove(GST_BIN(_recordingElement->pipelineStopRec), _recordingElement->queue);
        gst_bin_remove(GST_BIN(_recordingElement->pipelineStopRec), _recordingElement->parse);
        gst_bin_remove(GST_BIN(_recordingElement->pipelineStopRec), _recordingElement->mux);
        gst_bin_remove(GST_BIN(_recordingElement->pipelineStopRec), _recordingElement->sink);

        gst_element_set_state(_recordingElement->pipelineStopRec, GST_STATE_NULL);
        gst_object_unref(_recordingElement->pipelineStopRec);
        _recordingElement->pipelineStopRec = nullptr;

        gst_element_set_state(_recordingElement->sink, GST_STATE_NULL);
        gst_element_set_state(_recordingElement->parse, GST_STATE_NULL);
        gst_element_set_state(_recordingElement->mux, GST_STATE_NULL);
        gst_element_set_state(_recordingElement->queue, GST_STATE_NULL);

        gst_object_unref(_recordingElement->queue);
        gst_object_unref(_recordingElement->parse);
        gst_object_unref(_recordingElement->mux);
        gst_object_unref(_recordingElement->sink);

        _recording = false;
        emit onRecordingChanged(); // for qml button

        _recStoping = false;

        qDebug() << "Recording Stopped.";
    }

    if (_cvStoping)
    {
        QThreadPool *threadPool = new QThreadPool;
        threadPool->start(new StopCvJob(this));
    }
}

bool VideoReceiver::startCV()
{
    if (_cvRunning)
    {
        qDebug() << "CV is running.";
        return false;
    }

    gst_debug_set_default_threshold(GST_LEVEL_WARNING);
    _pipelineStopCV = gst_pipeline_new("pipelineOpenCV");

    GstElement *queue = gst_element_factory_make("queue", "queue-cv");
    GstElement *appsink = gst_element_factory_make("appsink", "appsink");

    g_assert(_pipelineStopCV && queue && appsink);

    gst_app_sink_set_emit_signals((GstAppSink *)appsink, true);
    gst_app_sink_set_drop((GstAppSink *)appsink, true);
    gst_app_sink_set_max_buffers((GstAppSink *)appsink, 1);

    gst_bin_add_many(GST_BIN(_pipeline), queue, appsink, nullptr);
    gst_element_link_many(queue, appsink, nullptr);

    gst_element_sync_state_with_parent(queue);
    gst_element_sync_state_with_parent(appsink);

    _cvElement->pThis = this;
    _cvElement->pipeline = _pipeline;
    _cvElement->tee = _teeCV;
    _cvElement->queue = queue;
    _cvElement->sink = appsink;
    _cvElement->removing = false;

    g_signal_connect(appsink, "new-sample", G_CALLBACK(newSample), (gpointer)this);
    g_signal_connect(appsink, "eos", G_CALLBACK(cvEOS), (gpointer)_cvElement);

    // Link the recording branch to the pipeline
    GstPad *teepad = gst_element_get_request_pad(_teeCV, "src_%u");
    GstPad *sinkpad = gst_element_get_static_pad(queue, "sink");
    g_assert(teepad && sinkpad);

    if (gst_pad_link(teepad, sinkpad) != GST_PAD_LINK_OK)
    {
        qCritical() << "link failed!";
    }

    gst_object_unref(sinkpad);

    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_FULL_PARAMS, "pipelineOpenCV");

    gst_debug_set_default_threshold(GST_LEVEL_ERROR);

    _cvRunning = true;

    // QThreadPool *threadPool = new QThreadPool;
    // threadPool->start(new cvJob((GstAppSink *)appsink));

    return true;
}

void VideoReceiver::stopCV()
{
    if (!_cvRunning)
    {
        qDebug() << "CV is NOT running.";
        return;
    }

    // We will only act once
    if (g_atomic_int_compare_and_exchange(&_cvElement->removing, false, true))
    {
        // unlinks
        gst_bin_remove_many(GST_BIN(_pipeline),
                            _cvElement->queue, _cvElement->sink,
                            nullptr);

        // Give tee its pad back
        GstPad *teepad = gst_element_get_request_pad(_cvElement->tee, "src_%u");
        gst_element_release_request_pad(_cvElement->tee, teepad);
        gst_object_unref(teepad);
        teepad = nullptr;

        // Create temporary pipeline
        _cvElement->pipelineStopCV = gst_pipeline_new("pipelineStopCV");

        // Put our elements from the recording branch into the temporary pipeline
        gst_bin_add_many(GST_BIN(_cvElement->pipelineStopCV),
                         _cvElement->queue, _cvElement->sink,
                         nullptr);
        gst_element_link_many(_cvElement->queue, _cvElement->sink,
                              nullptr);

        if (gst_element_set_state(_cvElement->pipelineStopCV, GST_STATE_PLAYING) ==
            GST_STATE_CHANGE_FAILURE)
        {
            qDebug() << "problem starting _pipelineStopCV";
        }

        // Send EOS at the beginning of the pipeline
        GstPad *sinkpad = gst_element_get_static_pad(_cvElement->queue, "sink");
        gst_pad_send_event(sinkpad, gst_event_new_eos());
        sinkpad = gst_element_get_static_pad(_cvElement->sink, "sink");
        gst_pad_send_event(sinkpad, gst_event_new_eos());
        gst_object_unref(sinkpad);
        sinkpad = nullptr;
        qDebug() << "CV branch unlinked";
    }
}

gboolean VideoReceiver::newSample(GstAppSink *appsink, gpointer udata)
{
    VideoReceiver *pThis = (VideoReceiver *)udata;

    int height, width, size;

    GstMemory *mem;

    GstSample *sample = gst_app_sink_pull_sample(appsink);

    if (sample)
    {
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        g_assert(buffer);

        GstCaps *cap = gst_sample_get_caps(sample);
        GstStructure *s = gst_caps_get_structure(cap, 0);
        gst_structure_get_int(s, "height", &height);
        gst_structure_get_int(s, "width", &width);
        // The caps remain valid as long as sample is valid.
        // gst_caps_unref(cap);

        size = gst_buffer_get_size(buffer);

        mem = gst_buffer_get_memory(buffer, 0);

        gst_memory_unref(mem);

        qDebug() << "height: " << height << "width: " << width << "size: " << size;

        // The buffer remains valid as long as sample is valid.
        // gst_buffer_unref(buffer);
        gst_sample_unref(sample);
    }

    return false;
}

gboolean VideoReceiver::cvEOS(GstAppSink *appsink, gpointer udata)
{
    VideoReceiver *pThis = ((CvElement *)udata)->pThis;

    pThis->_cvStoping = true;
    emit pThis->pipelineEOS();

    return true;
}
