#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QMainWindow>

namespace Ui {
class VideoWindow;
}

class VideoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoWindow(QWidget *parent = nullptr);
    ~VideoWindow();

signals:
    void closeWindows();

private:
    Ui::VideoWindow *ui;

    void closeEvent(QCloseEvent *event);
};

#endif // VIDEOWINDOW_H
