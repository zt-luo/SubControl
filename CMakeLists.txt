cmake_minimum_required(VERSION 3.10)

# your project name
project("SubControl")

add_compile_options(-g -O0 
                    # -Wno-unused-parameter
                    -static)
                    # -shared)

# find includes in the corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# run moc automatically
# set(CMAKE_AUTOMOC ON)
# run uic automatically
# set(CMAKE_AUTOUIC ON)
# run rcc automatically
# set (CMAKE_AUTORCC ON)

message("Looking for Qt...")
# Qt modules (https://doc.qt.io/qt-5/qtmodules.html) you're using in your application
find_package(Qt5 REQUIRED Widgets Core Gui Charts Gamepad QuickWidgets Qml)
if (${Qt5_FOUND})
    message("Found Qt " ${Qt5_VERSION})
else()
    message("Couldn't find Qt")
endif()

link_libraries(
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Charts
    Qt5::Gamepad
    Qt5::QuickWidgets
    Qt5::Qml
    )

qt5_wrap_ui(ui_list 
    "ui/mainwindow.ui"
    "ui/videowindow.ui"
    )

qt5_wrap_cpp(moc_list
    "inc/chart.h"
    "inc/mainwindow.h"
    "inc/qFlightInstruments.h"
    "inc/videowindow.h"
    "inc/VideoReceiver.h"
)

qt5_add_resources(qrc_list 
    "assets.qrc")

# check and search moudles
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(LIBSERIALPORT REQUIRED libserialport)
pkg_search_module(GST REQUIRED gstreamer-1.0)
pkg_search_module(GST_VIDEO REQUIRED gstreamer-video-1.0)

include_directories(
    ${GLIB_INCLUDE_DIRS} ${LIBSERIALPORT_INCLUDE_DIRS} 
    ${GST_INCLUDE_DIRS} ${GST_VIDEO_INCLUDE_DIRS} 
    )

link_libraries(
    ${GLIB_LDFLAGS} ${LIBSERIALPORT_LDFLAGS} 
    ${GST_LDFLAGS} ${GST_VIDEO_LDFLAGS} 
    )

ADD_SUBDIRECTORY(ardusub_api)
link_libraries(ardusub_static)
# link_libraries(ardusub_shared)

# add include dir
include_directories("ardusub_api/mavlink_c_library_v2")
include_directories("inc")

# your source files
set(src_list
    "src/chart.cpp"
    "src/control.cpp"
    "src/joystick.cpp"
    "src/main.cpp"
    "src/mainwindow.cpp"
    "src/qFlightInstruments.cpp"
    "src/timer.cpp"
    "src/video.cpp"
    "src/videowindow.cpp"
    "src/VideoReceiver.cpp"
)

# name of the .exe file, window flag and the list of things to compile
# add_executable(${CMAKE_PROJECT_NAME} WIN32 ${sources})
add_executable(${CMAKE_PROJECT_NAME} ${src_list} ${ui_list} ${moc_list} ${qrc_list})
