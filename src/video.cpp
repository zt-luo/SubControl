#include "inc/mainwindow.h"
#include "ui_mainwindow.h"

#include <QChartView>
#include <QDebug>
#include <QQuickItem>
#include <QRunnable>
#include <gst/gst.h>

#include <iostream>

void MainWindow::setupVideo()
{
    QPixmap pix(":/assets/picture/background.jpg");
    QPixmap dest=pix.scaled(ui->picture->size());
    ui->picture->setPixmap(pix);

    videoReceiver->start(ui->quickWidget);
}
