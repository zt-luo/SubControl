#pragma once

#include <QObject>
#include <QtGlobal>
#include <QRunnable>
#include <QThreadPool>
#include <QDateTime>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>

#include <gst/gst.h>
#include <gst/app/app.h>
#include <gst/app/gstappsink.h>

#include <opencv2/opencv.hpp>

#include "CvRunner.h"

class VideoReceiver : public QObject
{
    Q_OBJECT
private:
    struct RecordingElement
    {
        VideoReceiver *pThis;
        GstElement *pipeline;
        GstElement *pipelineStopRec;
        GstElement *tee;
        GstElement *queue;
        GstElement *parse;
        GstElement *mux;
        GstElement *sink;
        gboolean removing;
    };

    struct CvElement
    {
        VideoReceiver *pThis;
        GstElement *pipeline;
        GstElement *pipelineStopCV;
        GstElement *tee;
        GstElement *queue;
        GstElement *videoconvert;
        GstElement *sink;
        gboolean removing;
    };
    /* data */
    GstElement *_pipeline;
    GstElement *_teeRecording;
    GstElement *_pipelineStopRec;
    GstElement *_teeCV;
    GstElement *_pipelineStopCV;

    RecordingElement *_recordingElement;

    bool _starting;
    bool _playing;
    bool _pausing;

    bool _recStarting;
    bool _recording;
    bool _recStoping;

    bool _cvProcessing;

public:
    CvElement *_cvElement;

    bool _cvRunning;
    bool _cvStoping;

private:
    bool _verticalFlip;
    bool _mouseOverRecordingButton;

    // video info
    int _videoWidth;
    int _videoHeight;
    int _videoSize;

    // video data
    char *_videoDataRaw;
    GstMapInfo *_videoMap;
    GstBuffer *_videoBuffer;

    CvRunner *_cvRunner;

    static gboolean _onBusMessage(GstBus *bus, GstMessage *message, gpointer data);
    static GstPadProbeReturn _unlinkCallBack(GstPad *pad, GstPadProbeInfo *info, gpointer data);
    static GstPadProbeReturn _keyframeWatch(GstPad *pad, GstPadProbeInfo *info, gpointer data);
    static gboolean newSample(GstAppSink *appsink, gpointer udata);
    static gboolean cvEOS(GstAppSink *appsink, gpointer udata);

    bool startCV();
    void stopCV();

private slots:
    void onPipelineEOS();
    void onPauseCvProcess();
    void onPlayCvProcess();

public:
    Q_PROPERTY(bool recording
                   READ getRecording
                       NOTIFY onRecordingChanged)

    Q_PROPERTY(bool mouseOverRecordingButton
                   READ getMouseOverRecordingButton
                       NOTIFY onMouseOverRecordingButtonChanged)

    VideoReceiver(QObject *parent = nullptr, bool verticalFlip = false);
    ~VideoReceiver();

    virtual bool getRecording() { return _recording; }
    virtual bool getMouseOverRecordingButton() { return _mouseOverRecordingButton; }

    void start(QQuickWidget *quickWidget);
    void play();
    void pause();
    bool startRecording();
    void stopRecording();
    void setRecordingHightlight(bool hightlight);

signals:
    void onRecordingChanged();
    void onMouseOverRecordingButtonChanged();
    void pipelineEOS();
    void pauseCvProcess();
    void playCvProcess();
};
