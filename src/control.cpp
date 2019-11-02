#include "inc/mainwindow.h"
#include "ui_mainwindow.h"

#include <QChartView>
#include <QDebug>

#include <iostream>


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

    uint32_t dt = 50; // ms

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

    const float degreePerRad = 180.0f / 3.1415926f;
    depth_now = vehicle_data->alt / 1000.0;          // m
    yaw_now = vehicle_data->yaw * degreePerRad;      // degree
    pitch_now = vehicle_data->pitch * degreePerRad;  // degree
    roll_now = vehicle_data->roll * degreePerRad;    // degree

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

        // I_term_min and max prevent integral windup by saturating the I_term
        double I_term_max = 100;
        double I_term_min = -100;
        // [m/s] this is added to prevent setpoint kick in the derivative term
        double depth_vel_lim = 1;

        double P_term = 0;
        static double I_term;
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
                I_term = I_term + kI * depth_err * (dt / 1000.0);
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
        depth_vel = (depth_now - depth_old) / (dt / 1000.0);  //PI-D
//        depth_vel = depth_err / (dt / 1000.0);  //PID
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

        QString out_str;
        ui->textLogInfo->append(out_str.sprintf("err:%f, P:%f, I:%f, D:%f, out:%d.",
                                                depth_err, P_term, I_term, D_term, depth_pwm_out));

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

        // I_term_min and max prevent integral windup by saturating the I_term
        double I_term_max = 100;
        double I_term_min = -100;
        // [degree/s] this is added to prevent setpoint kick in the derivative term
        double yaw_vel_lim = 2;

        double P_term = 0;
        static double I_term;
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
                I_term = I_term + kI * yaw_err * (dt / 1000.0);
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
        yaw_vel = (yaw_now - yaw_old) / (dt / 1000.0);  //PI-D
//        yaw_vel = yaw_err / (dt / 1000.0);  //PID
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

        QString out_str;
        ui->textLogInfo->append(out_str.sprintf("err:%f, P:%f, I:%f, D:%f, vel:%f, out:%d.",
                                                yaw_err, P_term, I_term, D_term, yaw_vel,yaw_pwm_out));
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

        // I_term_min and max prevent integral windup by saturating the I_term
        double I_term_max = 100;
        double I_term_min = -100;
        // [degree/s] this is added to prevent setpoint kick in the derivative term
        double pitch_vel_lim = 1;

        double P_term = 0;
        static double I_term;
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
                I_term = I_term + kI * pitch_err * (dt / 1000.0);
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
        pitch_vel = (pitch_now - pitch_old) / (dt / 1000.0);  //PI-D
//        pitch_vel = pitch_err / (dt / 1000.0);  //PID
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
    motor_pwm[2] = 1500 + yaw_pwm_out + x_pwm_out - y_pwm_out;
    motor_pwm[3] = 1500 - yaw_pwm_out + x_pwm_out + y_pwm_out;

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

        qDebug() << manual_control.x << manual_control.y << manual_control.z << manual_control.r;
    }
}


void MainWindow::thrustersTest()
{
    AS::as_api_send_rc_channels_override(currentVehicle, 1,
            pwmOutput[0],
            pwmOutput[1],
            pwmOutput[2],
            pwmOutput[3],
            pwmOutput[4],
            pwmOutput[5],
            pwmOutput[6],
            pwmOutput[7]);
}
