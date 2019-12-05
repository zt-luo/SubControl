#pragma once

#include <QObject>
#include <QtGlobal>
#include <QRunnable>
#include <QThreadPool>

#include <opencv2/opencv.hpp>

using namespace cv;

class CvRunner : public QObject
{
    Q_OBJECT
private:
    int _width;
    int _height;
    Mat _srcFrame;
    Mat _destFrame;

    void imShowConfig();
    void imShow(Mat *frame = nullptr);

public:
    CvRunner(int width, int height, QObject *parent = nullptr);
    ~CvRunner();

    void redBall();

    static void process(char *data, CvRunner *cvRunner);
};
