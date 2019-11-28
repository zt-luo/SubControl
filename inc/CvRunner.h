#include <QObject>
#include <QtGlobal>
#include <QRunnable>
#include <QThreadPool>

#include <opencv2/opencv.hpp>

class CvRunner : public QObject
{
    Q_OBJECT
private:
    cv::Mat frame;

public:
    CvRunner(QObject *parent = nullptr);
    ~CvRunner();
};
