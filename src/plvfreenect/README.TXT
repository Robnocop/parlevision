This is the parlevision plugin for a libfreenect producer.

Installation instructions will appear here.

Requires libfreenect with all its dependencies; modify common.pri (especially the first 15 lines) to use your own builds of the dependent libraries.

If you have a  Windows 7 PC and your are lucky you might suffice with using our binaries and installing the driver manually. The pro is that it can run alongside the MRKinect drivers. To install using our pre-build binaries: go to device manager in control panel, (set view to by connection), find your kinect(s) under your USB root, (rmb on it) select properties, go to driver, go to update driver,  select browse my computer, select let me pick from a list, select have disk, browse, go to 
C:\libfreenect\platform\windows\inf\xbox nui motor select the Xbox_NUI_Motor.inf
do the same for the other drivers (camera)
somehow audio does not yet work, might come to that later.
You also need to add the files in "C:\libfreenect\ms\lib\Release" to your path. 


You will most probably have to build all libraries, including OpenCV (2.1.0 known to work), yourself using your own platform.

HINTS

For QTCreator, make sure that you
- work from a CLEAN command prompt (empth PATH) where you have set exactly the right paths to mingw32 and cmake.

set QTDIR=d:\Programs\qt-everywhere-opensource-src-4.7.1
set PATH=d:\Programs\qt-everywhere-opensource-src-4.7.1\bin;d:\Programs\qt-everywhere-opensource-src-4.7.1\lib;d:\Programs\qt-creator-2.2.1-build\bin;d:\Programs\MinGW\bin;%SystemRoot%\System32
set QMAKESPEC=win32-g++
....

For libfreenect to work you will need glut32, libusb and pththreadVC2. Libfreenect consists of the freenect dll&lib and the libfreenect_sync dll&lib.
Building libfreenect on windows machine will give some problems amongst others due to different versions of libusb. This was previously called usb. Binaries can be used as well to install and run libfreenect properly, unfortunatly libfreenect does not ship its binaries directly so you will have to find someone who allready has a proper build for your machine.

Libfreenect uses a different colour order so you need to switch rgb to bgr for example using the "color and scale conversion" processor in the pipeline. 

If building produces failures on first attempt try to rebuild the plvfreenect solution again when the rest is succesfully build.