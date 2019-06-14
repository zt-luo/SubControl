#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QChartView>
#include <QDebug>

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_yawRollChart(new YawRollChart),
    m_yawRollScene(new QGraphicsScene),
    m_pitchChart(new PitchChart),
    m_pitchScene(new QGraphicsScene),
    m_depthChart(new DepthChart),
    m_depthScene(new QGraphicsScene),
    currentVehicle(0)
{
    ui->setupUi(this);
    setupToolBars();

    // yawRoll chart
    // m_yawChart->setTitle("Yaw/degree");
    m_yawRollScene->addItem(m_yawRollChart);
    ui->yawRollView->setScene(m_yawRollScene);
    m_yawRollScene->addItem(m_yawRollChart->legendText[0]);
    m_yawRollScene->addItem(m_yawRollChart->legendText[1]);

    // pitch chart
    m_pitchScene->addItem(m_pitchChart);
    ui->pitchView->setScene(m_pitchScene);
    m_pitchScene->addItem(m_pitchChart->legendText[0]);

    // depth chart
    m_depthScene->addItem(m_depthChart);
    ui->depthView->setScene(m_depthScene);
    m_depthScene->addItem(m_depthChart->legendText[0]);

    vehicle_data_g = new AS::Vehicle_Data_t;

    std::string ip("192.168.2.");
    AS::as_api_init(ip.c_str(), F_THREAD_NONE);

    setupTimer();
    setupVideo();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    static int count;
    if (count > 0)
    {
        setChartsSize();
        ui->vedio->setGeometry(0, 0 , ui->stackedWidgetVideo->width(), ui->stackedWidgetVideo->height());

        ui->qCompass->setGeometry(ui->stackedWidgetVideo->width() - 160, 0, 160, 160);
        ui->qADI->setGeometry(ui->stackedWidgetVideo->width() - 320, 0, 160, 160);
    }
    else
    {
        count++;
    }

    QMainWindow::resizeEvent(event);
}

void MainWindow::setChartsSize()
{
    m_yawRollChart->setGeometry(0, 0, ui->yawRollView->geometry().width(),
                                ui->yawRollView->geometry().height());
    m_yawRollScene->setSceneRect(0, 0, ui->yawRollView->geometry().width(),
                                 ui->yawRollView->geometry().height());

    m_pitchChart->setGeometry(0, 0, ui->pitchView->geometry().width(),
                              ui->pitchView->geometry().height());
    m_pitchScene->setSceneRect(0, 0, ui->pitchView->geometry().width(),
                               ui->pitchView->geometry().height());

    m_depthChart->setGeometry(0, 0, ui->depthView->geometry().width(),
                              ui->depthView->geometry().height());
    m_depthScene->setSceneRect(0, 0, ui->depthView->geometry().width(),
                               ui->depthView->geometry().height());
}

void MainWindow::setupToolBars()
{
    vehicleLable = new QLabel;
    vehicleLable->setText("Vehicle: ");
    ui->vehicleToolBar->addWidget(vehicleLable);

    vehicleComboBox = new QComboBox;
    vehicleComboBox->addItem("/");
    QModelIndex index = vehicleComboBox->model()->index(0, 0);
    QVariant v(0);
    vehicleComboBox->model()->setData(index, v, Qt::UserRole - 1);
    vehicleComboBox->setFixedWidth(40);
    ui->vehicleToolBar->addWidget(vehicleComboBox);
    QObject::connect (vehicleComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
                     this, &MainWindow::on_vehicleComboBox_currentIndexChanged);

    QList<QAction *> actionList;
    actionList.append(ui->actionDisarm);
    ui->vehicleToolBar->addActions(actionList);

    armCheckBox = new QCheckBox;
    armCheckBox->setText("Arm");
    ui->vehicleToolBar->addWidget(armCheckBox);
    QObject::connect(armCheckBox, &QCheckBox::stateChanged,
                     this, &MainWindow::on_armCheckBox_stateChanged);

    ui->vehicleToolBar->addSeparator();
    modeLable = new QLabel;
    modeLable->setText("Mode: ");
    ui->vehicleToolBar->addWidget(modeLable);

    modeComboBox = new QComboBox;
    modeComboBox->addItem("MANUAL");
    modeComboBox->addItem("STABILIZE");
    modeComboBox->addItem("ACRO");
    modeComboBox->addItem("ALT_HOLD");
    modeComboBox->addItem("POSHOLD");
    modeComboBox->addItem("LAB_REMOTE");
    modeComboBox->addItem("LAB_ALT_HOLD");
    ui->vehicleToolBar->addWidget(modeComboBox);
    QObject::connect(modeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     this, &MainWindow::on_modeComboBox_currentIndexChanged);

    ui->actionDisarm->setDisabled(true);
}

void MainWindow::stringToHtml(QString &str, QColor _color)
{
    // html filter
    str.replace("&","&amp;");
    str.replace(">","&gt;");
    str.replace("<","&lt;");
    str.replace("\"","&quot;");
    str.replace("\'","&#39;");
    str.replace(" ","&nbsp;");
    str.replace("\n","<br>");
    str.replace("\r","<br>");

    QByteArray array;
    array.append(static_cast<char>(_color.red()));
    array.append(static_cast<char>(_color.green()));
    array.append(static_cast<char>(_color.blue()));
    QString strC(array.toHex());

    str = QString("<span style=\" color:#%1;\">%2</span>").arg(strC).arg(str);
}

void MainWindow::setupTimer()
{
    QObject::connect(&depthPidTimer, &QTimer::timeout, this, &MainWindow::depthPidControl);
    depthPidTimer.setInterval(25);
//    depthPidTimer.start();

    QObject::connect(&statusTexTimer, &QTimer::timeout, this, &MainWindow::fetchStatusTex);
    statusTexTimer.setInterval(100);
//    statusTexTimer.start();

    QObject::connect(&chartTimer, &QTimer::timeout, this, &MainWindow::updateChart);
    chartTimer.setInterval(10);
//    chartTimer.start();

    QObject::connect(&adiCompassTimer, &QTimer::timeout, this, &MainWindow::updateAdiCompass);
    adiCompassTimer.setInterval(20);
//    adiCompassTimer.start();

    QObject::connect(&namedValueTimer, &QTimer::timeout, this, &MainWindow::updateNamedValue);
    namedValueTimer.setInterval(20);

    QObject::connect(&vehicleCheckTimer, &QTimer::timeout, this, &MainWindow::vehicleCheck);
    vehicleCheckTimer.setInterval(1000);
    vehicleCheckTimer.start();
}

void MainWindow::setupVideo()
{
    _vlcInstance = new VlcInstance(VlcCommon::args() << "--network-caching=0", this);
    _vlcPlayer = new VlcMediaPlayer(_vlcInstance);
    _vlcPlayer->setVideoWidget(ui->vedio);
    ui->vedio->setMediaPlayer(_vlcPlayer);

//    _vlcMedia = new VlcMedia("file:///home/ztluo/video.mp4", _vlcInstance);
    _vlcMedia = new VlcMedia("http://192.168.2.2:2770/vlc.sdp", _vlcInstance);
    _vlcPlayer->open(_vlcMedia);
}

void MainWindow::depthPidControl()
{
//    AS::as_api_set_mode(1, AS::LAB_REMOTE);
    double zD, kP, kI, kD;

    zD = ui->doubleSpinBoxZd->value();
    kP = ui->doubleSpinBoxKp->value();
    kI = ui->doubleSpinBoxKi->value();
    kD = ui->doubleSpinBoxKd->value();

    int pwm_out = 1500, pwm_limit = 150;

    uint32_t dt = 25; // ms

    double I_term_max = -600; // I_term_min and max prevent integral windup by saturating the I_term
    double I_term_min = -700; //
    double z_vel_lim = 1;     // [m/s] this is added to prevent setpoint kick in the derivative term

    double P_term = 0;
    double I_term = 0;
    double D_term = 0;
    double z_now = 0; // depth in this iteration
    double z_old = 0; // depth in the previous iteration
    double z_err = 0; // declare the depth error
    double z_vel = 0; // declare the derivative term for z

    if(0 == AS::as_api_check_vehicle(currentVehicle))
    {
    }
    else
    {
        AS::Vehicle_Data_t *vehicle_data;
        vehicle_data = AS::as_api_get_vehicle_data(currentVehicle);
        if (nullptr != vehicle_data)
        {
            z_now = vehicle_data->alt / 1000.0; // m
        }
    }

    z_err = zD - z_now;
    z_vel = (z_now - z_old) / (dt / 1000);

    // clamp the value of z_vel
    if (z_vel < -z_vel_lim)
    {
        z_vel = -z_vel_lim;
    }
    if (z_vel > z_vel_lim)
    {
        z_vel = z_vel_lim;
    }

    P_term = kP * z_err;
    if (abs(kI) < 0.0000001)
    {
        I_term = 0;
    }
    else
    {
        if (zD < -0.2) // dont do this while disarm
        {
            I_term = I_term + kI * z_err * (dt / 1000);
            // clamp the value of I_term
            if (I_term < I_term_min)
            {
                I_term = I_term_min;
            }
            if (I_term > I_term_max)
            {
                I_term = I_term_max;
            }
        }
    }
    D_term = kD * z_vel;

    pwm_out = static_cast<int>(P_term + I_term + D_term);

    // clamp the value of pwm_out
    if (pwm_out < -pwm_limit)
    {
        pwm_out = -pwm_limit;
    }
    if (pwm_out > pwm_limit)
    {
        pwm_out = pwm_limit;
    }

    if (ui->depthPidCheckBox->checkState())
    {
        AS::as_api_send_rc_channels_override(currentVehicle, 1,
                                             1500, 1500,
                                             1500, 1500,
                                             static_cast<uint16_t>(1500 + pwm_out),
                                             static_cast<uint16_t>(1500 - pwm_out),
                                             static_cast<uint16_t>(1500 - pwm_out),
                                             static_cast<uint16_t>(1500 + pwm_out));
    }
    else
    {
        if(0 != AS::as_api_check_vehicle(currentVehicle))
        {
            AS::as_api_send_rc_channels_override(currentVehicle, 1, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500);
        }
    }


    QString out_str;
    ui->textLogInfo->append(out_str.sprintf("%d", pwm_out));
}

void MainWindow::fetchStatusTex()
{
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
            stringToHtml(out_str, Qt::magenta);
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
        AS::Vehicle_Data_t *vehicle_data;
        vehicle_data = new AS::Vehicle_Data_t;

        vehicle_data = AS::as_api_get_vehicle_data(currentVehicle);

        if (nullptr != vehicle_data)
        {
            #define D_PER_RAD (180.0f / 3.1415926f)
            yaw = vehicle_data->yaw * D_PER_RAD;
            roll = vehicle_data->roll * D_PER_RAD;
            pitch = vehicle_data->pitch * D_PER_RAD;
            depth = vehicle_data->alt / 1000.0f;
            time_us = vehicle_data->monotonic_time;
        }
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
    if(0 != AS::as_api_check_vehicle(currentVehicle))
    {
        AS::Vehicle_Data_t *vehicle_data;
        vehicle_data = new AS::Vehicle_Data_t;

        vehicle_data = AS::as_api_get_vehicle_data(currentVehicle);

        if (nullptr != vehicle_data)
        {
            #define D_PER_RAD (180.0f / 3.1415926f)
            yaw = vehicle_data->yaw * D_PER_RAD;
            roll = vehicle_data->roll * D_PER_RAD;
            pitch = vehicle_data->pitch * D_PER_RAD;
            depth = vehicle_data->alt / 1000.0f;
        }
    }

    ui->qADI->setData(roll, pitch);
    ui->qCompass->setYaw(yaw);
    ui->qCompass->setAlt(depth);
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
        adiCompassTimer.start();
        chartTimer.start();
        namedValueTimer.start();
    }
    else
    {
        statusTexTimer.stop();
        adiCompassTimer.stop();
        chartTimer.stop();
        namedValueTimer.stop();

        vehicleComboBox->setCurrentIndex(0);
    }
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
        value_float = new AS::mavlink_named_value_float_t;

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

void MainWindow::on_actionDive_triggered()
{
    ui->mainWindowsVerticalLayout->setStretch(0, 5000);
    ui->textVehicleInfo->verticalScrollBar()->setValue(
                ui->textVehicleInfo->verticalScrollBar()->maximum());
    ui->textLogInfo->verticalScrollBar()->setValue(
                ui->textLogInfo->verticalScrollBar()->maximum());

    ui->stackedWidget->setCurrentIndex(0);

    ui->vedio->setGeometry(0, 0 , ui->stackedWidgetVideo->width(), ui->stackedWidgetVideo->height());
    ui->qCompass->setGeometry(ui->stackedWidgetVideo->width() - 160, 0, 160, 160);
    ui->qADI->setGeometry(ui->stackedWidgetVideo->width() - 320, 0, 160, 160);
}

void MainWindow::on_actionAnalyze_triggered()
{
    ui->mainWindowsVerticalLayout->setStretch(0, 4);
    ui->textVehicleInfo->verticalScrollBar()->setValue(
                ui->textVehicleInfo->verticalScrollBar()->maximum());
    ui->textLogInfo->verticalScrollBar()->setValue(
                ui->textLogInfo->verticalScrollBar()->maximum());

    ui->stackedWidget->setCurrentIndex(1);

    setChartsSize();
}

void MainWindow::on_actionSetings_triggered()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->mainWindowsVerticalLayout->setStretch(0, 5000);
    ui->textVehicleInfo->verticalScrollBar()->setValue(
                ui->textVehicleInfo->verticalScrollBar()->maximum());
    ui->textLogInfo->verticalScrollBar()->setValue(
                ui->textLogInfo->verticalScrollBar()->maximum());
}


void MainWindow::on_armCheckBox_stateChanged(int state)
{
    switch (state)
    {
    case Qt::Checked:
        ui->actionDisarm->setDisabled(false);
        ui->depthPidCheckBox->setEnabled(true);
        ui->depthHoldCheckBox->setEnabled(true);

        AS::as_api_vehicle_arm(currentVehicle, 1);
        break;

    case Qt::Unchecked:
        ui->actionDisarm->setDisabled(true);
        ui->depthPidCheckBox->setCheckState(Qt::Unchecked);
        ui->depthPidCheckBox->setEnabled(false);
        ui->depthHoldCheckBox->setCheckState(Qt::Unchecked);
        ui->depthHoldCheckBox->setEnabled(false);

        AS::as_api_vehicle_disarm(currentVehicle, 1);
        break;

    default:
        ;
    }

}

void MainWindow::on_modeComboBox_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        AS::as_api_set_mode(currentVehicle, AS::MANUAL);
        break;

    case 1:
        AS::as_api_set_mode(currentVehicle, AS::STABILIZE);
        break;

    case 2:
        AS::as_api_set_mode(currentVehicle, AS::ACRO);
        break;

    case 3:
        AS::as_api_set_mode(currentVehicle, AS::ALT_HOLD);
        break;

    case 4:
        AS::as_api_set_mode(currentVehicle, AS::POSHOLD);
        break;

    case 5:
        AS::as_api_set_mode(currentVehicle, AS::LAB_REMOTE);
        break;

    case 6:
        AS::as_api_set_mode(currentVehicle, AS::LAB_ALT_HOLD);
        break;
    }
}

void MainWindow::on_vehicleComboBox_currentIndexChanged(const QString &index)
{
    qDebug() << index.toInt();
    currentVehicle = static_cast<uint8_t>(index.toInt());
}


void MainWindow::on_depthPidCheckBox_stateChanged(int arg1)
{
    if(armCheckBox->checkState() && modeComboBox->currentIndex() == 5)
    {
        if(arg1)
        {
            depthPidTimer.start();
        }
        else
        {
            depthPidTimer.stop();
            // clear I term
        }
    }
    else
    {
        ui->depthPidCheckBox->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::on_depthHoldCheckBox_stateChanged(int arg1)
{
    if(armCheckBox->checkState() && modeComboBox->currentIndex() == 6)
    {
        if(arg1)
        {
            modeComboBox->setCurrentIndex(6);
            AS::as_api_depth_pid(currentVehicle, 0,
                                 static_cast<float>(ui->doubleSpinBoxKp_2->value()),
                                 static_cast<float>(ui->doubleSpinBoxKi_2->value()),
                                 static_cast<float>(ui->doubleSpinBoxKd_2->value()),
                                 0, 0, 0);
            std::string info("depth_hold");
            std::string note("depth_hold");
            AS::as_api_test_start(info.c_str(), note.c_str());
            AS::as_api_depth_hold(currentVehicle, 1, static_cast<float>(ui->doubleSpinBoxZd_2->value()));
        }
        else
        {
            AS::as_api_depth_hold(currentVehicle, 0, 0);
            AS::as_api_test_stop();
            modeComboBox->setCurrentIndex(0);
        }
    }
    else
    {
        ui->depthPidCheckBox->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::on_actionDisarm_triggered()
{
    armCheckBox->setCheckState(Qt::Unchecked);
    ui->depthPidCheckBox->setDisabled(true);
    ui->depthPidCheckBox->setCheckState(Qt::Unchecked);
}

void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    switch (arg1)
    {
    case 0:
        ui->mainWindowsVerticalLayout->setStretch(0, 5000);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
                    ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
                    ui->textLogInfo->verticalScrollBar()->maximum());
        break;

    case 1:
        ui->mainWindowsVerticalLayout->setStretch(0, 4);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
                    ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
                    ui->textLogInfo->verticalScrollBar()->maximum());
        break;

    case 2:
        ui->mainWindowsVerticalLayout->setStretch(0, 5000);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
                    ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
                    ui->textLogInfo->verticalScrollBar()->maximum());
        break;

    default:
        break;
    }
}
