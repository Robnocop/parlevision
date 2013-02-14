# using parlevision as a shared library
DEFINES += PLV_SHARED_LIBRARY

CONFIG  += qxt
QXT     += core gui
	
OPENCV_PATH_VS2010 = D:/ddevelop/OpenCV242/opencv/build/x86/vc10
#not tested
OPENCV_PATH_GCC = D:/ddevelop/OpenCV242/opencv/build/x86/mingw/

QXT_PATH = C:/Qt/Qxt
#if done correctly this shouldn't be needed, check the mkspecs/features directory if the qxt.prf and qxtvars.prf have been put there after installing qxt (run configure in a command window (windows startbutton->cmd, go to dir)
#INCLUDEPATH += $${QXT_PATH}/src/core
#INCLUDEPATH += $${QXT_PATH}/src/gui
QMAKE_LIBDIR += C:/Qt/Qxt/lib

#makes opencv openable
#D:/ddevelop/OpenCV242/opencv/build/x86/vc10
#name convention changes sometimes people use include <opencv/cv.h> other times include cv.h etc, ugly and safe way to go for now seems
INCLUDEPATH += D:/ddevelop/OpenCV242/opencv/build/include/opencv
INCLUDEPATH += D:/ddevelop/OpenCV242/opencv/build/include

#Library path relative to where source is located
#Do NOT use trailing slashes in the libdir, this will make the linker choke
QMAKE_LIBDIR += ../../libs
INCLUDEPATH += ../../include
INCLUDEPATH += $${QXT_PATH}/bin

#prerequisite to include file msr nui api but allready done in the kinect .pro file please verify! The same for the libfreenect, pre-build dlls of libfreenct can be sent on request
#INCLUDEPATH += $${KINECT_PATH}/inc
#KINECT_PATH = D:/ddevelop/mrKinectSDK
#KINECT_PATH = "C:/Program Files/Microsoft SDKs/Kinect/v1.0"
#added paths FOR LIBFREENECT are moved to plvfreenect.pro as well

#Windows specific libraries, library paths and include paths
win32-g++ {
    ## Windows common build for mingw here I don't use it so please give it a shot based on the msvc stuff
    LIBS += -L$${QXT_PATH}/lib
    LIBS += -L$${OPENCV_PATH_GCC}/lib/
    LIBS += -L$${OPENCV_PATH_GCC}/bin/
}

win32-msvc2010 {
    INCLUDEPATH += $${QXT_PATH}/include/QxtCore
	INCLUDEPATH += $${QXT_PATH}/include/QxtGui
	INCLUDEPATH += $${OPENCV_PATH_VS2010}/include
	INCLUDEPATH += $${OPENCV_PATH_VS2010}/bin
	LIBS += -L$${OPENCV_PATH_VS2010}/lib
	LIBS += -L$${QXT_PATH}/lib
	LIBS += -L$${QXT_PATH}/bin
}

#-lopencv_imgproc242d \
#the original common.pri uses a more sufficticated solution that would diminish the work to change this.
win32 {
    CONFIG(release,debug|release){
    LIBS += -lopencv_calib3d242 \
            -lopencv_contrib242 \
            -lopencv_core242 \
            -lopencv_features2d242 \
            -lopencv_flann242 \
            -lopencv_gpu242 \
            -lopencv_highgui242 \
            -lopencv_imgproc242 \
            -lopencv_legacy242 \
            -lopencv_ml242 \
            -lopencv_objdetect242 \
            -lopencv_ts242 \
            -lopencv_video242 \
	}
 
CONFIG(debug,debug|release){
    LIBS += -lopencv_calib3d242d\
            -lopencv_contrib242d \
            -lopencv_core242d \
            -lopencv_features2d242d \
            -lopencv_flann242d \
            -lopencv_gpu242d \
            -lopencv_highgui242d \
            -lopencv_imgproc242d \
            -lopencv_legacy242d \
            -lopencv_ml242d \
            -lopencv_objdetect242d \
            -lopencv_ts242d \
            -lopencv_video242d \
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
