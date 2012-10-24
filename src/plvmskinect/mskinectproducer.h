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

        Q_CLASSINFO("author", "Richard Loos")
        Q_CLASSINFO("name", "MSKinectProducer")
        Q_CLASSINFO("description", "This producer produces data from the "
                    "Microsoft Kinect using the MS Kinect SDK.")
		
		Q_PROPERTY( bool realWorldCoord READ getRealWorldCoord WRITE setRealWorldCoord NOTIFY realWorldCoordChanged  )
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
	
	signals:
		void realWorldCoordChanged(bool newValue);

    public slots:
        void newDepthFrame( int deviceIndex, plv::CvMatData depth );
        void newVideoFrame( int deviceIndex, plv::CvMatData video );
        void newSkeletonFrame( int deviceIndex, plvmskinect::SkeletonFrame frame );
        void kinectFinished( int deviceIndex );
		//temp fix it should not be changed in real time, nor do kinect devices have direct access to the settings
		void setRealWorldCoord(bool i) ;

    private:
        int m_deviceCount;
		bool m_realWorldCoord;

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
