# using parlevision as a shared library
DEFINES += PLV_SHARED_LIBRARY

# org OPENCV_PATH_VS2010 = c:/develop/OpenCV2.1vs2008
OPENCV_PATH_VS2010 = c:/OpenCV-2.1.0/build
#OPENCV_PATH_GCC = c:/develop/OpenCV2.1.0
#not available??? to download!
OPENCV_PATH_GCC = c:/OpenCV-2.1.0/build
#OPENCV_PATH_GCC = c:/OpenCV-2.1.0/
KINECT_PATH = C:/mrKinectSDK
QXT_PATH = C:/Qt/libqxt-libqxt-v0.6.1
#added paths
USB_PATH_GCC = C:/libfreenect/other/libusb-win32-bin-1.2.5.0
PTHREADS_PATH_GCC = C:/libfreenect/other/phthreads/Pre-built.2
GLUT_PATH_GCC = C:/libfreenect/other/glut-3.7.6
FREENECT_PATH_GCC = c:/libfreenect
FREENECT_BUILD_PATH_GCC = C:/libfreenect/ms

#Include path relative to where source is located
#INCLUDEPATH += C:/Parlevision/include
#makes opencv openable
INCLUDEPATH += c:/OpenCV-2.1.0/include
INCLUDEPATH += $${QXT_PATH}/bin
#prerequisite to include file msr nui api
INCLUDEPATH += $${KINECT_PATH}/inc
INCLUDEPATH += ../../include

#Library path relative to where source is located
#Do NOT use trailing slashes in the libdir, this will make the linker choke
#QMAKE_LIBDIR += ../../libs
#QMAKE_LIBDIR += C:/Parlevision/libs
#worked sort of:
#QMAKE_LIBDIR += C:/OpenCV-2.1.0/build/lib/

#this is the qxt lib???
QMAKE_LIBDIR += C:/Qt/libqxt-libqxt-v0.6.1/lib
QMAKE_LIBDIR += ../../libs

#Windows specific libraries, library paths and include paths
win32-g++ {
    ## Windows common build for mingw here
    INCLUDEPATH += $${OPENCV_PATH_GCC}/include
    LIBS += -L$${QXT_PATH}/lib
    LIBS += -L$${OPENCV_PATH_GCC}/lib/Debug
    LIBS += -L$${OPENCV_PATH_GCC}/bin/Debug
#likely to be neccesary it is test what happens
    LIBS += -L$${KINECT_PATH}/lib
#this cant be neccesary
#    LIBS += C:/Parlevision/libs
}

win32-msvc2010 {
    INCLUDEPATH += $${OPENCV_PATH_VS2010}/include
    LIBS += -L$${QXT_PATH}/lib
    LIBS += -L$${OPENCV_PATH_VS2010}/lib/Debug
    LIBS += -L$${OPENCV_PATH_VS2010}/bin/Debug
#likely to be neccesary
    LIBS += -L$${KINECT_PATH}/lib
#this cant be neccesary
#    LIBS += C:/Parlevision/libs
#added libs for freenect
INCLUDEPATH += $${USB_PATH_GCC}/include
    QMAKE_LIBDIR += $${USB_PATH_GCC}/lib/gcc
    LIBS += -L$${USB_PATH_GCC}/bin/x86

    INCLUDEPATH += $${PTHREADS_PATH_GCC}/include
    QMAKE_LIBDIR += $${PTHREADS_PATH_GCC}/lib
    LIBS += -L$${PTHREADS_PATH_GCC}/lib

    INCLUDEPATH += $${GLUT_PATH_GCC}/include
    QMAKE_LIBDIR += $${GLUT_PATH_GCC}
    LIBS += -L$${GLUT_PATH_GCC}

    INCLUDEPATH += $${FREENECT_PATH_GCC}/include
    INCLUDEPATH += $${FREENECT_PATH_GCC}/wrappers/cpp
    #QMAKE_LIBDIR += $${FREENECT_BUILD_PATH_GCC}/lib
	QMAKE_LIBDIR += $${FREENECT_BUILD_PATH_GCC}/lib/Release
    LIBS += -L$${FREENECT_BUILD_PATH_GCC}/lib


}

win32 {
    CONFIG(debug, debug|release) {
        LIBS += -lcv210d \
                -lcxcore210d \
                -lcvaux210d \
                -lhighgui210d \
                -lcxts210d \
                -lml210d \ #added
				-llibusb0 \
                #-lpthreadGC1 \
                -lpthreadVC2 \
                -lfreenect \
                -lfreenect_sync \
                -lglut32 
    }
    CONFIG(release, debug|release) {

        LIBS += -lcv210 \
                -lcxcore210 \
                -lcvaux210 \
                -lhighgui210 \
                -lcxts210 \
                -lml210\ #added
				-llibusb0 \
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

#win32 {
#    ## Windows common build here
#    CONFIG(debug, debug|release) {
#        LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_ml
#        QMAKE_LIBDIR += c:/develop/OpenCV-2.2.0/lib
#        QMAKE_LIBDIR += c:/develop/OpenCV-2.2.0/bin
#        LIBS += -Lc:/develop/OpenCV-2.2.0/bin
#    }
#    CONFIG(release, debug|release) {
#        LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_ml
#        QMAKE_LIBDIR += c:/develop/OpenCV-2.2.0/lib
#        QMAKE_LIBDIR += c:/develop/OpenCV-2.2.0/bin
#        LIBS += -Lc:/develop/OpenCV-2.2.0/bin
#    }
#    INCLUDEPATH += c:/develop/OpenCV-2.2.0/include
#}

#Unix specific libraries
unix {
    LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_ml -lopencv_features2d -lopencv_objdetect -lopencv_calib3d -lopencv_video
    #LIBS += -lcv \
    #            -lcxcore \
    #            -lcvaux \
    #            -lhighgui \
    #            #-lcxts \
    #            -lml
}

macx {
    #QMAKE_LIBDIR += /opt/local/lib
    QMAKE_LIBDIR += /usr/local/Cellar/opencv/2.2/lib/
    #INCLUDEPATH += /opt/local/include/opencv
    INCLUDEPATH += /usr/local/Cellar/opencv/2.2/include/opencv
}
