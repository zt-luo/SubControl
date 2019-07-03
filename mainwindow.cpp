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
    m_joystick(nullptr),
    currentVehicle(0)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":icon/icon/main_icon.svg"));
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
    setupJoystick();
    setupConfigView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    static bool firstResize = true;
    if (!firstResize)
    {
        setChartsSize();
        ui->vedio->setGeometry(0, 0 , ui->stackedWidgetVideo->width(), ui->stackedWidgetVideo->height());

        ui->qCompass->setGeometry(ui->stackedWidgetVideo->width() - 160, 0, 160, 160);
        ui->qADI->setGeometry(ui->stackedWidgetVideo->width() - 320, 0, 160, 160);
    }
    else
    {
        firstResize = false;
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
    QObject::connect (vehicleComboBox,
                      static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
                      this, &MainWindow::on_vehicleComboBox_currentIndexChanged);

    QList<QAction *> actionListDisarm;
    actionListDisarm.append(ui->actionDisarm);
    ui->vehicleToolBar->addActions(actionListDisarm);

    armCheckBox = new QCheckBox;
    armCheckBox->setText("Arm");
    ui->vehicleToolBar->addWidget(armCheckBox);
    QObject::connect(armCheckBox, &QCheckBox::stateChanged,
                     this, &MainWindow::on_armCheckBox_stateChanged);

    ui->vehicleToolBar->addSeparator();

    QList<QAction *> actionListJoystick;
    actionListJoystick.append(ui->actionJoystick);
    ui->vehicleToolBar->addActions(actionListJoystick);
//    ui->actionJoystick->setDisabled(true);

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
    armCheckBox->setDisabled(true);
    modeComboBox->setDisabled(true);
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
    QObject::connect(&closeControlTimer, &QTimer::timeout, this, &MainWindow::closeControl);
    closeControlTimer.setInterval(25);
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
    vehicleCheckTimer.setInterval(500);
    vehicleCheckTimer.start();

    QObject::connect(&manualControlTimer, &QTimer::timeout, this, &MainWindow::manualControl);
    manualControlTimer.setInterval(50);
    manualControlTimer.start();
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

void MainWindow::setupJoystick()
{
    joystickManager = QGamepadManager::instance();

    QObject::connect(joystickManager,
                     &QGamepadManager::connectedGamepadsChanged,
                     this,
                     &MainWindow::on_connectedGamepadsChanged);

    QList<int> joysticks = joystickManager->connectedGamepads();

    if (!joysticks.isEmpty())
    {
        QIcon icon = QIcon(":/icon/icon/joystick_black.svg");
        ui->actionJoystick->setIcon(icon);

        m_joystick = new QGamepad(*joysticks.begin(), this);

        qDebug() << m_joystick->name() << "deviceId:" << m_joystick->deviceId();

        connectJoystickSlots(true, m_joystick);
    }
}

void MainWindow::connectJoystickSlots(bool b, QGamepad* m_joystick)
{
    if (b)
    {
        QObject::connect(m_joystick, &QGamepad::axisLeftXChanged, this,
                         &MainWindow::on_joystick_axisLeftXChanged);

        QObject::connect(m_joystick, &QGamepad::axisLeftYChanged, this,
                         &MainWindow::on_joystick_axisLeftYChanged);

        QObject::connect(m_joystick, &QGamepad::axisRightXChanged, this,
                         &MainWindow::on_joystick_axisRightXChanged);

        QObject::connect(m_joystick, &QGamepad::axisRightYChanged, this,
                         &MainWindow::on_joystick_axisRightYChanged);

        connect(m_joystick, &QGamepad::buttonAChanged, this, [](bool pressed){
            qDebug() << "Button A" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonBChanged, this, [](bool pressed){
            qDebug() << "Button B" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonXChanged, this, [](bool pressed){
            qDebug() << "Button X" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonYChanged, this, [](bool pressed){
            qDebug() << "Button Y" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonL1Changed, this, [](bool pressed){
            qDebug() << "Button L1" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonR1Changed, this, [](bool pressed){
            qDebug() << "Button R1" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonL2Changed, this, [](double value){
            qDebug() << "Button L2: " << value;
        });
        connect(m_joystick, &QGamepad::buttonR2Changed, this, [](double value){
            qDebug() << "Button R2: " << value;
        });
        connect(m_joystick, &QGamepad::buttonSelectChanged, this, [](bool pressed){
            qDebug() << "Button Select" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonStartChanged, this, [](bool pressed){
            qDebug() << "Button Start" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonGuideChanged, this, [](bool pressed){
            qDebug() << "Button Guide" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonUpChanged, this, [](bool pressed){
            qDebug() << "Button Up" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonDownChanged, this, [](bool pressed){
            qDebug() << "Button Down" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonLeftChanged, this, [](bool pressed){
            qDebug() << "Button Left" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonRightChanged, this, [](bool pressed){
            qDebug() << "Button Right" << pressed;
        });
        connect(m_joystick, &QGamepad::buttonCenterChanged, this, [](bool pressed){
            qDebug() << "Button Center" << pressed;
        });
    }
    else
    {
        m_joystick->disconnect();
    }
}

void MainWindow::setupConfigView()
{
    QFont *m_font = new QFont();
    m_font->setBold(true);
    m_font->setPointSize(12);

    QListWidgetItem *buttonGeneral = new QListWidgetItem(ui->listWidget);
    QIcon iconGeneral = QIcon(":/icon/icon/setting.svg");
    buttonGeneral->setIcon(iconGeneral);
    buttonGeneral->setText(" General");
    buttonGeneral->setFont(*m_font);
    ui->listWidget->addItem(buttonGeneral);

    QListWidgetItem *buttonJoystick = new QListWidgetItem(ui->listWidget);
    QIcon iconJoystick = QIcon(":/icon/icon/joystick_black.svg");
    buttonJoystick->setIcon(iconJoystick);
    buttonJoystick->setText(" Joystick");
    buttonJoystick->setFont(*m_font);
    ui->listWidget->addItem(buttonJoystick);

    ui->listWidget->setCurrentRow(0);

    // switch to vedio view
    ui->stackedWidgetMain->setCurrentIndex(1);
    ui->stackedWidgetMain->setCurrentIndex(0);
}

void MainWindow::closeControl()
{
//    AS::as_api_set_mode(1, AS::LAB_REMOTE);
    // check current mode
    if (modeComboBox->currentText() != "LAB_REMOTE")
    {
        return;
    }

    // controlor output
    int depth_pwm_out = 0,
            yaw_pwm_out = 0, pitch_pwm_out = 0, roll_pwm_out = 0,
            x_pwm_out  = 0, y_pwm_out = 0;

    static double depth_now;   // depth in this iteration, m
    static double yaw_now;     // yaw in this iteration, degree
    static double pitch_now;   // pitch in this iteration, degree
    static double roll_now;    // roll in this iteration, degree

    static double depth_old;   // depth in the previous iteration
    static double yaw_old;     // yaw in the previous iteration
    static double pitch_old;   // pitch in the previous iteration
    static double roll_old;    // roll in the previous iteration

    depth_old = depth_now;
    yaw_old = yaw_now;
    pitch_old = pitch_now;
    roll_old = roll_now;

    if(0 != AS::as_api_check_vehicle(currentVehicle))
    {
        AS::Vehicle_Data_t *vehicle_data;
        vehicle_data = AS::as_api_get_vehicle_data(currentVehicle);
        if (nullptr != vehicle_data)
        {
            const float degreePerRad = 180.0f / 3.1415926f;
            depth_now = vehicle_data->alt / 1000.0;          // m
            yaw_now = vehicle_data->yaw * degreePerRad;      // degree
            pitch_now = vehicle_data->pitch * degreePerRad;  // degree
            roll_now = vehicle_data->roll * degreePerRad;    // degree
        }
    }

    // depth controlor start
    if (!ui->depthPidCheckBox->checkState())
    {
        depth_pwm_out = 0;
    }
    else
    {
        double depth_expected, kP, kI, kD;

        depth_expected = ui->doubleSpinBoxDe->value();

        kP = ui->doubleSpinBoxKpDepth->value();
        kI = ui->doubleSpinBoxKiDepth->value();
        kD = ui->doubleSpinBoxKdDepth->value();

        int pwm_limit = 150;

        uint32_t dt = 25; // ms

        // I_term_min and max prevent integral windup by saturating the I_term
        double I_term_max = -600;
        double I_term_min = -700;
        // [m/s] this is added to prevent setpoint kick in the derivative term
        double depth_vel_lim = 1;

        double P_term = 0;
        double I_term = 0;
        double D_term = 0;

        double depth_err = 0; // declare the depth error
        double depth_vel = 0; // declare the derivative term for z

        depth_err = depth_expected - depth_now;

        // P_term
        P_term = kP * depth_err;

        // I_term
        if (abs(kI) < 0.0000001)
        {
            I_term = 0;
        }
        else
        {
            if (depth_expected < -0.2) // dont do this while disarm
            {
                I_term = I_term + kI * depth_err * (dt / 1000);
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

        // D_term
        depth_vel = (depth_now - depth_old) / (dt / 1000);  //PI-D
//        depth_vel = depth_err / (dt / 1000);  //PID
        // clamp the value of z_vel
        if (depth_vel < -depth_vel_lim)
        {
            depth_vel = -depth_vel_lim;
        }
        if (depth_vel > depth_vel_lim)
        {
            depth_vel = depth_vel_lim;
        }
        D_term = kD * depth_vel;

        depth_pwm_out = static_cast<int>(P_term + I_term + D_term);

        // clamp the value of depth_pwm_out
        if (depth_pwm_out < -pwm_limit)
        {
            depth_pwm_out = -pwm_limit;
        }
        if (depth_pwm_out > pwm_limit)
        {
            depth_pwm_out = pwm_limit;
        }

    } // depth controlor end

    // yaw controlor start
    if (!ui->yawPidCheckBox->checkState())
    {
        yaw_pwm_out = 0;
    }
    else
    {
        double yaw_expected, kP, kI, kD;

        yaw_expected = ui->doubleSpinBoxYe->value();

        kP = ui->doubleSpinBoxKpYaw->value();
        kI = ui->doubleSpinBoxKiYaw->value();
        kD = ui->doubleSpinBoxKdYaw->value();

        int pwm_limit = 150;

        uint32_t dt = 25; // ms

        // I_term_min and max prevent integral windup by saturating the I_term
        double I_term_max = -600;
        double I_term_min = -700;
        // [degree/s] this is added to prevent setpoint kick in the derivative term
        double yaw_vel_lim = 1;

        double P_term = 0;
        double I_term = 0;
        double D_term = 0;

        double yaw_err = 0; // declare the depth error
        double yaw_vel = 0; // declare the derivative term for z

        yaw_err = yaw_expected - yaw_now;

        // P_term
        P_term = kP * yaw_err;

        // I_term
        if (abs(kI) < 0.0000001)
        {
            I_term = 0;
        }
        else
        {
            if (yaw_expected < -0.2) // dont do this while disarm
            {
                I_term = I_term + kI * yaw_err * (dt / 1000);
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

        // D_term
        yaw_vel = (yaw_now - yaw_old) / (dt / 1000);  //PI-D
//        yaw_vel = yaw_err / (dt / 1000);  //PID
        // clamp the value of z_vel
        if (yaw_vel < -yaw_vel_lim)
        {
            yaw_vel = -yaw_vel_lim;
        }
        if (yaw_vel > yaw_vel_lim)
        {
            yaw_vel = yaw_vel_lim;
        }
        D_term = kD * yaw_vel;

        yaw_pwm_out = static_cast<int>(P_term + I_term + D_term);

        // clamp the value of yaw_pwm_out
        if (yaw_pwm_out < -pwm_limit)
        {
            yaw_pwm_out = -pwm_limit;
        }
        if (yaw_pwm_out > pwm_limit)
        {
            yaw_pwm_out = pwm_limit;
        }
    } // yaw controlor end

    // pitch controlor start
    if (!ui->pitchPidCheckBox->checkState())
    {
        pitch_pwm_out = 0;
    }
    else
    {
        double pitch_expected, kP, kI, kD;

        pitch_expected = ui->doubleSpinBoxPe->value();

        kP = ui->doubleSpinBoxKpPitch->value();
        kI = ui->doubleSpinBoxKiPitch->value();
        kD = ui->doubleSpinBoxKdPitch->value();

        int pwm_limit = 150;

        uint32_t dt = 25; // ms

        // I_term_min and max prevent integral windup by saturating the I_term
        double I_term_max = -600;
        double I_term_min = -700;
        // [degree/s] this is added to prevent setpoint kick in the derivative term
        double pitch_vel_lim = 1;

        double P_term = 0;
        double I_term = 0;
        double D_term = 0;

        double pitch_err = 0; // declare the depth error
        double pitch_vel = 0; // declare the derivative term for z

        pitch_err = pitch_expected - pitch_now;

        // P_term
        P_term = kP * pitch_err;

        // I_term
        if (abs(kI) < 0.0000001)
        {
            I_term = 0;
        }
        else
        {
            if (pitch_expected < -0.2) // dont do this while disarm
            {
                I_term = I_term + kI * pitch_err * (dt / 1000);
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

        // D_term
        pitch_vel = (pitch_now - pitch_old) / (dt / 1000);  //PI-D
//        pitch_vel = pitch_err / (dt / 1000);  //PID
        // clamp the value of z_vel
        if (pitch_vel < -pitch_vel_lim)
        {
            pitch_vel = -pitch_vel_lim;
        }
        if (pitch_vel > pitch_vel_lim)
        {
            pitch_vel = pitch_vel_lim;
        }
        D_term = kD * pitch_vel;

        pitch_pwm_out = static_cast<int>(P_term + I_term + D_term);

        // clamp the value of yaw_pwm_out
        if (pitch_pwm_out < -pwm_limit)
        {
            pitch_pwm_out = -pwm_limit;
        }
        if (pitch_pwm_out > pwm_limit)
        {
            pitch_pwm_out = pwm_limit;
        }
    } // pitch controlor end

    // roll controlor start
    if (!ui->rollPidCheckBox->checkState())
    {
        roll_pwm_out = 0;
    }
    else
    {
        double roll_expected, kP, kI, kD;

        roll_expected = ui->doubleSpinBoxRe->value();

        kP = ui->doubleSpinBoxKpRoll->value();
        kI = ui->doubleSpinBoxKiRoll->value();
        kD = ui->doubleSpinBoxKdRoll->value();

        int pwm_limit = 150;

        uint32_t dt = 25; // ms

        // I_term_min and max prevent integral windup by saturating the I_term
        double I_term_max = -600;
        double I_term_min = -700;
        // [degree/s] this is added to prevent setpoint kick in the derivative term
        double roll_vel_lim = 1;

        double P_term = 0;
        double I_term = 0;
        double D_term = 0;

        double roll_err = 0; // declare the depth error
        double roll_vel = 0; // declare the derivative term for z

        roll_err = roll_expected - roll_now;

        // P_term
        P_term = kP * roll_err;

        // I_term
        if (abs(kI) < 0.0000001)
        {
            I_term = 0;
        }
        else
        {
            if (roll_expected < -0.2) // dont do this while disarm
            {
                I_term = I_term + kI * roll_err * (dt / 1000);
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

        // D_term
        roll_vel = (roll_now - roll_old) / (dt / 1000);  //PI-D
//        roll_vel = roll_err / (dt / 1000);  //PID
        // clamp the value of z_vel
        if (roll_vel < -roll_vel_lim)
        {
            roll_vel = -roll_vel_lim;
        }
        if (roll_vel > roll_vel_lim)
        {
            roll_vel = roll_vel_lim;
        }
        D_term = kD * roll_vel;

        roll_pwm_out = static_cast<int>(P_term + I_term + D_term);

        // clamp the value of yaw_pwm_out
        if (roll_pwm_out < -pwm_limit)
        {
            roll_pwm_out = -pwm_limit;
        }
        if (roll_pwm_out > pwm_limit)
        {
            roll_pwm_out = pwm_limit;
        }
    } // roll controlor end

    // x movement start
    {
        int pwm_limit = 150;

        int x_input = 0;

        x_pwm_out = x_input * 150;

        // clamp the value of x_pwm_out
        if (x_pwm_out < -pwm_limit)
        {
            x_pwm_out = -pwm_limit;
        }
        if (x_pwm_out > pwm_limit)
        {
            x_pwm_out = pwm_limit;
        }
    } // x movement end

    // y movement start
    {
        int pwm_limit = 150;

        int y_input = 0;

        x_pwm_out = y_input * 150;

        // clamp the value of y_pwm_out
        if (y_pwm_out < -pwm_limit)
        {
            y_pwm_out = -pwm_limit;
        }
        if (y_pwm_out > pwm_limit)
        {
            y_pwm_out = pwm_limit;
        }
    } // y movement end

    int motor_pwm[8] = {1500};

    motor_pwm[0] = 1500 - yaw_pwm_out + x_pwm_out - y_pwm_out;
    motor_pwm[1] = 1500 + yaw_pwm_out + x_pwm_out + y_pwm_out;
    motor_pwm[2] = 1500 - yaw_pwm_out + x_pwm_out + y_pwm_out;
    motor_pwm[3] = 1500 + yaw_pwm_out + x_pwm_out - y_pwm_out;

    motor_pwm[4] = 1500 + depth_pwm_out + pitch_pwm_out + roll_pwm_out;
    motor_pwm[5] = 1500 - depth_pwm_out - pitch_pwm_out + roll_pwm_out;
    motor_pwm[6] = 1500 - depth_pwm_out + pitch_pwm_out - roll_pwm_out;
    motor_pwm[7] = 1500 + depth_pwm_out - pitch_pwm_out - roll_pwm_out;

    AS::as_api_send_rc_channels_override(
                currentVehicle, 1,
                static_cast<uint16_t>(motor_pwm[0]),
                static_cast<uint16_t>(motor_pwm[1]),
                static_cast<uint16_t>(motor_pwm[2]),
                static_cast<uint16_t>(motor_pwm[3]),
                static_cast<uint16_t>(motor_pwm[4]),
                static_cast<uint16_t>(motor_pwm[5]),
                static_cast<uint16_t>(motor_pwm[6]),
                static_cast<uint16_t>(motor_pwm[7]));


//    QString out_str;
//    ui->textLogInfo->append(out_str.sprintf("%d", depth_pwm_out));
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
            const float degreePerRad = 180.0f / 3.1415926f;

            yaw = vehicle_data->yaw * degreePerRad;
            roll = vehicle_data->roll * degreePerRad;
            pitch = vehicle_data->pitch * degreePerRad;
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

        armCheckBox->setEnabled(true);
        modeComboBox->setEnabled(true);
    }
    else
    {
        currentVehicle = 0;

        statusTexTimer.stop();
        adiCompassTimer.stop();
        chartTimer.stop();
        namedValueTimer.stop();

        vehicleComboBox->setCurrentIndex(0);

        armCheckBox->setDisabled(true);
        modeComboBox->setDisabled(true);
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

void MainWindow::manualControl()
{
    if (currentVehicle != 0 &&
            armCheckBox->checkState() == Qt::Checked &&
            modeComboBox->currentText() == "MANUAL" &&
            AS::as_api_check_vehicle(currentVehicle))
    {
        AS::as_api_manual_control(manual_control.x, manual_control.y,
                                  manual_control.z, manual_control.r,
                                  manual_control.buttons);
    }
}

void MainWindow::on_actionVideo_triggered()
{
    ui->mainWindowsVerticalLayout->setStretch(0, 5000);
    ui->textVehicleInfo->verticalScrollBar()->setValue(
                ui->textVehicleInfo->verticalScrollBar()->maximum());
    ui->textLogInfo->verticalScrollBar()->setValue(
                ui->textLogInfo->verticalScrollBar()->maximum());

    ui->stackedWidgetMain->setCurrentIndex(0);

    ui->vedio->setGeometry(0, 0 , ui->stackedWidgetVideo->width(), ui->stackedWidgetVideo->height());
    ui->qCompass->setGeometry(ui->stackedWidgetVideo->width() - 160, 0, 160, 160);
    ui->qADI->setGeometry(ui->stackedWidgetVideo->width() - 320, 0, 160, 160);
}

void MainWindow::on_actionControl_triggered()
{
    ui->mainWindowsVerticalLayout->setStretch(0, 4);
    ui->textVehicleInfo->verticalScrollBar()->setValue(
                ui->textVehicleInfo->verticalScrollBar()->maximum());
    ui->textLogInfo->verticalScrollBar()->setValue(
                ui->textLogInfo->verticalScrollBar()->maximum());

    ui->stackedWidgetMain->setCurrentIndex(1);

    setChartsSize();
}

void MainWindow::on_actionSetings_triggered()
{
    ui->stackedWidgetMain->setCurrentIndex(3);
    ui->mainWindowsVerticalLayout->setStretch(0, 5000);
    ui->textVehicleInfo->verticalScrollBar()->setValue(
                ui->textVehicleInfo->verticalScrollBar()->maximum());
    ui->textLogInfo->verticalScrollBar()->setValue(
                ui->textLogInfo->verticalScrollBar()->maximum());
}


void MainWindow::on_armCheckBox_stateChanged(int state)
{
    if (0 == currentVehicle)
    {
        return;
    }

    switch (state)
    {
    case Qt::Checked:
        ui->actionDisarm->setDisabled(false);
        ui->upperCloseControl->setEnabled(true);

        AS::as_api_vehicle_arm(currentVehicle, 1);
        break;

    case Qt::Unchecked:
        ui->actionDisarm->setDisabled(true);
        ui->upperCloseControl->setCheckState(Qt::Unchecked);
        ui->upperCloseControl->setEnabled(false);

        AS::as_api_vehicle_disarm(currentVehicle, 1);
        break;

    default:
        ;
    }

}

void MainWindow::on_modeComboBox_currentIndexChanged(int index)
{
    if (0 == currentVehicle)
    {
        modeComboBox->setCurrentIndex(0);
        return;
    }

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
//    qDebug() << index.toInt();
    currentVehicle = static_cast<uint8_t>(index.toInt());
}

void MainWindow::on_depthHoldCheckBox_stateChanged(int arg1)
{
    if (0 == currentVehicle)
    {
        return;
    }

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

void MainWindow::on_stackedWidgetMain_currentChanged(int arg1)
{
    switch (arg1)
    {
    case 0:
        ui->mainWindowsVerticalLayout->setStretch(0, 5000);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
                    ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
                    ui->textLogInfo->verticalScrollBar()->maximum());

        ui->actionVideo->setChecked(true);
        ui->actionControl->setChecked(false);
        ui->actionSetings->setChecked(false);

        ui->actionVideo->setDisabled(true);
        ui->actionControl->setDisabled(false);
        ui->actionSetings->setDisabled(false);

        break;

    case 1:
        ui->mainWindowsVerticalLayout->setStretch(0, 4);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
                    ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
                    ui->textLogInfo->verticalScrollBar()->maximum());

        ui->actionVideo->setChecked(false);
        ui->actionControl->setChecked(true);
        ui->actionSetings->setChecked(false);

        ui->actionVideo->setDisabled(false);
        ui->actionControl->setDisabled(true);
        ui->actionSetings->setDisabled(false);

        break;

    case 2:
        ui->mainWindowsVerticalLayout->setStretch(0, 4);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
                    ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
                    ui->textLogInfo->verticalScrollBar()->maximum());

        ui->actionVideo->setChecked(false);
        ui->actionControl->setChecked(true);
        ui->actionSetings->setChecked(false);

        ui->actionVideo->setDisabled(false);
        ui->actionControl->setDisabled(true);
        ui->actionSetings->setDisabled(false);

        break;

    case 3:
        ui->mainWindowsVerticalLayout->setStretch(0, 5000);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
                    ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
                    ui->textLogInfo->verticalScrollBar()->maximum());

        ui->actionVideo->setChecked(false);
        ui->actionControl->setChecked(false);
        ui->actionSetings->setChecked(true);

        ui->actionVideo->setDisabled(false);
        ui->actionControl->setDisabled(false);
        ui->actionSetings->setDisabled(true);

        break;

    default:
        break;
    }
}


void MainWindow::on_connectedGamepadsChanged()
{
    qDebug() << "on_connectedGamepadsChanged";

    QList<int> joysticks = joystickManager->connectedGamepads();

    if (joysticks.isEmpty())
    {
        if (m_joystick != nullptr)
        {
            connectJoystickSlots(false, m_joystick);

            QIcon icon = QIcon(":/icon/icon/joystick_red.svg");
            ui->actionJoystick->setIcon(icon);

            delete m_joystick;
            m_joystick = nullptr;
        }
    }
    else
    {
        QIcon icon = QIcon(":/icon/icon/joystick_black.svg");
        ui->actionJoystick->setIcon(icon);

        m_joystick = new QGamepad(*joysticks.begin(), this);

        qDebug() << m_joystick->name() << "deviceId:" << m_joystick->deviceId();

        connectJoystickSlots(true, m_joystick);
    }
}

void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidgetConfig->setCurrentIndex(currentRow);
}

void MainWindow::on_actionJoystick_triggered()
{
    ui->stackedWidgetMain->setCurrentIndex(3);
    ui->listWidget->setCurrentRow(1);
}

void MainWindow::on_joystick_axisLeftXChanged(double value)
{
    if (ui->listWidget->currentRow() == 1 && ui->stackedWidgetMain->currentIndex() == 3)
    {
        ui->axisLeftXSlider->setValue(static_cast<int>(value * 50 + 50));
    }
    else
    {
//        qDebug() << "Left X" << value;

        manual_control.y = static_cast<int16_t>(value * 1000);
    }
}

void MainWindow::on_joystick_axisLeftYChanged(double value)
{
    if (ui->listWidget->currentRow() == 1 && ui->stackedWidgetMain->currentIndex() == 3)
    {
        ui->axisLeftYSlider->setValue(static_cast<int>(value * 50 + 50));
    }
    else
    {
//        qDebug() << "Left Y" << value;

        manual_control.x = static_cast<int16_t>(value * 1000);
    }
}

void MainWindow::on_joystick_axisRightXChanged(double value)
{
    if (ui->listWidget->currentRow() == 1 && ui->stackedWidgetMain->currentIndex() == 3)
    {
        ui->axisRightXSlider->setValue(static_cast<int>(value * 50 + 50));
    }
    else
    {
//        qDebug() << "Right X" << value;

        manual_control.r = static_cast<int16_t>(value * 1000);
    }
}

void MainWindow::on_joystick_axisRightYChanged(double value)
{
    if (ui->listWidget->currentRow() == 1 && ui->stackedWidgetMain->currentIndex() == 3)
    {
        ui->axisRightYSlider->setValue(static_cast<int>(value * 50 + 50));
    }
    else
    {
//        qDebug() << "Right Y" << value;

        manual_control.z = static_cast<int16_t>(value * 1000);
    }
}

void MainWindow::on_lowerControlButton_clicked()
{
    ui->stackedWidgetMain->setCurrentIndex(2);
}

void MainWindow::on_upperCloseControl_stateChanged(int state)
{
    // set LAB_REMOTE mode
    modeComboBox->setCurrentIndex(5);

    std::string info("");
    info = ui->lineEditTestInfo->text().toStdString();
    std::string note("");
    note = ui->lineEditTestNote->text().toStdString();

    switch (state)
    {
    case Qt::Checked:
        ui->depthPidCheckBox->setEnabled(true);
        ui->yawPidCheckBox->setEnabled(true);
        ui->pitchPidCheckBox->setEnabled(true);
        ui->rollPidCheckBox->setEnabled(true);

        closeControlTimer.start();
        AS::as_api_test_start(info.c_str(), note.c_str());

        break;

    case Qt::Unchecked:
        ui->depthPidCheckBox->setCheckState(Qt::Unchecked);
        ui->yawPidCheckBox->setCheckState(Qt::Unchecked);
        ui->pitchPidCheckBox->setCheckState(Qt::Unchecked);
        ui->rollPidCheckBox->setCheckState(Qt::Unchecked);

        ui->depthPidCheckBox->setEnabled(false);
        ui->yawPidCheckBox->setEnabled(false);
        ui->pitchPidCheckBox->setEnabled(false);
        ui->rollPidCheckBox->setEnabled(false);

        closeControlTimer.stop();
        AS::as_api_test_stop();

        break;

    default:
        ;
    }
}
