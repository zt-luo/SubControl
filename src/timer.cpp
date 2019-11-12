#include "inc/mainwindow.h"
#include "ui_mainwindow.h"

#include <QChartView>
#include <QDebug>

#include <iostream>


void MainWindow::setupTimer()
{
    QObject::connect(&vehicleDataUpdateTimer, &QTimer::timeout, this, &MainWindow::updateVehicleData);
    vehicleDataUpdateTimer.setInterval(25);
    vehicleDataUpdateTimer.start();

    QObject::connect(&closeControlTimer, &QTimer::timeout, this, &MainWindow::closeControl);
    closeControlTimer.setInterval(50);
//    depthPidTimer.start();

    QObject::connect(&statusTexTimer, &QTimer::timeout, this, &MainWindow::fetchStatusTex);
    statusTexTimer.setInterval(100);
//    statusTexTimer.start();

    QObject::connect(&namedValueTimer, &QTimer::timeout, this, &MainWindow::updateNamedValue);
    namedValueTimer.setInterval(50);

    QObject::connect(&vehicleCheckTimer, &QTimer::timeout, this, &MainWindow::vehicleCheck);
    vehicleCheckTimer.setInterval(500);
    vehicleCheckTimer.start();

    QObject::connect(&manualControlTimer, &QTimer::timeout, this, &MainWindow::manualControl);
    manualControlTimer.setInterval(50);
    manualControlTimer.start();

    QObject::connect(&thrustersTestTimer, &QTimer::timeout, this, &MainWindow::thrustersTest);
    thrustersTestTimer.setInterval(50);
//    thrustersTestTimer.start();

    QObject::connect(&countScreenTimer, &QTimer::timeout, this, &MainWindow::countScreens);
    countScreenTimer.setInterval(500);
    countScreenTimer.start();
}

void MainWindow::vehicleCheck()
{

    // wait for vehicle to connect, and start timer
    static bool fixedWidth = false;
    bool existActiveVehicle = false;

    QHash<QString, int>::iterator iter;

    for (int i = 1; i < 255; i++)
    {
        QString vehicleID;
        vehicleID.sprintf("%d", i);

        iter = activeVehicleHash.find(vehicleID);

        if (AS::as_api_check_vehicle(static_cast<uint8_t>(i)))
        {
            existActiveVehicle = true;

            // vehicleComboBox index
            int index = activeVehicleHash.count() + 1;

            if(iter == activeVehicleHash.end())
            {
                // new
                activeVehicleHash.insert(vehicleID, index);
                vehicleComboBox->insertItem(i, vehicleID);
                vehicleComboBox->setCurrentIndex(index);
            }

            if (i > 9 && !fixedWidth)
            {
                vehicleComboBox->setFixedWidth(45);
            }

            if (i > 99 && !fixedWidth)
            {
                vehicleComboBox->setFixedWidth(50);
                fixedWidth = true;
            }
        }
        else
        {
            if(iter != activeVehicleHash.end())
            {
                activeVehicleHash.remove(vehicleID);
                vehicleComboBox->removeItem(iter.value());
            }
        }
    }

    if (existActiveVehicle)
    {
        statusTexTimer.start();
        namedValueTimer.start();

        armCheckBox->setEnabled(true);
        modeComboBox->setEnabled(true);
    }
    else
    {
        currentVehicle = 0;

        statusTexTimer.stop();
        namedValueTimer.stop();

        vehicleComboBox->setCurrentIndex(0);

        armCheckBox->setDisabled(true);
        modeComboBox->setDisabled(true);
    }
}

void MainWindow::fetchStatusTex()
{
    if (0 == currentVehicle)
    {
        return;
    }

    AS::mavlink_statustext_t *statustxt = nullptr;
    QString severity_tex[8] = {"EMERGENCY", "ALERT",
                               "CRITICAL", "ERROR",
                               "WARNING", "NOTICE",
                               "INFO", "DEBUG"};
    statustxt = AS::as_api_statustex_queue_pop(currentVehicle);

    QString out_str;
    if (nullptr != statustxt)
    {
        QDateTime current_date_time =QDateTime::currentDateTime();
        QString current_time =current_date_time.toString("[hh:mm:ss.zzz] ");
        out_str.append(current_time);
        out_str.append(severity_tex[statustxt->severity]);
        out_str.append(QString(" - %1").arg(statustxt->text));

        if (statustxt->severity < 4)
        {
            stringToHtml(out_str, Qt::red);
        }
        else if (statustxt->severity < 6)
        {
            stringToHtml(out_str, Qt::darkYellow);
        }
        else
        {
            stringToHtml(out_str, Qt::darkGray);
        }

        ui->textVehicleInfo->append(out_str);
    }
}

void MainWindow::updateChart()
{
    float yaw = 0, roll = 0, pitch = 0, depth = 0;
    static int64_t last_time_us, last_d_time_us;
    int64_t time_us = 0, d_time_us = 0;
    if(0 == AS::as_api_check_vehicle(currentVehicle))
    {
        yaw = 0;
        roll = 0;
        pitch = 0;
        depth = 0;
    }
    else
    {

        const float degreePerRad = 180.0f / 3.1415926f;

        yaw = vehicle_data->yaw * degreePerRad;
        roll = vehicle_data->roll * degreePerRad;
        pitch = vehicle_data->pitch * degreePerRad;
        depth = vehicle_data->alt / 1000.0f;
//        depth = - (vehicle_data->press_abs2 - 1000) / 100;
        time_us = vehicle_data->monotonic_time;
    }

    if (last_time_us == 0)
    {
        d_time_us = 0;
    }
    else
    {
        d_time_us = time_us - last_time_us;
    }

    if (d_time_us < 0)
    {
        d_time_us = last_d_time_us;
    }
    else
    {
        last_d_time_us = d_time_us;
    }

    last_time_us = time_us;

    qreal m_y1[2];
    m_y1[0] = yaw;
    m_y1[1] = roll;
    m_yawRollChart->seriesAppendData(m_y1, d_time_us / 1000000.0);

    qreal m_y2[1];
    m_y2[0] = pitch;
    m_pitchChart->seriesAppendData(m_y2, d_time_us / 1000000.0);

    qreal m_y3[1];
    m_y3[0] = depth;
    m_depthChart->seriesAppendData(m_y3, d_time_us / 1000000.0);
}

void MainWindow::updateAdiCompass()
{
    if (0 == currentVehicle)
    {
        return;
    }

    float yaw = 0, roll = 0, pitch = 0, depth =0;

    const float degreePerRad = 180.0f / 3.1415926f;
    yaw = vehicle_data->yaw * degreePerRad;
    roll = vehicle_data->roll * degreePerRad;
    pitch = vehicle_data->pitch * degreePerRad;
    depth = vehicle_data->alt / 1000.0f;

    ui->qADI->setData(roll, pitch);
    ui->qCompass->setYaw(yaw);
    ui->qCompass->setAlt(depth);

    rollLabelValue->setNum(round(roll * 100) / 100);
    pitchLabelValue->setNum(round(pitch * 100) / 100);
    yawLabelValue->setNum(round(yaw * 100) / 100);
    depthLabelValue->setNum(round(depth * 100) / 100);
}

void MainWindow::updateNamedValue()
{
    if (0 == currentVehicle)
    {
        return;
    }

    if(0 != AS::as_api_check_vehicle(currentVehicle))
    {
        AS::mavlink_named_value_float_t *value_float;

        value_float = AS::as_api_named_val_float_queue_pop(currentVehicle);

        if (nullptr != value_float)
        {
            QHash<QString, float>::iterator iter;
            iter = namedFloatHash.find(value_float->name);
            // new key
            if(iter == namedFloatHash.end())
            {
                namedFloatHash.insert(value_float->name, value_float->value);
                ui->comboBoxL1->addItem(value_float->name);
                ui->comboBoxL2->addItem(value_float->name);
                ui->comboBoxL3->addItem(value_float->name);
            }
            else
            {
                iter.value() = value_float->value;
            }
        }
    }

    QHash<QString, float>::iterator iter;

    iter = namedFloatHash.find(ui->comboBoxL1->currentText());
    if(iter != namedFloatHash.end())
    {
        ui->line1->setText(QString("%1").arg(iter.value()));
    }

    iter = namedFloatHash.find(ui->comboBoxL2->currentText());
    if(iter != namedFloatHash.end())
    {
        ui->line2->setText(QString("%1").arg(iter.value()));
    }

    iter = namedFloatHash.find(ui->comboBoxL3->currentText());
    if(iter != namedFloatHash.end())
    {
        ui->line3->setText(QString("%1").arg(iter.value()));
    }
}

void MainWindow::countScreens()
{
    QList<QScreen *> m_screens = QGuiApplication::screens();

    if (m_screens.count() >= 2)
    {
        ui->actionAdvanceMode->setEnabled(true);
    }
    else
    {
        ui->actionAdvanceMode->setEnabled(false);
    }
}

void MainWindow::updateVehicleData()
{
    if (currentVehicle == 0)
    {
        return;
    }

    AS::as_api_get_vehicle_data2(currentVehicle, vehicle_data);

    updateChart();

    if (ui->stackedWidgetMain->currentIndex() == 0)
    {
        updateAdiCompass();
    }

    if (videoWindow->isHidden() != true)
    {
        emit updateVehicleDataSignal(vehicle_data);
    }
}
