#include <QObject>
#include <QtGlobal>
#include <QRunnable>
#include <QDateTime>
#include <QQuickWidget>
#include <QQuickItem>

#include <gst/gst.h>

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
    /* data */
    GstElement *_pipeline;
    GstElement *_tee;
    GstElement *_pipelineStopRec;

    RecordingElement *_recordingElement;

    bool _starting;
    bool _playing;
    bool _pausing;
    bool _recording;

    static gboolean _onBusMessage(GstBus *bus, GstMessage *message, gpointer data);
    static GstPadProbeReturn _unlinkCallBack(GstPad *pad, GstPadProbeInfo *info, gpointer data);
    static GstPadProbeReturn _keyframeWatch(GstPad *pad, GstPadProbeInfo *info, gpointer data);

public:
    VideoReceiver(QObject *parent = nullptr);
    ~VideoReceiver();

    void start(QQuickWidget *quickWidget);
    void play();
    void pause();
    void startRecording();
    void stopRecording();
};
