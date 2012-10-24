#include "mskinectproducer.h"
#include "mskinectdevice.h"
#include <plvcore/CvMatDataPin.h>

using namespace plvmskinect;

MSKinectProducer::MSKinectProducer() : 
	m_deviceCount( 0 ),
	m_realWorldCoord(true)
{
    //NUIAPI HRESULT MSR_NUIGetDeviceCount( int * pCount ); // note capitalization
    //NUIAPI HRESULT MSR_NuiCreateInstanceByIndex( int Index, INuiInstance ** ppInstance );
    //NUIAPI void    MSR_NuiDestroyInstance( INuiInstance * pInstance );

    //MSR_NUIGetDeviceCount( &m_deviceCount );
	qDebug() << "getting count";
	NuiGetSensorCount( &m_deviceCount );
	qDebug() << "I do set device callback"; 
	//seems to create two statuscallbacks (runs twice
	//NuiSetDeviceStatusCallback(KinectStatusProc,&m_deviceCount);
	NuiSetDeviceStatusCallback(KinectStatusProc,NULL);

    if( m_deviceCount == 0 )
    {

    }
    else
    {
        m_depthFrames.resize( m_deviceCount );
        m_videoFrames.resize( m_deviceCount );
        m_skeletonFrames.resize( m_deviceCount );

        m_outputPinsDepth.resize( m_deviceCount );
        m_outputPinsVideo.resize( m_deviceCount );
        m_outputPinsSkeleton.resize( m_deviceCount );

        for( int i = 0; i < m_deviceCount; ++i )
        {
            KinectDevice* device = new KinectDevice(i, this);
            m_kinects.push_back(device);

            connect( device, SIGNAL( newDepthFrame( int, plv::CvMatData ) ),
                     this,     SLOT( newDepthFrame( int, plv::CvMatData ) ) );

            connect( device, SIGNAL( newVideoFrame( int, plv::CvMatData ) ),
                    this,     SLOT( newVideoFrame( int, plv::CvMatData ) ) );

            connect( device, SIGNAL( newSkeletonFrame( int, plvmskinect::SkeletonFrame ) ),
                     this,     SLOT( newSkeletonFrame( int, plvmskinect::SkeletonFrame ) ) );

            connect( device, SIGNAL( deviceFinished(int)), this, SLOT(kinectFinished(int)));

            // we have one output pin
            m_outputPinsDepth[i] = plv::createCvMatDataOutputPin( "depth", this );
            m_outputPinsVideo[i] = plv::createCvMatDataOutputPin( "camera", this );
            m_outputPinsSkeleton[i] = plv::createOutputPin<SkeletonFrame>( "skeletons", this );

            // supports all types of images
            m_outputPinsVideo[i]->addAllChannels();
            m_outputPinsVideo[i]->addAllDepths();

            m_outputPinsDepth[i]->addAllChannels();
            m_outputPinsDepth[i]->addAllDepths();
        }
    }
}

MSKinectProducer::~MSKinectProducer()
{
}


//////////////////////////////////////////////////////////////////////////////////////////
//added in producer as it shouldn't be added inside one device and is needed in new SDK to run multiple times
// CallBack 
//

void  CALLBACK  MSKinectProducer::KinectStatusProc(HRESULT hr, const OLECHAR* instanceName, const OLECHAR* deviceName, void * pUserData)
{   
	// a Kinect has been plugged or unplugged
	//TODO give feedback using the pUserData for pointing to a device number
	//int* pint ;
	//pint = (int*) pUserData;
	//qDebug() << tr("Kinect has been plugged or uplugged").arg(blabla);
	qDebug() << "Kinect has been plugged or uplugged";
	//qDebug() << tr("sensor count %1 ").arg(m_deviceCount);
	reinterpret_cast<MSKinectProducer *>(pUserData)->Nui_StatusProc( hr, instanceName, deviceName );
}


//-------------------------------------------------------------------
// Nui_StatusProc
//
// Callback to handle Kinect status changes
//-------------------------------------------------------------------
void CALLBACK MSKinectProducer::Nui_StatusProc( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName )
{
    // Update UI
	int nrOfDevices;
    if( SUCCEEDED(hrStatus) )
    {
		qDebug() << "you seem to have plugged in an additional Kinect!";
		
		//not allowed to read or edit m_deviceCount at this point
		NuiGetSensorCount( &nrOfDevices );

		if( nrOfDevices > 0) 
		{
			qDebug() << tr("you now got %1 working devices").arg(nrOfDevices);
			qDebug() << "please reinitialise/replace MSKinectProducer or restart Parlevision for additional needed pins";
		}
		else
		{
			qDebug() << "please reconnect Kinect, replace MSKinectProducer or restart Parlevision, you somehow wtill seem to have no devices left";
		}


		//MSKinectProducer::produce();
		/*  if ( S_OK == hrStatus )
        {
            if ( m_instanceId && 0 == wcscmp(instanceName, m_instanceId) )
            {
                Nui_Init(m_instanceId);
            }
            else if ( !m_pNuiSensor )
            {
                Nui_Init();
            }
        }*/
    }
    else
    {
		qDebug() << "you probably unplugged a Kinect this can lead to errors!!";
	/*	setError( PlvNonFatalError, tr("Kinect device quit unexpectedly."));
        emit onError( PlvNonFatalError, this );*/

		//MSKinectProducer::kinectFinished((int) instanceName);
       /* if ( m_instanceId && 0 == wcscmp(instanceName, m_instanceId) )
        {
            Nui_UnInit();
            Nui_Zero();
        }*/
    }
	//setError( PlvPipelineInitError, tr("Kinect status change see logger for more info. Please do not plug/unplug your Kinect while running Parlevision."));
}

bool MSKinectProducer::init()
{	
	//qDebug() << "I do set device callback"; 
	//NuiSetDeviceStatusCallback(KinectStatusProc,NULL);
	for( int i = 0; i < m_deviceCount; ++i )
    {
		//NuiSetDeviceStatusCallback(KinectStatusProc,NULL);
		//NuiSetDeviceStatusCallback((NuiStatusProc)&KinectStatusProc, NULL);
		//declared as QVector< KinectDevice* > m_kinects;
        if( !m_kinects.at(i)->init() )
        {
		    setError( PlvPipelineInitError, tr("Kinect with id %1 failed to initialise").arg(m_kinects.at(i)->getId()) );

			// deinit already initalized devices
            for( int j=0; j < i; ++j )
                m_kinects.at(j)->deinit();

            return false;
        }
		//add to deal with realspace cooridnates
		else
		{
			m_kinects.at(i)->setRealWorldCoord(m_realWorldCoord);
		}
    }
    return true;
}

bool MSKinectProducer::deinit() throw()
{
	//qDebug() << tr("deinit listcount %1 ").arg(m_kinects.count());
	//m_kinects.size();
	//NuiGetSensorCount( &m_deviceCount );
	qDebug() << tr("sensor count %1 ").arg(m_deviceCount);
	//setError( PlvPipelineInitError, tr("Number of Kinects %1 ").arg(m_deviceCount));
    for( int i = 0; i < m_deviceCount; ++i )
    {
        m_kinects.at(i)->deinit();
    }
    return true;
}

bool MSKinectProducer::start()
{
	for( int i = 0; i < m_deviceCount; ++i )
    {
        m_kinects.at(i)->start();
    }
    return true;
}


bool MSKinectProducer::stop()
{
    for( int i = 0; i < m_deviceCount; ++i )
    {
        m_kinects.at(i)->stop();
    }
    return true;
}

bool MSKinectProducer::produce()
{
    QMutexLocker lock( &m_kinectProducerMutex );
	
    for( int i = 0; i < m_deviceCount; ++i )
    {
        m_outputPinsDepth.at(i)->put( m_depthFrames.at(i) );
        m_outputPinsVideo.at(i)->put( m_videoFrames.at(i) );
		
		m_depthFrames[i] = cv::Mat();
        m_videoFrames[i] = cv::Mat();
		if( m_skeletonFrames.at(i).isValid() )
        {
            m_outputPinsSkeleton.at(i)->put( m_skeletonFrames.at(i) );
            m_skeletonFrames[i] = SkeletonFrame();
        }
    }
    return true;
}


bool MSKinectProducer::readyToProduce() const
{
    bool ready = true;
    for( int i=0; i < m_deviceCount && ready; ++i )
    {
        ready = m_depthFrames.at(i).isValid() && m_videoFrames.at(i).isValid();
    }
    return ready;
}

void MSKinectProducer::setRealWorldCoord(bool change)
{
	QMutexLocker lock(m_propertyMutex);
	m_realWorldCoord = change; 
	qDebug() << "restart pipeline to incorporate realworldcoord changes"; 
	for (int i=0;i<m_deviceCount;i++)
	{
		m_kinects.at(i)->setRealWorldCoord(change);
	}
	emit (realWorldCoordChanged(change));
}

void MSKinectProducer::newDepthFrame( int deviceIndex, plv::CvMatData depth )
{
    QMutexLocker lock( &m_kinectProducerMutex );
    assert( deviceIndex > -1 && deviceIndex < m_deviceCount );
    m_depthFrames[deviceIndex] = depth;
}

void MSKinectProducer::newVideoFrame( int deviceIndex, plv::CvMatData video )
{
    QMutexLocker lock( &m_kinectProducerMutex );
    assert( deviceIndex > -1 && deviceIndex < m_deviceCount );
    m_videoFrames[deviceIndex] = video;
}

void MSKinectProducer::newSkeletonFrame( int deviceIndex, plvmskinect::SkeletonFrame frame )
{
    QMutexLocker lock( &m_kinectProducerMutex );
    assert( deviceIndex > -1 && deviceIndex < m_deviceCount );
    m_skeletonFrames[deviceIndex] = frame;
}

void MSKinectProducer::kinectFinished( int deviceIndex )
{
	qDebug() << "kinectFinished signal"; 
    if( getState() > PLINITIALIZED )
    {
        setError( PlvNonFatalError, tr("Kinect with ID %1 quit unexpectedly.").arg(deviceIndex) );
        emit onError( PlvNonFatalError, this );
    }
}
