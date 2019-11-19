#include <QObject>
#include <QtGlobal>
#include <QRunnable>
#include <QDateTime>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>

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

    bool _mouseOverRecordingButton;

    static gboolean _onBusMessage(GstBus *bus, GstMessage *message, gpointer data);
    static GstPadProbeReturn _unlinkCallBack(GstPad *pad, GstPadProbeInfo *info, gpointer data);
    static GstPadProbeReturn _keyframeWatch(GstPad *pad, GstPadProbeInfo *info, gpointer data);

public:
    Q_PROPERTY(bool recording
                   READ getRecording
                       NOTIFY onRecordingChanged)

    Q_PROPERTY(bool mouseOverRecordingButton
                   READ getMouseOverRecordingButton
                       NOTIFY onMouseOverRecordingButtonChanged)

    VideoReceiver(QObject *parent = nullptr);
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
};
