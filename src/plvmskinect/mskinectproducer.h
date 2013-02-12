#ifndef MSKINECTPRODUCER_H
#define MSKINECTPRODUCER_H

#include <plvcore/PipelineProducer.h>
#include <plvcore/CvMatData.h>
#include <plvcore/Pin.h>
#include <QMutex>

#include "mskinectdatatypes.h"

namespace plv
{
    class CvMatDataOutputPin;
}

namespace plvmskinect
{
    class KinectDevice;

    class MSKinectProducer : public plv::PipelineProducer
    {
        Q_OBJECT
        Q_DISABLE_COPY( MSKinectProducer )

        Q_CLASSINFO("author", "Robby van Delden, org Richard Loos")
        Q_CLASSINFO("name", "MSKinectProducer")
        Q_CLASSINFO("description", "This producer produces data from the "
                    "Microsoft Kinect using the MS Kinect SDK 1.0-1.6, infrared will not work for 1.0-1.5"
					"one can switch between infrared and colour only when the pipeline is not running, on request this choice can be made per Kinect"
					"the tilt motor contains some errors due to limited API, only -27-27 degree tilt with regard to base is possible"
					"However, the sensor is in the top so we can't know this relative angle, the absolute angle is given when angle is changed in gui"
					"the motor will be tilted when the checkbox above the angle is clicked, after turning it will turn to true again"
					"do not turn the motor regularly to minimize wear, API:  no more than once per second. In addition, you must allow at least 20 seconds of rest after 15 consecutive changes."
					"The Highres colour stream will result in a 12fps instead of the maximum 30fps"
					"The cutx, cuty, cutz values are in mm and define the distance to the left, right, or back from the Kinect that will be cut away"
					"This is implemented inside this device to speed it up by excluding non intersting pixels.")
		
		Q_PROPERTY( bool realWorldCoord READ getRealWorldCoord WRITE setRealWorldCoord NOTIFY realWorldCoordChanged  )
		Q_PROPERTY( bool infrared READ getInfrared WRITE setInfrared NOTIFY infraredChanged  )
        Q_PROPERTY( bool highres READ getHighres WRITE setHighres NOTIFY highresChanged  ) //highresColour
		Q_PROPERTY( int cutXL READ getCutXL WRITE setCutXL NOTIFY cutXChangedL  )
		Q_PROPERTY( int cutXR READ getCutXR WRITE setCutXR NOTIFY cutXChangedR  )
		Q_PROPERTY( int cutYU READ getCutYU WRITE setCutYU NOTIFY cutYChangedU  )
		Q_PROPERTY( int cutYD READ getCutYD WRITE setCutYD NOTIFY cutYChangedD  )
		Q_PROPERTY( int cutZ READ getCutZ WRITE setCutZ NOTIFY cutZChanged  )

		//angle of Kinect needs to be separte there is a maximum of 4Kinects per PC , todo I could not add GUI elements on initialisation, it is possible however
		Q_PROPERTY( bool rotateKinect1 READ getRotateKinect1 WRITE setRotateKinect1 NOTIFY rotateKinect1Changed  )
		Q_PROPERTY( int angleKinect1 READ getAngleKinect1 WRITE setAngleKinect1 NOTIFY angleKinect1Changed  )
		//2
		Q_PROPERTY( bool rotateKinect2 READ getRotateKinect2 WRITE setRotateKinect2 NOTIFY rotateKinect2Changed  )
		Q_PROPERTY( int angleKinect2 READ getAngleKinect2 WRITE setAngleKinect2 NOTIFY angleKinect2Changed  )
		//3
		Q_PROPERTY( bool rotateKinect3 READ getRotateKinect3 WRITE setRotateKinect3 NOTIFY rotateKinect3Changed  )
		Q_PROPERTY( int angleKinect3 READ getAngleKinect3 WRITE setAngleKinect3 NOTIFY angleKinect3Changed  )
		//4
		Q_PROPERTY( bool rotateKinect4 READ getRotateKinect4 WRITE setRotateKinect4 NOTIFY rotateKinect4Changed  )
		Q_PROPERTY( int angleKinect4 READ getAngleKinect4 WRITE setAngleKinect4 NOTIFY angleKinect4Changed  )
		
		/** required standard method declaration for plv::PipelineProducer */
        PLV_PIPELINE_PRODUCER

    public:
        MSKinectProducer();
        virtual ~MSKinectProducer();

        virtual bool init();
        virtual bool deinit() throw();
        virtual bool start();
        virtual bool stop();
		bool getRealWorldCoord() { return m_realWorldCoord; };
		bool getInfrared() { return m_infrared; };
		bool getHighres() { return m_highres; };
		bool getCutXL() { return m_cutxl; };
		bool getCutXR() { return m_cutxr; };
		bool getCutYU() { return m_cutyu; };
		bool getCutYD() { return m_cutyd; };
		
		//TODO independent cutable Kinects!!
		//bool getCutXL1() { return m_cutxl1; };
		//bool getCutXR1() { return m_cutxr1; };
		//bool getCutYU1() { return m_cutyu1; };
		//bool getCutYD1() { return m_cutyd1; };
		//
		//bool getCutXL2() { return m_cutxl2; };
		//bool getCutXR2() { return m_cutxr2; };
		//bool getCutYU2() { return m_cutyu2; };
		//bool getCutYD2() { return m_cutyd2; };
		//
		//bool getCutXL3() { return m_cutxl3; };
		//bool getCutXR3() { return m_cutxr3; };
		//bool getCutYU3() { return m_cutyu3; };
		//bool getCutYD3() { return m_cutyd3; };

		bool getCutZ() { return m_cutz; };
		/*bool getCutZ1() { return m_cutz; };
		bool getCutZ2() { return m_cutz; };
		bool getCutZ3() { return m_cutz; };*/

		int getAngleKinect(int device);
		//the actual activator
		void rotateKinect(int device, int angle);
	
	signals:
		void realWorldCoordChanged(bool newValue);
		void infraredChanged(bool newValue);
		void highresChanged(bool newValue);
		void cutXChangedL(int value);
		void cutXChangedR(int value);
		void cutYChangedU(int value);
		void cutYChangedD(int value);
		void cutZChanged(int value);

		void rotateKinect1Changed(bool newValue);
		void angleKinect1Changed(int newIntValue);
		//2
		void rotateKinect2Changed(bool newValue);
		void angleKinect2Changed(int newIntValue);
		//3
		void rotateKinect3Changed(bool newValue);
		void angleKinect3Changed(int newIntValue);
		//4
		void rotateKinect4Changed(bool newValue);
		void angleKinect4Changed(int newIntValue);

    public slots:
        void newDepthFrame( int deviceIndex, plv::CvMatData depth );
        void newVideoFrame( int deviceIndex, plv::CvMatData video );
        void newSkeletonFrame( int deviceIndex, plvmskinect::SkeletonFrame frame );
        void kinectFinished( int deviceIndex );
		
		void setRealWorldCoord(bool i) ;
		void setCutXL(int value);
		void setCutXR(int value);
		void setCutYU(int value);
		void setCutYD(int value);
		void setCutZ(int value);

		//this setting can't be altered in realtime
		void setInfrared(bool b) {m_infrared = b; qDebug()<< "restart pipeline to incorporate change"; emit(infraredChanged(b));}
		void setHighres(bool b) {m_highres = b; qDebug()<< "restart pipeline to incorporate res change"; emit(highresChanged(b));}
		

		//kinect activatoronly set once that is why i is used, acctually does not else than run rotate
		void setRotateKinect1(bool i) {!i ? rotateKinect(1,m_angleKinect1): i=true; emit (rotateKinect1Changed(true));}
		void setRotateKinect2(bool i) {!i ? rotateKinect(2,m_angleKinect2): i=true; emit (rotateKinect2Changed(true));}
		void setRotateKinect3(bool i) {!i ? rotateKinect(3,m_angleKinect3): i=true; emit (rotateKinect3Changed(true));}
		void setRotateKinect4(bool i) {!i ? rotateKinect(4,m_angleKinect4): i=true; emit (rotateKinect4Changed(true));}
		
		//set angle in GUI
		void setAngleKinect1(int alpha);
		void setAngleKinect2(int alpha);
		void setAngleKinect3(int alpha);
		void setAngleKinect4(int alpha);
		

    private:
        int m_deviceCount;
		bool m_realWorldCoord;
		bool m_infrared;
		bool m_highres;
		int m_cutxl;
		int m_cutxr;
		int m_cutyu;
		int m_cutyd;
		int m_cutz;

		//kinect activator bool and int
		bool m_rotateKinect1;
		int m_angleKinect1;
		//2
		bool m_rotateKinect2;
		int m_angleKinect2;
		//3
		bool m_rotateKinect3;
		int m_angleKinect3;
		//4
		bool m_rotateKinect4;
		int m_angleKinect4;

		//the kinect angle activator and angle
		bool getRotateKinect1() { return m_rotateKinect1; };
		int getAngleKinect1() {return m_angleKinect1; };
		//2
		bool getRotateKinect2() { return m_rotateKinect2; };
		int getAngleKinect2() {return m_angleKinect2; };
		//3
		bool getRotateKinect3() { return m_rotateKinect3; };
		int getAngleKinect3() {return m_angleKinect3; };
		//4
		bool getRotateKinect4() { return m_rotateKinect4; };
		int getAngleKinect4() {return m_angleKinect4; };
		//generic method, to get angle by sensor

        QVector<plv::CvMatDataOutputPin*> m_outputPinsVideo;
        QVector<plv::CvMatDataOutputPin*> m_outputPinsDepth;
        QVector<plv::OutputPin<SkeletonFrame>*> m_outputPinsSkeleton;

        mutable QMutex m_kinectProducerMutex;
        QVector< KinectDevice* > m_kinects;

        QVector<plv::CvMatData> m_videoFrames;
        QVector<plv::CvMatData> m_depthFrames;
        QVector<SkeletonFrame> m_skeletonFrames;

		//callback in plvm
	//	static void CALLBACK    Nui_StatusProcThunk(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void* pUserData);
	//   void CALLBACK           Nui_StatusProc( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName );
		static void CALLBACK    KinectStatusProc(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* deviceName, void* pUserData);
		void CALLBACK           Nui_StatusProc( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName );
    };
}

#endif // MSKINECTPRODUCER_H
