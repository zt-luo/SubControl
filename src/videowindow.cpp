#include "inc/videowindow.h"
#include "ui_videowindow.h"
#include <QCloseEvent>
#include <QDebug>

VideoWindow::VideoWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoWindow)
{
    ui->setupUi(this);

    QPixmap pix(":/assets/picture/background.jpg");
    QPixmap dest=pix.scaled(ui->picture->size());
    ui->picture->setPixmap(pix);
}

VideoWindow::~VideoWindow()
{
    delete ui;
}

void VideoWindow::showVideo(bool show)
{
    if (show)
    {
        ui->quickWidget->show();
    }
    else
    {
        ui->quickWidget->hide();
    }
}

void VideoWindow::resizeWindowsManual()
{
    int m_width = 0;
    m_width = ui->centralwidget->width();

    ui->quickWidget->setGeometry(0, 0 , m_width, ui->centralwidget->height());
    ui->picture->setGeometry(0, 0 , m_width, ui->centralwidget->height());

    ui->qCompass->setGeometry(m_width - 160, 0, 160, 160);
    ui->qADI->setGeometry(m_width - 320, 0, 160, 160);
}

void VideoWindow::resizeEvent(QResizeEvent* event)
{
    resizeWindowsManual();

    QMainWindow::resizeEvent(event);
}

void VideoWindow::closeEvent(QCloseEvent *event)
{
    emit closeWindows();
    event->accept();
}

void VideoWindow::updateAdiCompass(AS::Vehicle_Data_t *vehicle_data)
{
    float yaw = 0, roll = 0, pitch = 0, depth =0;

    const float degreePerRad = 180.0f / 3.1415926f;
    yaw = vehicle_data->yaw * degreePerRad;
    roll = vehicle_data->roll * degreePerRad;
    pitch = vehicle_data->pitch * degreePerRad;
    depth = vehicle_data->alt / 1000.0f;

    ui->qADI->setData(roll, pitch);
    ui->qCompass->setYaw(yaw);
    ui->qCompass->setAlt(depth);
}

void  VideoWindow::on_updateVehicleDataSignal(AS::Vehicle_Data_t *vehicle_data)
{
    assert(vehicle_data != nullptr);
    updateAdiCompass(vehicle_data);
}
