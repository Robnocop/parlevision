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

DEFINES += LIBFREENECT_PLUGIN_LIBRARY

SOURCES += libfreenectplugin.cpp \
    freenectproducer.cpp \
    hmifreenect.cpp

HEADERS +=  libfreenectplugin.h \
            libfreenect_plugin_global.h \
    freenectproducer.h \
    hmifreenect.h
