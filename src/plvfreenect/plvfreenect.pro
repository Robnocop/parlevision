#-------------------------------------------------
#
# Project created by QtCreator 2010-07-22T14:39:10
#
#-------------------------------------------------
TEMPLATE = lib
TARGET = libfreenect_plugin
CONFIG += plugin
QT += core xml gui
#QT -= gui
DESTDIR = ../../libs/plugins
INCLUDEPATH += ../../include/plvopencv
LIBS += -lplvcore
QMAKE_LIBDIR += ../../libs/plugins

include (../../common.pri)

USB_PATH_GCC = C:/libfreenect/other/libusb-win32-bin-1.2.5.0
PTHREADS_PATH_GCC = C:/libfreenect/other/phthreads/Pre-built.2
GLUT_PATH_GCC = C:/libfreenect/other/glut-3.7.6
FREENECT_PATH_GCC = c:/libfreenect
FREENECT_BUILD_PATH_GCC = C:/libfreenect/ms

#DESTDIR = ../parlevision/libs/plugins
#INCLUDEPATH += ../parlevision/include
#LIBS += -lplvcore
#QMAKE_LIBDIR += ../parlevision/libs/plugins

#include (common.pri)

#macx {
#    LITERAL_DOT=.
#    LITERAL_LIB=lib
#    LIBRARYFILE = $$DISTDIR$$LITERAL_LIB$$TARGET$$LITERAL_DOT$$QMAKE_EXTENSION_SHLIB
#    LIBS+= -L../../libs/ParleVision.app/Contents/Frameworks
#    LIBS+= -framework OpenCV
#    QMAKE_POST_LINK  = install_name_tool -change libplvcore.dylib @loader_path/../Frameworks/libplvcore.dylib $$LIBRARYFILE
#    #QMAKE_POST_LINK += && install_name_tool -change libplvgui.dylib loader_path/../Frameworks/libplvgui.dylib $$LIBRARYFILE
#}
#win32 {
#    LIBS += -lcv200d \
#        -lcxcore200d \
#        -lcvaux200d \
#        -lhighgui200d \
#        -lcxts200d \
#        -lml200d
#    #do NOT use trailing slashes in the libdir, this will make the linker choke
#    QMAKE_LIBDIR += c:/OpenCV2.0/lib/Debug ../../libs ../../libs/plugins
#    INCLUDEPATH += c:/OpenCV2.0/include
#}

win32-msvc2010 {
	#added libs for freenect
	INCLUDEPATH += $${USB_PATH_GCC}/include
    QMAKE_LIBDIR += $${USB_PATH_GCC}/lib/gcc
    LIBS += -L$${USB_PATH_GCC}/bin/x86

    INCLUDEPATH += $${PTHREADS_PATH_GCC}/include
    QMAKE_LIBDIR += $${PTHREADS_PATH_GCC}/lib
    LIBS += -L$${PTHREADS_PATH_GCC}/lib

    INCLUDEPATH += $${GLUT_PATH_GCC}/include
    QMAKE_LIBDIR += $${GLUT_PATH_GCC}
    LIBS += -L$${GLUT_PATH_GCC}/include

    INCLUDEPATH += $${FREENECT_PATH_GCC}/include
    INCLUDEPATH += $${FREENECT_PATH_GCC}/wrappers/cpp
    #QMAKE_LIBDIR += $${FREENECT_BUILD_PATH_GCC}/lib
	QMAKE_LIBDIR += $${FREENECT_BUILD_PATH_GCC}/lib/Release
    LIBS += -L$${FREENECT_BUILD_PATH_GCC}/lib
}

win32 {
    CONFIG(debug, debug|release) {
        LIBS += -llibusb0 \
                #-lpthreadGC1 \
                -lpthreadVC2 \
                -lfreenect \
                -lfreenect_sync \
                -lglut32 
    }
    CONFIG(release, debug|release) {

        LIBS += -llibusb0 \
                #-lpthreadGC1 \
                -lpthreadVC2 \
                -lfreenect \
                -lfreenect_sync \
                -lglut32 
    }

    !contains(QMAKE_HOST.arch, x86_64) {
        message("x86 build")

        ## Windows x86 (32bit) specific build here

    } else {
        message("x86_64 build")

        ## Windows x64 (64bit) specific build here

    }
}

DEFINES += LIBFREENECT_PLUGIN_LIBRARY

SOURCES += libfreenectplugin.cpp \
    freenectproducer.cpp \
    hmifreenect.cpp

HEADERS +=  libfreenectplugin.h \
            libfreenect_plugin_global.h \
    freenectproducer.h \
    hmifreenect.h
