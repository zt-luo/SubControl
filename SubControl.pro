#-------------------------------------------------
#
# Project created by QtCreator 2019-05-10T21:29:03
#
#-------------------------------------------------

QT       += core gui charts gamepad

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets quickwidgets

TARGET = SubControl
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    src/chart.cpp \
    src/control.cpp \
    src/joystick.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/qFlightInstruments.cpp \
    src/timer.cpp \
    src/video.cpp \
    src/videowindow.cpp

HEADERS += \
    inc/chart.h \
    inc/mainwindow.h \
    inc/qFlightInstruments.h \
    inc/videowindow.h \
    inc/videowindow.hS

FORMS += \
        ui/mainwindow.ui \
        ui/videowindow.ui

CONFIG += link_pkgconfig

PKGCONFIG += glib-2.0 gthread-2.0 gio-2.0 sqlite3 libserialport libVLCQtCore libVLCQtWidgets

LIBS += $$PWD/ardusub_api/build/api/libardusub.a

INCLUDEPATH += \
    ./ardusub_api/mavlink_c_library_v2

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    assets.qrc
