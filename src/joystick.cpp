#include "inc/mainwindow.h"
#include "ui_mainwindow.h"

#include <QChartView>
#include <QDebug>

#include <iostream>

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
        QIcon icon = QIcon(":/assets/icon/joystick_black.svg");
        ui->actionJoystick->setIcon(icon);

        m_joystick = new QGamepad(*joysticks.begin(), this);

        qDebug() << m_joystick->name() << "deviceId:" << m_joystick->deviceId();

        connectJoystickSlots(true, m_joystick);
    }
}

void MainWindow::connectJoystickSlots(bool b, QGamepad *m_joystick)
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

        QObject::connect(m_joystick, &QGamepad::buttonAChanged, this,
                         &MainWindow::on_joystick_buttonAChanged);

        QObject::connect(m_joystick, &QGamepad::buttonBChanged, this,
                         &MainWindow::on_joystick_buttonBChanged);

        QObject::connect(m_joystick, &QGamepad::buttonXChanged, this,
                         &MainWindow::on_joystick_buttonXChanged);

        QObject::connect(m_joystick, &QGamepad::buttonYChanged, this,
                         &MainWindow::on_joystick_buttonYChanged);

        QObject::connect(m_joystick, &QGamepad::buttonL1Changed, this,
                         &MainWindow::on_joystick_buttonL1Changed);

        QObject::connect(m_joystick, &QGamepad::buttonR1Changed, this,
                         &MainWindow::on_joystick_buttonR1Changed);

        QObject::connect(m_joystick, &QGamepad::buttonL2Changed, this,
                         &MainWindow::on_joystick_buttonL2Changed);

        QObject::connect(m_joystick, &QGamepad::buttonR2Changed, this,
                         &MainWindow::on_joystick_buttonR2Changed);

        QObject::connect(m_joystick, &QGamepad::buttonL3Changed, this,
                         &MainWindow::on_joystick_buttonL3Changed);

        QObject::connect(m_joystick, &QGamepad::buttonR3Changed, this,
                         &MainWindow::on_joystick_buttonR3Changed);

        QObject::connect(m_joystick, &QGamepad::buttonSelectChanged, this,
                         &MainWindow::on_joystick_buttonSelectChanged);

        QObject::connect(m_joystick, &QGamepad::buttonStartChanged, this,
                         &MainWindow::on_joystick_buttonStartChanged);

        QObject::connect(m_joystick, &QGamepad::buttonUpChanged, this,
                         &MainWindow::on_joystick_buttonUpChanged);

        QObject::connect(m_joystick, &QGamepad::buttonDownChanged, this,
                         &MainWindow::on_joystick_buttonDownChanged);

        QObject::connect(m_joystick, &QGamepad::buttonLeftChanged, this,
                         &MainWindow::on_joystick_buttonLeftChanged);

        QObject::connect(m_joystick, &QGamepad::buttonRightChanged, this,
                         &MainWindow::on_joystick_buttonRightChanged);

        QObject::connect(m_joystick, &QGamepad::buttonGuideChanged, this, [](bool pressed) {
            qDebug() << "Button Guide" << pressed;
        });

        QObject::connect(m_joystick, &QGamepad::buttonCenterChanged, this, [](bool pressed) {
            qDebug() << "Button Center" << pressed;
        });
    }
    else
    {
        m_joystick->disconnect();
    }
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

        manual_control.y = static_cast<int16_t>(value * 500 - 1);
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

        manual_control.x = static_cast<int16_t>(value * 500 + 1);
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

        manual_control.r = static_cast<int16_t>(value * 500 - 1);
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

        manual_control.z = static_cast<int16_t>(value * 500 + 500 - 1);
    }
}

void MainWindow::on_joystick_buttonUpChanged(bool pressed)
{
    if (pressed)
    {
        ui->bUp->setEnabled(false);
        ui->bUp->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bUp->setEnabled(true);
        ui->bUp->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonDownChanged(bool pressed)
{
    if (pressed)
    {
        ui->bDown->setEnabled(false);
        ui->bDown->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bDown->setEnabled(true);
        ui->bDown->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonLeftChanged(bool pressed)
{
    if (pressed)
    {
        ui->bLeft->setEnabled(false);
        ui->bLeft->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bLeft->setEnabled(true);
        ui->bLeft->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonRightChanged(bool pressed)
{
    if (pressed)
    {
        ui->bRight->setEnabled(false);
        ui->bRight->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bRight->setEnabled(true);
        ui->bRight->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonXChanged(bool pressed)
{
    if (pressed)
    {
        ui->bX->setEnabled(false);
        ui->bX->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bX->setEnabled(true);
        ui->bX->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonYChanged(bool pressed)
{
    if (pressed)
    {
        ui->bY->setEnabled(false);
        ui->bY->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bY->setEnabled(true);
        ui->bY->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonAChanged(bool pressed)
{
    if (pressed)
    {
        ui->bA->setEnabled(false);
        ui->bA->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bA->setEnabled(true);
        ui->bA->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonBChanged(bool pressed)
{
    if (pressed)
    {
        ui->bB->setEnabled(false);
        ui->bB->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bB->setEnabled(true);
        ui->bB->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonL1Changed(bool pressed)
{
    if (pressed)
    {
        ui->bL1->setEnabled(false);
        ui->bL1->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bL1->setEnabled(true);
        ui->bL1->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonL2Changed(bool pressed)
{
    if (pressed)
    {
        ui->bL2->setEnabled(false);
        ui->bL2->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bL2->setEnabled(true);
        ui->bL2->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonL3Changed(bool pressed)
{
    if (pressed)
    {
        ui->bL3->setEnabled(false);
        ui->bL3->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bL3->setEnabled(true);
        ui->bL3->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonSelectChanged(bool pressed)
{
    if (pressed)
    {
        ui->bBack->setEnabled(false);
        ui->bBack->setStyleSheet("color: white; background-color: darkGray;");

        if (armMessageBox->isActiveWindow())
        {
            armMessageBox->done(QMessageBox::No);
        }
        else
        {
            armCheckBox->setChecked(false);
        }
    }
    else
    {
        ui->bBack->setEnabled(true);
        ui->bBack->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonR1Changed(bool pressed)
{
    if (pressed)
    {
        ui->bR1->setEnabled(false);
        ui->bR1->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bR1->setEnabled(true);
        ui->bR1->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonR2Changed(bool pressed)
{
    if (pressed)
    {
        ui->bR2->setEnabled(false);
        ui->bR2->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bR2->setEnabled(true);
        ui->bR2->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonR3Changed(bool pressed)
{
    if (pressed)
    {
        ui->bR3->setEnabled(false);
        ui->bR3->setStyleSheet("color: white; background-color: darkGray;");
    }
    else
    {
        ui->bR3->setEnabled(true);
        ui->bR3->setStyleSheet("");
    }
}

void MainWindow::on_joystick_buttonStartChanged(bool pressed)
{
    if (pressed)
    {
        ui->bStart->setEnabled(false);
        ui->bStart->setStyleSheet("color: white; background-color: darkGray;");

        if (armCheckBox->checkState() == Qt::Checked)
        {
            return;
        }

        if (armMessageBox->isActiveWindow())
        {
            armMessageBox->done(QMessageBox::Yes);
        }
        else
        {
            emit armCheckBox->stateChanged(2);
        }
    }
    else
    {
        ui->bStart->setEnabled(true);
        ui->bStart->setStyleSheet("");
    }
}
