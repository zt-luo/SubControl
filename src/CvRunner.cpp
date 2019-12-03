#include "inc/CvRunner.h"

using namespace cv;
using namespace std;

CvRunner::CvRunner(int width, int height, QObject *parent)
    : QObject(parent),
      _width(width),
      _height(height)
{
    imShowConfig();
}

CvRunner::~CvRunner()
{
    cv::destroyAllWindows();
}

void CvRunner::imShowConfig()
{
    cv::namedWindow("Frame", cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    cv::resizeWindow("Frame", _width / 2 - 80, _height / 2);
}

void CvRunner::imShow(Mat *frame)
{
    if (frame)
    {
        cv::imshow("Frame", *frame);
    }
    else
    {
        cv::imshow("Frame", _destFrame);
    }
}

void CvRunner::process(char *data, CvRunner *cvRunner)
{
    cvRunner->_srcFrame = Mat(cv::Size(cvRunner->_width, cvRunner->_height), CV_8UC3, data);
    cvRunner->redBall();
    cvRunner->imShow();
}

void CvRunner::redBall()
{
    _destFrame = _srcFrame;

    int g_nHm = 7;
    Mat channel[3];
    split(_srcFrame, channel);
    channel[0] = channel[0].mul(.1 * g_nHm);           //B
    channel[1] = channel[1].mul(.1 * g_nHm);           //G
    channel[2] = channel[2] - channel[0] - channel[1]; //R
    channel[2] *= 4;

    GaussianBlur(channel[2], channel[2], Size(9, 9), 2, 2);

    vector<Vec3f> circles; //3通道 float 型向量
    HoughCircles(channel[2], circles, HOUGH_GRADIENT, 1, _srcFrame.rows / 5, 200, 25, 20, 400);

    for (size_t i = 0; i < circles.size(); i++)
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        //draw the center of the circle
        circle(_destFrame, center, 3, Scalar(0, 0, 0), -1, 8, 0);
        //draw the profile of the ball
        circle(_destFrame, center, radius, Scalar(0, 255, 0), 3, 8, 0);
    }

    imShow();
}
