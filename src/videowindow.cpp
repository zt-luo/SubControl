#include "inc/videowindow.h"
#include "ui_videowindow.h"
#include <QCloseEvent>

VideoWindow::VideoWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoWindow)
{
    ui->setupUi(this);
}

VideoWindow::~VideoWindow()
{
    delete ui;
}

void VideoWindow::closeEvent(QCloseEvent *event)
{
    emit closeWindows();
    event->accept();
}
