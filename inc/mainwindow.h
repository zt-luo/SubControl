#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QGamepadManager>
#include <QtGamepad/QGamepad>
#include <QSettings>
#include <QQuickWidget>

#include <gst/gst.h>

#include "chart.h"
#include "videowindow.h"
#include "VideoReceiver.h"

namespace AS {
#include "./ardusub_api/api/inc/ardusub_api.h"
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setChartsSize();
    void resizeWindowsManual();

private slots:

    void on_actionVideo_triggered();

    void on_actionControl_triggered();

    void on_armCheckBox_stateChanged(int state);

    void on_modeComboBox_currentIndexChanged(int index);

    void on_vehicleComboBox_currentIndexChanged(const QString &index);

    void on_actionDisarm_triggered();

    void on_actionSetings_triggered();

    void on_depthHoldCheckBox_stateChanged(int arg1);

    void on_stackedWidgetMain_currentChanged(int arg1);

    void on_connectedGamepadsChanged();

    void on_listWidget_currentRowChanged(int currentRow);

    void on_actionJoystick_triggered();

    void on_lowerControlButton_clicked();

    void on_upperCloseControl_stateChanged(int state);

    // joystick
    void on_joystick_axisLeftXChanged(double value);
    void on_joystick_axisLeftYChanged(double value);
    void on_joystick_axisRightXChanged(double value);
    void on_joystick_axisRightYChanged(double value);
    void on_joystick_buttonUpChanged(bool pressed);
    void on_joystick_buttonDownChanged(bool pressed);
    void on_joystick_buttonLeftChanged(bool pressed);
    void on_joystick_buttonRightChanged(bool pressed);
    void on_joystick_buttonXChanged(bool pressed);
    void on_joystick_buttonYChanged(bool pressed);
    void on_joystick_buttonAChanged(bool pressed);
    void on_joystick_buttonBChanged(bool pressed);
    void on_joystick_buttonL1Changed(bool pressed);
    void on_joystick_buttonL2Changed(bool pressed);
    void on_joystick_buttonL3Changed(bool pressed);
    void on_joystick_buttonSelectChanged(bool pressed);
    void on_joystick_buttonR1Changed(bool pressed);
    void on_joystick_buttonR2Changed(bool pressed);
    void on_joystick_buttonR3Changed(bool pressed);
    void on_joystick_buttonStartChanged(bool pressed);

    void on_verticalSliderPWM_1_valueChanged(int value);
    void on_clearPwmButton_1_pressed();
    void on_pwmBox_1_editingFinished();

    void on_verticalSliderPWM_2_valueChanged(int value);
    void on_clearPwmButton_2_pressed();
    void on_pwmBox_2_editingFinished();

    void on_verticalSliderPWM_3_valueChanged(int value);
    void on_clearPwmButton_3_pressed();
    void on_pwmBox_3_editingFinished();

    void on_verticalSliderPWM_4_valueChanged(int value);
    void on_clearPwmButton_4_pressed();
    void on_pwmBox_4_editingFinished();

    void on_verticalSliderPWM_5_valueChanged(int value);
    void on_clearPwmButton_5_pressed();
    void on_pwmBox_5_editingFinished();

    void on_verticalSliderPWM_6_valueChanged(int value);
    void on_clearPwmButton_6_pressed();
    void on_pwmBox_6_editingFinished();

    void on_verticalSliderPWM_7_valueChanged(int value);
    void on_clearPwmButton_7_pressed();
    void on_pwmBox_7_editingFinished();

    void on_verticalSliderPWM_8_valueChanged(int value);
    void on_clearPwmButton_8_pressed();
    void on_pwmBox_8_editingFinished();

    void on_checkBoxThrustersTest_stateChanged(int state);

    void on_checkBoxADI_stateChanged(int arg1);

    void on_checkBoxCompass_stateChanged(int arg1);

    void on_checkBoxVideoLink_stateChanged(int arg1);

    void on_upperControlButton_clicked();

    void on_actionAdvanceMode_triggered(bool arg1);

    void on_closeVideoWindow_triggered();

    void on_pushButtonStartCV_clicked();

    void on_pushButtonStopCV_clicked();

private:
    Ui::MainWindow *ui;
    VideoWindow *videoWindow;

    YawRollChart *m_yawRollChart;
    QGraphicsScene *m_yawRollScene;

    PitchChart *m_pitchChart;
    QGraphicsScene *m_pitchScene;

    DepthChart *m_depthChart;
    QGraphicsScene *m_depthScene;

    QLabel *vehicleLabel;
    QComboBox *vehicleComboBox;
    QCheckBox *armCheckBox;
    QLabel *modeLable;
    QComboBox *modeComboBox;

    QLabel *yawLabel;
    QLabel *yawLabelValue;
    QLabel *pitchLabel;
    QLabel *pitchLabelValue;
    QLabel *rollLabel;
    QLabel *rollLabelValue;
    QLabel *depthLabel;
    QLabel *depthLabelValue;

    QGamepadManager *joystickManager;
    QGamepad *m_joystick;

    QTimer vehicleDataUpdateTimer;
    QTimer closeControlTimer;
    QTimer statusTexTimer;
    QTimer vehicleCheckTimer;
    QTimer namedValueTimer;
    QTimer manualControlTimer;
    QTimer thrustersTestTimer;
    QTimer countScreenTimer;

    typedef struct
    {
        int16_t x;
        int16_t y;
        int16_t z;
        int16_t r;
        uint16_t buttons;
    } manual_control_t;

    manual_control_t manual_control;

    uint8_t currentVehicle;
    bool videoOk;

    VideoReceiver *videoReceiver;

    AS::Vehicle_Data_t *vehicle_data;

    uint16_t pwmOutput[8] = {1500};

    QHash<QString, float> namedFloatHash;
    QHash<QString, int> activeVehicleHash;

    void resizeEvent(QResizeEvent* event);

    void setupToolBars();

    void stringToHtml(QString &str, QColor _color);

    void setupTimer();

    void setupVideo();

    void setupJoystick();
    void connectJoystickSlots(bool b, QGamepad* m_joystick);

    void setupConfigView();

    bool enterRecordingButton(QPoint postion);

    // Timer callback
    void closeControl();
    void fetchStatusTex();
    void updateChart();
    void updateAdiCompass();
    void vehicleCheck();
    void updateNamedValue();
    void manualControl();
    void thrustersTest();
    void countScreens();
    void updateVehicleData();

    void writeSettings();
    void readSettings();

    void closeEvent(QCloseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void updateVehicleDataSignal(AS::Vehicle_Data_t *vehicle_data);
};

#endif // MAINWINDOW_H
