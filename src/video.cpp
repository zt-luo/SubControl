#include "inc/mainwindow.h"
#include "ui_mainwindow.h"

#include <QChartView>
#include <QDebug>

#include <iostream>


void MainWindow::setupVideo()
{
    _vlcInstance = new VlcInstance(VlcCommon::args() << "--network-caching=0", this);
    _vlcPlayer = new VlcMediaPlayer(_vlcInstance);
    _vlcPlayer->setVideoWidget(ui->video);
    ui->video->setMediaPlayer(_vlcPlayer);
    _vlcMedia = new VlcMedia("file:///home/ztluo/video.mp4", _vlcInstance);
//    _vlcMedia = new VlcMedia("http://192.168.2.2:2770/vlc.sdp", _vlcInstance);
    _vlcPlayer->open(_vlcMedia);
    _vlcPlayer->stop();
    ui->video->hide();

    //    ui->video->resize();
    QPixmap pix(":/assets/picture/background.jpg");
    QPixmap dest=pix.scaled(ui->picture->size());
    ui->picture->setPixmap(pix);

    if (ui->checkBoxVideoLink->checkState() == Qt::Checked)
    {
        _vlcPlayer->play();
        ui->video->show();
    }

    videoOk = true;
}
