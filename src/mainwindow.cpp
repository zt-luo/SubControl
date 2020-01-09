#include "inc/mainwindow.h"
#include "ui_mainwindow.h"

#include <QChartView>
#include <QDebug>

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      cvRunner(nullptr),
      ui(new Ui::MainWindow),
      videoWindow(new VideoWindow(this)),
      m_yawRollChart(new YawRollChart),
      m_yawRollScene(new QGraphicsScene),
      m_pitchChart(new PitchChart),
      m_pitchScene(new QGraphicsScene),
      m_depthChart(new DepthChart),
      m_depthScene(new QGraphicsScene),
      joystickManager(nullptr),
      m_joystick(nullptr),
      armMessageBox(new QMessageBox(this)),
      currentVehicle(0),
      videoOk(false),
      videoReceiver(new VideoReceiver(this, true)),
      vehicle_data(new AS::Vehicle_Data_t)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/assets/icon/main_icon.svg"));
    setupToolBars();
    countScreens();

    QCoreApplication::setOrganizationName("ARMs of HUST");
    QCoreApplication::setOrganizationDomain("https://github.com/hust-arms");
    QCoreApplication::setApplicationName("SubControl");

    armMessageBox->setWindowTitle("Arm vehicle");
    armMessageBox->setText("Do you want to ARM vehicle?");
    armMessageBox->setIcon(QMessageBox::Question);
    armMessageBox->addButton(QMessageBox::No);
    armMessageBox->addButton(QMessageBox::Yes);
    armMessageBox->setDefaultButton(QMessageBox::Yes);

    manual_control.x = 0;
    manual_control.y = 0;
    manual_control.z = 500;
    manual_control.r = 0;
    manual_control.buttons = 0;

    pressedKey.W = false;
    pressedKey.S = false;
    pressedKey.A = false;
    pressedKey.D = false;
    pressedKey.Up = false;
    pressedKey.Down = false;
    pressedKey.Left = false;
    pressedKey.Down = false;

    keyControlValue.forward = 100;
    keyControlValue.backward = -100;
    keyControlValue.leftward = 100;
    keyControlValue.rightward = -100;
    keyControlValue.upward = 600;
    keyControlValue.downward = 400;
    keyControlValue.turnLeft = 100;
    keyControlValue.turnRight = -100;

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

    readSettings();

    //    std::string ip("192.168.2.");
    std::string ip("192.168.1.");
    // std::string ip("serial port");
    AS::as_api_init(ip.c_str(), F_THREAD_NONE);

    setupTimer();
    setupVideo();
    setupJoystick();
    setupConfigView();

    this->setMouseTracking(true);
    ui->centralWidget->setMouseTracking(true);
    ui->quickWidget->setMouseTracking(true);

    connect(videoWindow, &VideoWindow::closeWindows, this, &MainWindow::on_closeVideoWindow_triggered);
    connect(this, &MainWindow::updateVehicleDataSignal,
            videoWindow, &VideoWindow::on_updateVehicleDataSignal);
}

MainWindow::~MainWindow()
{
    videoWindow->close();
    writeSettings();

    delete ui;
    delete videoWindow;
    delete m_yawRollChart;
    delete m_yawRollScene;
    delete m_pitchChart;
    delete m_pitchScene;
    delete m_depthChart;
    delete m_depthScene;
    delete videoReceiver;
}

void MainWindow::resizeWindowsManual()
{
    int m_width = 0;
    m_width = ui->stackedWidgetVideo->width();

    setChartsSize();

    ui->quickWidget->setGeometry(0, 0, m_width, ui->stackedWidgetVideo->height());
    ui->picture->setGeometry(0, 0, m_width, ui->stackedWidgetVideo->height());

    ui->qCompass->setGeometry(m_width - 160, 0, 160, 160);
    ui->qADI->setGeometry(m_width - 320, 0, 160, 160);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    resizeWindowsManual();

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
    vehicleLabel = new QLabel;
    vehicleLabel->setText("Vehicle: ");
    ui->vehicleToolBar->addWidget(vehicleLabel);

    vehicleComboBox = new QComboBox;
    vehicleComboBox->addItem("/");
    QModelIndex index = vehicleComboBox->model()->index(0, 0);
    QVariant v(0);
    vehicleComboBox->model()->setData(index, v, Qt::UserRole - 1);
    vehicleComboBox->setFixedWidth(40);
    ui->vehicleToolBar->addWidget(vehicleComboBox);
    QObject::connect(vehicleComboBox,
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

    // status toolbar
    yawLabel = new QLabel("Yaw: ");
    yawLabelValue = new QLabel("0.00");
    pitchLabel = new QLabel("Pitch: ");
    pitchLabelValue = new QLabel("0.00");
    rollLabel = new QLabel("Roll: ");
    rollLabelValue = new QLabel("0.00");
    depthLabel = new QLabel("Depth: ");
    depthLabelValue = new QLabel("0.00");

    yawLabelValue->setFixedWidth(50);
    pitchLabelValue->setFixedWidth(50);
    rollLabelValue->setFixedWidth(50);
    depthLabelValue->setFixedWidth(50);

    ui->statusToolBar->addWidget(yawLabel);
    ui->statusToolBar->addWidget(yawLabelValue);
    ui->statusToolBar->addWidget(pitchLabel);
    ui->statusToolBar->addWidget(pitchLabelValue);
    ui->statusToolBar->addWidget(rollLabel);
    ui->statusToolBar->addWidget(rollLabelValue);
    ui->statusToolBar->addWidget(depthLabel);
    ui->statusToolBar->addWidget(depthLabelValue);
}

void MainWindow::stringToHtml(QString &str, QColor _color)
{
    // html filter
    str.replace("&", "&amp;");
    str.replace(">", "&gt;");
    str.replace("<", "&lt;");
    str.replace("\"", "&quot;");
    str.replace("\'", "&#39;");
    str.replace(" ", "&nbsp;");
    str.replace("\n", "<br>");
    str.replace("\r", "<br>");

    QByteArray array;
    array.append(static_cast<char>(_color.red()));
    array.append(static_cast<char>(_color.green()));
    array.append(static_cast<char>(_color.blue()));
    QString strC(array.toHex());

    str = QString("<span style=\" color:#%1;\">%2</span>").arg(strC).arg(str);
}

void MainWindow::setupConfigView()
{
    QFont *m_font = new QFont();
    m_font->setBold(true);
    m_font->setPointSize(12);

    QListWidgetItem *buttonGeneral = new QListWidgetItem(ui->listWidget);
    QIcon iconGeneral = QIcon(":/assets/icon/setting.svg");
    buttonGeneral->setIcon(iconGeneral);
    buttonGeneral->setText(" General");
    buttonGeneral->setFont(*m_font);
    ui->listWidget->addItem(buttonGeneral);

    QListWidgetItem *buttonJoystick = new QListWidgetItem(ui->listWidget);
    QIcon iconJoystick = QIcon(":/assets/icon/joystick_black.svg");
    buttonJoystick->setIcon(iconJoystick);
    buttonJoystick->setText(" Joystick");
    buttonJoystick->setFont(*m_font);
    ui->listWidget->addItem(buttonJoystick);

    QListWidgetItem *buttonThruster = new QListWidgetItem(ui->listWidget);
    QIcon iconThruster = QIcon(":/assets/icon/thruster.png");
    buttonThruster->setIcon(iconThruster);
    buttonThruster->setText(" Thruster");
    buttonThruster->setFont(*m_font);
    ui->listWidget->addItem(buttonThruster);

    ui->listWidget->setCurrentRow(0);

    // switch to video view
    ui->stackedWidgetMain->setCurrentIndex(1);
    ui->stackedWidgetMain->setCurrentIndex(0);
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    settings.beginGroup("General/Video");
    settings.setValue("enable", ui->checkBoxVideoLink->checkState());
    settings.setValue("port", ui->lineEditVideoLink->text());
    settings.setValue("ADI", ui->checkBoxADI->checkState());
    settings.setValue("compass", ui->checkBoxCompass->checkState());
    settings.endGroup();

    settings.beginGroup("General/Others");
    settings.setValue("keyboard_control", ui->checkBoxKeyboardControl->checkState());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(1200, 700)).toSize());
    move(settings.value("pos", QPoint(0, 0)).toPoint());
    settings.endGroup();

    settings.beginGroup("General/Video");
    ui->checkBoxVideoLink->setCheckState(
        static_cast<Qt::CheckState>(settings.value("enable", 2).toInt()));
    ui->lineEditVideoLink->setText(
        settings.value("port", "5600").toString());
    ui->checkBoxADI->setCheckState(
        static_cast<Qt::CheckState>(settings.value("ADI", 2).toInt()));
    ui->checkBoxCompass->setCheckState(
        static_cast<Qt::CheckState>(settings.value("compass", 2).toInt()));
    settings.endGroup();

    settings.beginGroup("General/Others");
    ui->checkBoxKeyboardControl->setCheckState(
        static_cast<Qt::CheckState>(settings.value("keyboard_control", 0).toInt()));
    settings.endGroup();
}

void MainWindow::on_actionVideo_triggered()
{
    ui->mainWindowsVerticalLayout->setStretch(0, 5000);
    ui->textVehicleInfo->verticalScrollBar()->setValue(
        ui->textVehicleInfo->verticalScrollBar()->maximum());
    ui->textLogInfo->verticalScrollBar()->setValue(
        ui->textLogInfo->verticalScrollBar()->maximum());

    ui->stackedWidgetMain->setCurrentIndex(0);

    int m_width = ui->stackedWidgetVideo->width();
    int m_height = ui->stackedWidgetVideo->height();
    ui->quickWidget->setGeometry(0, 0, m_width, m_height);
    ui->picture->setGeometry(0, 0, m_width, m_height);

    ui->qCompass->setGeometry(m_width - 160, 0, 160, 160);
    ui->qADI->setGeometry(m_width - 320, 0, 160, 160);
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

    QObject::disconnect(armCheckBox, &QCheckBox::stateChanged,
                        this, &MainWindow::on_armCheckBox_stateChanged);

    if (state == Qt::Checked)
    {
        QMessageBox::StandardButton ret = (QMessageBox::StandardButton)armMessageBox->exec();
        if (ret == QMessageBox::Yes)
        {
            armCheckBox->setCheckState(Qt::Checked);
            ui->actionDisarm->setDisabled(false);
            ui->upperCloseControl->setEnabled(true);

            AS::as_api_vehicle_arm(currentVehicle, 1);
        }
    }
    else if (state == Qt::Unchecked)
    {
        ui->actionDisarm->setDisabled(true);
        ui->upperCloseControl->setCheckState(Qt::Unchecked);
        ui->upperCloseControl->setEnabled(false);
        ui->checkBoxThrustersTest->setCheckState(Qt::Unchecked);

        // set manual control value to zero
        manual_control.x = 0;
        manual_control.y = 0;
        manual_control.z = 500;
        manual_control.r = 0;
        manual_control.buttons = 0;

        AS::as_api_vehicle_disarm(currentVehicle, 1);
    }

    QObject::connect(armCheckBox, &QCheckBox::stateChanged,
                     this, &MainWindow::on_armCheckBox_stateChanged);
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
    // qDebug() << index.toInt();
    currentVehicle = static_cast<uint8_t>(index.toInt());
}

void MainWindow::on_depthHoldCheckBox_stateChanged(int arg1)
{
    if (0 == currentVehicle)
    {
        return;
    }

    if (armCheckBox->checkState() && modeComboBox->currentIndex() == 6)
    {
        if (arg1)
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

        ui->actionVideo->setDisabled(true);
        ui->actionControl->setDisabled(false);
        ui->actionSetings->setDisabled(false);

        ui->actionVideo->setChecked(true);
        ui->actionControl->setChecked(false);
        ui->actionSetings->setChecked(false);

        break;

    case 1:
        ui->mainWindowsVerticalLayout->setStretch(0, 4);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
            ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
            ui->textLogInfo->verticalScrollBar()->maximum());

        // only enable actionVideo when it is checked
        if (ui->actionVideo->isChecked())
        {
            ui->actionVideo->setDisabled(false);
        }
        ui->actionControl->setDisabled(true);
        ui->actionSetings->setDisabled(false);

        ui->actionVideo->setChecked(false);
        ui->actionControl->setChecked(true);
        ui->actionSetings->setChecked(false);

        break;

    case 2:
        ui->mainWindowsVerticalLayout->setStretch(0, 4);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
            ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
            ui->textLogInfo->verticalScrollBar()->maximum());

        if (ui->actionVideo->isChecked())
        {
            ui->actionVideo->setDisabled(false);
        }
        ui->actionControl->setDisabled(true);
        ui->actionSetings->setDisabled(false);

        ui->actionVideo->setChecked(false);
        ui->actionControl->setChecked(true);
        ui->actionSetings->setChecked(false);

        break;

    case 3:
        ui->mainWindowsVerticalLayout->setStretch(0, 5000);
        ui->textVehicleInfo->verticalScrollBar()->setValue(
            ui->textVehicleInfo->verticalScrollBar()->maximum());
        ui->textLogInfo->verticalScrollBar()->setValue(
            ui->textLogInfo->verticalScrollBar()->maximum());

        if (ui->actionVideo->isChecked())
        {
            ui->actionVideo->setDisabled(false);
        }
        ui->actionControl->setDisabled(false);
        ui->actionSetings->setDisabled(true);

        ui->actionVideo->setChecked(false);
        ui->actionControl->setChecked(false);
        ui->actionSetings->setChecked(true);

        break;

    default:
        break;
    }
}

void MainWindow::on_connectedGamepadsChanged()
{
    // qDebug() << "on_connectedGamepadsChanged";

    if (joystickManager == nullptr)
    {
        return;
    }

    QList<int> joysticks = joystickManager->connectedGamepads();

    if (joysticks.isEmpty() || ui->checkBoxKeyboardControl->checkState() == Qt::Checked)
    {
        if (m_joystick != nullptr)
        {
            connectJoystickSlots(false, m_joystick);

            QIcon icon = QIcon(":/assets/icon/joystick_red.svg");
            ui->actionJoystick->setIcon(icon);

            delete m_joystick;
            m_joystick = nullptr;
        }
    }
    else
    {
        QIcon icon = QIcon(":/assets/icon/joystick_black.svg");
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

    default:;
    }
}

void MainWindow::on_verticalSliderPWM_1_valueChanged(int value)
{
    ui->pwmBox_1->setValue(value);
    on_pwmBox_1_editingFinished();
}

void MainWindow::on_clearPwmButton_1_pressed()
{
    ui->verticalSliderPWM_1->setValue(0);
    ui->pwmBox_1->setValue(0);
}

void MainWindow::on_pwmBox_1_editingFinished()
{
    ui->verticalSliderPWM_1->blockSignals(true);
    ui->verticalSliderPWM_1->setValue(ui->pwmBox_1->value());
    ui->verticalSliderPWM_1->blockSignals(false);
    // set pwm here
    pwmOutput[0] = static_cast<uint16_t>(1500 + ui->verticalSliderPWM_1->value());
    //    qDebug() << "finished:" << ui->pwmBox_1->value();
}

void MainWindow::on_verticalSliderPWM_2_valueChanged(int value)
{
    ui->pwmBox_2->setValue(value);
    on_pwmBox_2_editingFinished();
}

void MainWindow::on_clearPwmButton_2_pressed()
{
    ui->verticalSliderPWM_2->setValue(0);
    ui->pwmBox_2->setValue(0);
}

void MainWindow::on_pwmBox_2_editingFinished()
{
    ui->verticalSliderPWM_2->blockSignals(true);
    ui->verticalSliderPWM_2->setValue(ui->pwmBox_2->value());
    ui->verticalSliderPWM_2->blockSignals(false);
    // set pwm here
    pwmOutput[1] = static_cast<uint16_t>(1500 + ui->verticalSliderPWM_2->value());
    //    qDebug() << "finished:" << ui->pwmBox_2->value();
}

void MainWindow::on_verticalSliderPWM_3_valueChanged(int value)
{
    ui->pwmBox_3->setValue(value);
    on_pwmBox_3_editingFinished();
}

void MainWindow::on_clearPwmButton_3_pressed()
{
    ui->verticalSliderPWM_3->setValue(0);
    ui->pwmBox_3->setValue(0);
}

void MainWindow::on_pwmBox_3_editingFinished()
{
    ui->verticalSliderPWM_3->blockSignals(true);
    ui->verticalSliderPWM_3->setValue(ui->pwmBox_3->value());
    ui->verticalSliderPWM_3->blockSignals(false);
    // set pwm here
    pwmOutput[2] = static_cast<uint16_t>(1500 + ui->verticalSliderPWM_3->value());
    //    qDebug() << "finished:" << ui->pwmBox_3->value();
}

void MainWindow::on_verticalSliderPWM_4_valueChanged(int value)
{
    ui->pwmBox_4->setValue(value);
    on_pwmBox_4_editingFinished();
}

void MainWindow::on_clearPwmButton_4_pressed()
{
    ui->verticalSliderPWM_4->setValue(0);
    ui->pwmBox_4->setValue(0);
}

void MainWindow::on_pwmBox_4_editingFinished()
{
    ui->verticalSliderPWM_4->blockSignals(true);
    ui->verticalSliderPWM_4->setValue(ui->pwmBox_4->value());
    ui->verticalSliderPWM_4->blockSignals(false);
    // set pwm here
    pwmOutput[3] = static_cast<uint16_t>(1500 + ui->verticalSliderPWM_4->value());
    //    qDebug() << "finished:" << ui->pwmBox_4->value();
}

void MainWindow::on_verticalSliderPWM_5_valueChanged(int value)
{
    ui->pwmBox_5->setValue(value);
    on_pwmBox_5_editingFinished();
}

void MainWindow::on_clearPwmButton_5_pressed()
{
    ui->verticalSliderPWM_5->setValue(0);
    ui->pwmBox_5->setValue(0);
}

void MainWindow::on_pwmBox_5_editingFinished()
{
    ui->verticalSliderPWM_5->blockSignals(true);
    ui->verticalSliderPWM_5->setValue(ui->pwmBox_5->value());
    ui->verticalSliderPWM_5->blockSignals(false);
    // set pwm here
    pwmOutput[4] = static_cast<uint16_t>(1500 + ui->verticalSliderPWM_5->value());
    //    qDebug() << "finished:" << ui->pwmBox_5->value();
}

void MainWindow::on_verticalSliderPWM_6_valueChanged(int value)
{
    ui->pwmBox_6->setValue(value);
    on_pwmBox_6_editingFinished();
}

void MainWindow::on_clearPwmButton_6_pressed()
{
    ui->verticalSliderPWM_6->setValue(0);
    ui->pwmBox_6->setValue(0);
}

void MainWindow::on_pwmBox_6_editingFinished()
{
    ui->verticalSliderPWM_6->blockSignals(true);
    ui->verticalSliderPWM_6->setValue(ui->pwmBox_6->value());
    ui->verticalSliderPWM_6->blockSignals(false);
    // set pwm here
    pwmOutput[5] = static_cast<uint16_t>(1500 + ui->verticalSliderPWM_6->value());
    //    qDebug() << "finished:" << ui->pwmBox_6->value();
}

void MainWindow::on_verticalSliderPWM_7_valueChanged(int value)
{
    ui->pwmBox_7->setValue(value);
    on_pwmBox_7_editingFinished();
}

void MainWindow::on_clearPwmButton_7_pressed()
{
    ui->verticalSliderPWM_7->setValue(0);
    ui->pwmBox_7->setValue(0);
}

void MainWindow::on_pwmBox_7_editingFinished()
{
    ui->verticalSliderPWM_7->blockSignals(true);
    ui->verticalSliderPWM_7->setValue(ui->pwmBox_7->value());
    ui->verticalSliderPWM_7->blockSignals(false);
    // set pwm here
    pwmOutput[6] = static_cast<uint16_t>(1500 + ui->verticalSliderPWM_7->value());
    //    qDebug() << "finished:" << ui->pwmBox_7->value();
}

void MainWindow::on_verticalSliderPWM_8_valueChanged(int value)
{
    ui->pwmBox_8->setValue(value);
    //    emit ui->pwmBox_8->editingFinished();
    on_pwmBox_8_editingFinished();
}

void MainWindow::on_clearPwmButton_8_pressed()
{
    ui->verticalSliderPWM_8->setValue(0);
    ui->pwmBox_8->setValue(0);
}

void MainWindow::on_pwmBox_8_editingFinished()
{
    ui->verticalSliderPWM_8->blockSignals(true);
    ui->verticalSliderPWM_8->setValue(ui->pwmBox_8->value());
    ui->verticalSliderPWM_8->blockSignals(false);
    // set pwm here
    pwmOutput[7] = static_cast<uint16_t>(1500 + ui->verticalSliderPWM_8->value());
    //    qDebug() << "finished:" << ui->pwmBox_8->value();
}

void MainWindow::on_checkBoxThrustersTest_stateChanged(int state)
{
    switch (state)
    {
    case Qt::Checked:

        if (armCheckBox->checkState() == Qt::Unchecked)
        {
            ui->checkBoxThrustersTest->setCheckState(Qt::Unchecked);
            QMessageBox::critical(this, "Arm vehicle first",
                                  "Vehicle is disarmed, please ARM vehicle first.",
                                  QMessageBox::Ok, QMessageBox::Ok);
            break;
        }

        ui->verticalSliderPWM_1->setEnabled(true);
        ui->pwmBox_1->setEnabled(true);
        ui->clearPwmButton_1->setEnabled(true);

        ui->verticalSliderPWM_2->setEnabled(true);
        ui->pwmBox_2->setEnabled(true);
        ui->clearPwmButton_2->setEnabled(true);

        ui->verticalSliderPWM_3->setEnabled(true);
        ui->pwmBox_3->setEnabled(true);
        ui->clearPwmButton_3->setEnabled(true);

        ui->verticalSliderPWM_4->setEnabled(true);
        ui->pwmBox_4->setEnabled(true);
        ui->clearPwmButton_4->setEnabled(true);

        ui->verticalSliderPWM_5->setEnabled(true);
        ui->pwmBox_5->setEnabled(true);
        ui->clearPwmButton_5->setEnabled(true);

        ui->verticalSliderPWM_6->setEnabled(true);
        ui->pwmBox_6->setEnabled(true);
        ui->clearPwmButton_6->setEnabled(true);

        ui->verticalSliderPWM_7->setEnabled(true);
        ui->pwmBox_7->setEnabled(true);
        ui->clearPwmButton_7->setEnabled(true);

        ui->verticalSliderPWM_8->setEnabled(true);
        ui->pwmBox_8->setEnabled(true);
        ui->clearPwmButton_8->setEnabled(true);

        thrustersTestTimer.start();

        break;

    case Qt::Unchecked:

        emit ui->clearPwmButton_1->pressed();
        ui->verticalSliderPWM_1->setEnabled(false);
        ui->pwmBox_1->setEnabled(false);
        ui->clearPwmButton_1->setEnabled(false);

        emit ui->clearPwmButton_2->pressed();
        ui->verticalSliderPWM_2->setEnabled(false);
        ui->pwmBox_2->setEnabled(false);
        ui->clearPwmButton_2->setEnabled(false);

        emit ui->clearPwmButton_3->pressed();
        ui->verticalSliderPWM_3->setEnabled(false);
        ui->pwmBox_3->setEnabled(false);
        ui->clearPwmButton_3->setEnabled(false);

        emit ui->clearPwmButton_4->pressed();
        ui->verticalSliderPWM_4->setEnabled(false);
        ui->pwmBox_4->setEnabled(false);
        ui->clearPwmButton_4->setEnabled(false);

        emit ui->clearPwmButton_5->pressed();
        ui->verticalSliderPWM_5->setEnabled(false);
        ui->pwmBox_5->setEnabled(false);
        ui->clearPwmButton_5->setEnabled(false);

        emit ui->clearPwmButton_6->pressed();
        ui->verticalSliderPWM_6->setEnabled(false);
        ui->pwmBox_6->setEnabled(false);
        ui->clearPwmButton_6->setEnabled(false);

        emit ui->clearPwmButton_7->pressed();
        ui->verticalSliderPWM_7->setEnabled(false);
        ui->pwmBox_7->setEnabled(false);
        ui->clearPwmButton_7->setEnabled(false);

        emit ui->clearPwmButton_8->pressed();
        ui->verticalSliderPWM_8->setEnabled(false);
        ui->pwmBox_8->setEnabled(false);
        ui->clearPwmButton_8->setEnabled(false);

        pwmOutput[0] = 1500;
        pwmOutput[1] = 1500;
        pwmOutput[2] = 1500;
        pwmOutput[3] = 1500;
        pwmOutput[4] = 1500;
        pwmOutput[5] = 1500;
        pwmOutput[6] = 1500;
        pwmOutput[7] = 1500;

        thrustersTestTimer.stop();

        AS::as_api_send_rc_channels_override(currentVehicle, 1,
                                             pwmOutput[0],
                                             pwmOutput[1],
                                             pwmOutput[2],
                                             pwmOutput[3],
                                             pwmOutput[4],
                                             pwmOutput[5],
                                             pwmOutput[6],
                                             pwmOutput[7]);

        AS::as_api_vehicle_disarm(currentVehicle, 1);

        break;

    default:
        break;
    }
}

void MainWindow::on_checkBoxADI_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->qADI->show();
    }
    else
    {
        ui->qADI->hide();
    }
}

void MainWindow::on_checkBoxCompass_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->qCompass->show();
    }
    else
    {
        ui->qCompass->hide();
    }
}

void MainWindow::on_checkBoxVideoLink_stateChanged(int arg1)
{
    if (arg1)
    {
        videoReceiver->play();
        ui->quickWidget->show();
    }
    else
    {
        videoReceiver->pause();
        ui->quickWidget->hide();
    }
}

void MainWindow::on_checkBoxKeyboardControl_stateChanged(int arg1)
{
    on_connectedGamepadsChanged();
}

void MainWindow::on_upperControlButton_clicked()
{
    ui->stackedWidgetMain->setCurrentIndex(1);
}

void MainWindow::on_actionAdvanceMode_triggered(bool arg1)
{
    if (arg1)
    {
        QList<QScreen *> m_screens = QGuiApplication::screens();
        if (m_screens.count() < 2)
        {
            return;
        }

        QScreen *screen = m_screens[0];
        setGeometry(screen->availableGeometry());
        setWindowState(Qt::WindowMaximized);

        screen = m_screens[1];
        videoWindow->setGeometry(screen->availableGeometry());
        videoWindow->show();
        videoWindow->setWindowState(Qt::WindowMaximized);

        emit ui->actionControl->trigger();

        ui->actionVideo->setDisabled(true);
    }
    else
    {
        videoWindow->close();
    }
}

void MainWindow::on_closeVideoWindow_triggered()
{
    ui->actionAdvanceMode->setChecked(false);

    emit ui->actionVideo->trigger();

    ui->actionVideo->setDisabled(false);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton ret =
        QMessageBox::question(this,
                              "Exit",
                              "Do you want to close SubControl?",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Yes);
    if (ret == QMessageBox::Yes)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // QQuickWidget can not response to mouse move event when is not foremost
    // qDebug() << "mouse move";
    videoReceiver->setRecordingHightlight(enterRecordingButton(event->pos()));
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) &&
        enterRecordingButton(event->pos()))
    {
        if (!videoReceiver->getRecording())
        {
            videoReceiver->startRecording();
        }
        else
        {
            videoReceiver->stopRecording();
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    {
        return;
    }

    if (ui->checkBoxKeyboardControl->checkState() == Qt::Unchecked)
    {
        return;
    }

    if (ui->stackedWidgetMain->currentIndex() != 0)
    {
        return;
    }

    if (event->key() == Qt::Key_W)
    {
        qDebug() << "You Pressed Key W";
        pressedKey.W = true;
        manual_control.x = keyControlValue.forward;
    }
    else if (event->key() == Qt::Key_S)
    {
        qDebug() << "You Pressed Key S";
        pressedKey.S = true;
        manual_control.x = keyControlValue.backward;
    }
    else if (event->key() == Qt::Key_A)
    {
        qDebug() << "You Pressed Key A";
        pressedKey.A = true;
        manual_control.y = keyControlValue.leftward;
    }
    else if (event->key() == Qt::Key_D)
    {
        qDebug() << "You Pressed Key D";
        pressedKey.D = true;
        manual_control.y = keyControlValue.rightward;
    }
    else if (event->key() == Qt::Key_Up)
    {
        qDebug() << "You Pressed Key Up";
        pressedKey.Up = true;
        manual_control.z = keyControlValue.upward;
    }
    else if (event->key() == Qt::Key_Down)
    {
        qDebug() << "You Pressed Key Down";
        pressedKey.Down = true;
        manual_control.z = keyControlValue.downward;
    }
    else if (event->key() == Qt::Key_Left)
    {
        qDebug() << "You Pressed Key Left";
        pressedKey.Left = true;
        manual_control.r = keyControlValue.turnLeft;
    }
    else if (event->key() == Qt::Key_Right)
    {
        qDebug() << "You Pressed Key Right";
        pressedKey.Right = true;
        manual_control.r = keyControlValue.turnRight;
    }
    else
    {
        qDebug() << "You Pressed NOT supported Key";
    }

    qDebug() << "x: " << manual_control.x;
    qDebug() << "y: " << manual_control.y;
    qDebug() << "z: " << manual_control.z;
    qDebug() << "r: " << manual_control.r;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    {
        return;
    }

    if (ui->checkBoxKeyboardControl->checkState() == Qt::Unchecked)
    {
        return;
    }

    if (ui->stackedWidgetMain->currentIndex() != 0)
    {
        return;
    }

    if (event->key() == Qt::Key_W)
    {
        qDebug() << "You Released Key W";
        if (pressedKey.S)
        {
            manual_control.x = keyControlValue.backward;
        }
        else
        {
            manual_control.x = 0;
        }
        pressedKey.W = false;
    }
    else if (event->key() == Qt::Key_S)
    {
        qDebug() << "You Released Key S";
        if (pressedKey.W)
        {
            manual_control.x = keyControlValue.forward;
        }
        else
        {
            manual_control.x = 0;
        }
        pressedKey.S = false;
    }
    else if (event->key() == Qt::Key_A)
    {
        qDebug() << "You Released Key A";
        if (pressedKey.D)
        {
            manual_control.y = keyControlValue.rightward;
        }
        else
        {
            manual_control.y = 0;
        }
        pressedKey.A = false;
    }
    else if (event->key() == Qt::Key_D)
    {
        qDebug() << "You Released Key D";
        if (pressedKey.A)
        {
            manual_control.y = keyControlValue.leftward;
        }
        else
        {
            manual_control.y = 0;
        }
        pressedKey.D = false;
    }
    else if (event->key() == Qt::Key_Up)
    {
        qDebug() << "You Released Key Up";
        if (pressedKey.Down)
        {
            manual_control.z = keyControlValue.downward;
        }
        else
        {
            manual_control.z = 500;
        }
        pressedKey.Up = false;
    }
    else if (event->key() == Qt::Key_Down)
    {
        qDebug() << "You Released Key Down";
        if (pressedKey.Up)
        {
            manual_control.z = keyControlValue.upward;
        }
        else
        {
            manual_control.z = 500;
        }
        pressedKey.Down = false;
    }
    else if (event->key() == Qt::Key_Left)
    {
        qDebug() << "You Released Key Left";
        if (pressedKey.Right)
        {
            manual_control.r = keyControlValue.turnRight;
        }
        else
        {
            manual_control.r = 0;
        }
        pressedKey.Left = false;
    }
    else if (event->key() == Qt::Key_Right)
    {
        qDebug() << "You Released Key Right";
        if (pressedKey.Left)
        {
            manual_control.r = keyControlValue.turnRight;
        }
        else
        {
            manual_control.r = 0;
        }
        pressedKey.Right = false;
    }
    else
    {
        qDebug() << "You Released NOT supported Key";
    }

    qDebug() << "x: " << manual_control.x;
    qDebug() << "y: " << manual_control.y;
    qDebug() << "z: " << manual_control.z;
    qDebug() << "r: " << manual_control.r;
}

bool MainWindow::enterRecordingButton(QPoint postion)
{
    QPoint buttonCenter =
        QPoint(this->width() - 20 - 18,
               this->height() - ui->textLogInfo->height() - 20 - 18);

    return (postion.x() - buttonCenter.x()) * (postion.x() - buttonCenter.x()) +
               (postion.y() - buttonCenter.y()) * (postion.y() - buttonCenter.y()) <
           18 * 18;
}

void MainWindow::on_pushButtonStartCV_clicked()
{
    emit videoReceiver->playCvProcess();
}

void MainWindow::on_pushButtonStopCV_clicked()
{
    emit videoReceiver->pauseCvProcess();
}
