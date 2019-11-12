#include "inc/mainwindow.h"
#include "ui_mainwindow.h"

#include <QChartView>
#include <QDebug>
#include <QQuickItem>
#include <QRunnable>
#include <gst/gst.h>

#include <iostream>

class SetPlaying : public QRunnable
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
    pipeline_ = pipeline ? static_cast<GstElement *>(gst_object_ref(pipeline)) : NULL;
}

SetPlaying::~SetPlaying()
{
    // if (pipeline_)
        // gst_object_unref(pipeline_);
}

void SetPlaying::run()
{
    if (pipeline_)
        gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}

void MainWindow::setupGstPipeline(GstElement *pipeline, QQuickWidget *quickWidget)
{
    pipeline = gst_pipeline_new(nullptr);
    GstElement *src = gst_element_factory_make("udpsrc", "udp");
    GstElement *demux = gst_element_factory_make("rtph264depay", "rtp-h264-depacketizer");
    GstElement *parser = gst_element_factory_make("h264parse", "h264-parser");
    GstElement *decoder = gst_element_factory_make("avdec_h264", "h264-decoder");
    GstElement *videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    GstElement *glupload = gst_element_factory_make("glupload", nullptr);
    GstElement *glcolorconvert = gst_element_factory_make("glcolorconvert", nullptr);
    GstCaps *caps = nullptr;

    if ((caps = gst_caps_from_string(
             "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264")) == nullptr)
    {
        qCritical() << "VideoReceiver::start() failed. Error with gst_caps_from_string()";
    }

    g_object_set(G_OBJECT(src), "caps", caps, nullptr);

    /* the plugin must be loaded before loading the qml file to register the
     * GstGLVideoItem qml item */
    GstElement *sink = gst_element_factory_make("qmlglsink", nullptr);

    g_object_set(G_OBJECT(src), "port", 5600, nullptr);

    g_assert(src && glupload && sink);

    gst_bin_add_many(GST_BIN(pipeline), src, demux, parser, decoder,
                     videoconvert, glupload, glcolorconvert, sink, nullptr);
    if (!gst_element_link_many(src, demux, parser, decoder,
                               videoconvert, glupload, glcolorconvert, sink, nullptr))
    {
        qCritical() << "failed to link elements";
    }

    assert(quickWidget);
    QUrl qmlSource("qrc:/assets/video.qml");
    quickWidget->setSource(qmlSource);

    QQuickItem *videoItem;
    QQuickWindow *qQuickWindows;

    /* find and set the videoItem on the sink */
    videoItem = ui->quickWidget->rootObject();
    videoItem = videoItem->findChild<QQuickItem *>("videoItem");
    assert(videoItem);
    g_object_set(sink, "widget", videoItem, nullptr);

    qQuickWindows = videoItem->window();
    assert(qQuickWindows);
    qQuickWindows->scheduleRenderJob(new SetPlaying(pipeline),
                                     QQuickWindow::BeforeSynchronizingStage);
}

void MainWindow::setupVideo()
{
    setupGstPipeline(pipeline1, ui->quickWidget);
}

void setPiplineState(GstElement *pipeline, int gstState)
{
    gst_element_set_state(pipeline, static_cast<GstState>(gstState));
}
