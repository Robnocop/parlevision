TEMPLATE = lib
TARGET = mskinect_plugin
CONFIG += plugin
QT += core gui
DESTDIR = ../../libs/plugins
INCLUDEPATH += ../../include/plvopencv
LIBS += -lplvcore -lplvgui
QMAKE_LIBDIR += ../../libs/plugins

#MS_KINECT_SDK_PATH = "C:/Program Files (x86)/Microsoft Research KinectSDK"
#MS_KINECT_SDK_PATH = "C:/Program Files/Microsoft SDKs/Kinect/v1.0"
MS_KINECT_SDK_PATH = D:/ddevelop/mrKinectSDK

include (../../common.pri)

macx {
    error( "This plugin is not supported on Mac OSX because it uses \
            the Microsoft Windows 7 Kinect SDK." )
}

unix {
    error( "This plugin is not supported on Unix type operating systems \
            because it uses the Microsoft Windows 7 Kinect SDK." )
}

INCLUDEPATH += $${MS_KINECT_SDK_PATH}/inc
QMAKE_LIBDIR += $${MS_KINECT_SDK_PATH}/lib/x86
#LIBS += -lMSRKinectNUI
LIBS += -lKinect10

DEFINES += MSKINECT_PLUGIN_LIBRARY

SOURCES += mskinectplugin.cpp \
           mskinectproducer.cpp \
           mskinectdevice.cpp \
           skeletondataviewer.cpp \
           mskinectdatatypes.cpp \
		   mskinectfakecolor.cpp \
    kinectthreshold.cpp

HEADERS +=  mskinectplugin_global.h \
            mskinectplugin.h \
            mskinectproducer.h \
            mskinectdevice.h \
            skeletondataviewer.h \
            mskinectdatatypes.h \
			mskinectfakecolor.h \
    kinectthreshold.h
