#include "mskinectproducer.h"
#include "mskinectdevice.h"
#include <plvcore/CvMatDataPin.h>

//copy-pasted order from 1.0-1.8
enum BoneSelection{
	plv_NUI_SKELETON_POSITION_HIP_CENTER,
	plv_NUI_SKELETON_POSITION_SPINE	,
	plv_NUI_SKELETON_POSITION_SHOULDER_CENTER	,
	plv_NUI_SKELETON_POSITION_HEAD	,
	plv_NUI_SKELETON_POSITION_SHOULDER_LEFT	 ,
	plv_NUI_SKELETON_POSITION_ELBOW_LEFT,
	plv_NUI_SKELETON_POSITION_WRIST_LEFT,
	plv_NUI_SKELETON_POSITION_HAND_LEFT,
	plv_NUI_SKELETON_POSITION_SHOULDER_RIGHT,
	plv_NUI_SKELETON_POSITION_ELBOW_RIGHT,
	plv_NUI_SKELETON_POSITION_WRIST_RIGHT,
	plv_NUI_SKELETON_POSITION_HAND_RIGHT,
	plv_NUI_SKELETON_POSITION_HIP_LEFT,
	plv_NUI_SKELETON_POSITION_KNEE_LEFT,
	plv_NUI_SKELETON_POSITION_ANKLE_LEFT,
	plv_NUI_SKELETON_POSITION_FOOT_LEFT,
	plv_NUI_SKELETON_POSITION_HIP_RIGHT,
	plv_NUI_SKELETON_POSITION_KNEE_RIGHT,
	plv_NUI_SKELETON_POSITION_ANKLE_RIGHT,
	plv_NUI_SKELETON_POSITION_FOOT_RIGHT
//	plv_NUI_SKELETON_POSITION_FOOT_RIGHT,
//	plv_NUI_SKELETON_POSITION_COUNT 
};

using namespace plvmskinect;

MSKinectProducer::MSKinectProducer() : 
	m_deviceCount( 0 ),
	m_realWorldCoord(true),
	m_rotateKinect1(true),
	m_infrared(false),
	m_highres(false),
	m_facetrack(true),
	m_skeletons(true),
	m_cutxl(0),
	m_cutxr(0),
	m_cutyu(0),
	m_cutyd(0),
	m_cutz(0),
	m_cutxl1(0),  //I am ashamed to have this ugly solution array or something would be better
	m_cutxr1(0),
	m_cutyu1(0),
	m_cutyd1(0),
	m_cutz1(0),
	m_cutxl2(0),
	m_cutxr2(0),
	m_cutyu2(0),
	m_cutyd2(0),
	m_cutz2(0),
	m_cutxl3(0),
	m_cutxr3(0),
	m_cutyu3(0),
	m_cutyd3(0),
	m_cutz3(0),
	m_maxScaleX(8960),
	m_maxScaleY(6726),
	m_angleKinect1(0),
	m_angleKinect2(0),
	m_angleKinect3(0),
	m_angleKinect4(0)
{
    //NUIAPI HRESULT MSR_NUIGetDeviceCount( int * pCount ); // note capitalization
    //NUIAPI HRESULT MSR_NuiCreateInstanceByIndex( int Index, INuiInstance ** ppInstance );
    //NUIAPI void    MSR_NuiDestroyInstance( INuiInstance * pInstance );
	
	//Set kinect
	//setAngleKinect1(0);
    //MSR_NUIGetDeviceCount( &m_deviceCount );
	qDebug() << "getting count";
	NuiGetSensorCount( &m_deviceCount );
	
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
		m_rotationFrames.resize( m_deviceCount );
		m_translationFrames.resize( m_deviceCount );
		m_faceFeatureFrames.resize( m_deviceCount );
		m_bonePositionFrames.resize( m_deviceCount );
		m_boneRotationFrames.resize( m_deviceCount );

		m_faceFeatureVector.resize( m_deviceCount );
		//specific bone
		m_specificBoneFrames.resize( m_deviceCount );
		m_specificBoneRotationFrames.resize( m_deviceCount );

        m_outputPinsDepth.resize( m_deviceCount );
        m_outputPinsVideo.resize( m_deviceCount );
        m_outputPinsSkeleton.resize( m_deviceCount );
		m_outputPinsFaceRotation.resize( m_deviceCount );
		m_outputPinsFaceTranslation.resize( m_deviceCount );
		m_outputPinsFacePoints.resize( m_deviceCount );
		m_outputPinsBonesPositions.resize( m_deviceCount );
		//specific bone
		m_outputSpecifiedBonePositions.resize( m_deviceCount );
		m_outputSpecifiedBoneRotations.resize( m_deviceCount );

		m_skeletonReady.resize( m_deviceCount );
		m_faceTrackingReady.resize( m_deviceCount );

        for( int i = 0; i < m_deviceCount; ++i )
        {
            KinectDevice* device = new KinectDevice(i, this);
            m_kinects.push_back(device);

            connect( device, SIGNAL( newDepthFrame( int, plv::CvMatData ) ),
                     this,     SLOT( newDepthFrame( int, plv::CvMatData ) ) );

            connect( device, SIGNAL( newVideoFrame( int, plv::CvMatData ) ),
                    this,     SLOT( newVideoFrame( int, plv::CvMatData ) ) );

			//replaced plvmskinect::SkeletonFrame  with NUI_SKELETON_FRAME
            connect( device, SIGNAL( newSkeletonFrame( int, NUI_SKELETON_FRAME ) ),
                     this,     SLOT( newSkeletonFrame( int, NUI_SKELETON_FRAME ) ) );

			//TODO add for temp solution the signal containing the upperbody bones that will be passed and parsed to a string....
			connect( device, SIGNAL( newSkeletonPointsSignal( int, QVector<QVector4D>)),
					this, SLOT (newSkeletonPoints( int, QVector<QVector4D>)));
			//connect( device, SIGNAL( newSkeletonPointsSignal( int, QString)),
			//		this, SLOT (newSkeletonPoints( int, QString)));

			//changed
			connect( device, SIGNAL( newSkeletonRotationSignal( int, QVector<QVector4D>)),
					this, SLOT (newSkeletonRotationSignal( int, QVector<QVector4D>)));

            connect( device, SIGNAL( deviceFinished(int)), this, SLOT(kinectFinished(int)));
			//added for facetracker
			connect( device, SIGNAL( newfacerotation(int, float, float, float)), this, SLOT(newFaceRotation(int, float, float, float)));
			connect( device, SIGNAL( newfacetranslation(int, float, float, float)), this, SLOT(newFaceTranslation(int, float, float, float)));
			connect( device, SIGNAL( newfacefeatures(int, QVector<cv::Point2f>)), this, SLOT(newFaceFeatures(int, QVector<cv::Point2f>)));

            // we have one output pin
            m_outputPinsDepth[i] = plv::createCvMatDataOutputPin( "depth", this );
            m_outputPinsVideo[i] = plv::createCvMatDataOutputPin( "camera", this );
            //replaced m_outputPinsSkeleton[i] = plv::createOutputPin<SkeletonFrame>( "skeletons", this );
			m_outputPinsSkeleton[i] = plv::createOutputPin<NUI_SKELETON_FRAME>( "skeletons", this );
			//facetracker
			m_outputPinsFaceRotation[i] = plv::createOutputPin<QString>( "h-rotations", this );
			m_outputPinsFaceTranslation[i] = plv::createOutputPin<QString>( "h-position", this );
			m_outputPinsFacePoints[i] = plv::createOutputPin<QString>( "f-points", this );
			m_outputPinsBonesPositions[i] = plv::createOutputPin<QString>( "bones-pos", this );
			//specific bone TODO select the bone
			m_outputSpecifiedBonePositions[i] = plv::createOutputPin<QVector4D>( "one specifc bone", this );
			m_outputSpecifiedBoneRotations[i] = plv::createOutputPin<QVector4D>( "one specifc bone rot", this );

            // supports all types of images
            m_outputPinsVideo[i]->addAllChannels();
            m_outputPinsVideo[i]->addAllDepths();

            m_outputPinsDepth[i]->addAllChannels();
            m_outputPinsDepth[i]->addAllDepths();
			m_skeletonReady[i]= false;
			m_faceTrackingReady[i]=false;
        }
    }

	//enum:
	//style: m_method.add("gray", BGSM_GRAYSCALE);
	m_boneEnum.add("HIP_CENTER", plv_NUI_SKELETON_POSITION_HIP_CENTER);
	m_boneEnum.add("SPINE", plv_NUI_SKELETON_POSITION_SPINE);
	m_boneEnum.add("SHOULDER_CENTER", plv_NUI_SKELETON_POSITION_SHOULDER_CENTER);
	m_boneEnum.add("HEAD", plv_NUI_SKELETON_POSITION_HEAD);
	m_boneEnum.add("SHOULDER_LEFT", plv_NUI_SKELETON_POSITION_SHOULDER_LEFT);
	m_boneEnum.add("ELBOW_LEFT", plv_NUI_SKELETON_POSITION_ELBOW_LEFT);
	m_boneEnum.add("WRIST_LEFT", plv_NUI_SKELETON_POSITION_WRIST_LEFT);
	m_boneEnum.add("HAND_LEFT", plv_NUI_SKELETON_POSITION_HAND_LEFT);
	m_boneEnum.add("SHOULDER_RIGHT", plv_NUI_SKELETON_POSITION_SHOULDER_RIGHT);
	m_boneEnum.add("ELBOW_RIGHT", plv_NUI_SKELETON_POSITION_ELBOW_RIGHT);
	m_boneEnum.add("WRIST_RIGHT", plv_NUI_SKELETON_POSITION_WRIST_RIGHT);
	m_boneEnum.add("HAND_RIGHT", plv_NUI_SKELETON_POSITION_HAND_RIGHT);
	m_boneEnum.add("HIP_LEFT", plv_NUI_SKELETON_POSITION_HIP_LEFT);
	m_boneEnum.add("KNEE_LEFT", plv_NUI_SKELETON_POSITION_KNEE_LEFT);
	m_boneEnum.add("ANKLE_LEFT", plv_NUI_SKELETON_POSITION_ANKLE_LEFT);
	m_boneEnum.add("FOOT_LEFT", plv_NUI_SKELETON_POSITION_FOOT_LEFT);
	m_boneEnum.add("HIP_RIGHT", plv_NUI_SKELETON_POSITION_HIP_RIGHT);
	m_boneEnum.add("KNEE_RIGHT", plv_NUI_SKELETON_POSITION_KNEE_RIGHT);
	m_boneEnum.add("ANKLE_RIGHT", plv_NUI_SKELETON_POSITION_ANKLE_RIGHT);
	m_boneEnum.add("FOOT_RIGHT", plv_NUI_SKELETON_POSITION_FOOT_RIGHT);
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
	qDebug() << "Kinect has been plugged or uplugged";
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
			qDebug() << "please reconnect Kinect, replace MSKinectProducer or restart Parlevision, you somehow still seem to have no devices";
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
	m_specifiedbone = 0;

	//setAngleKinect1(0);
	//qDebug() << "I do set device callback"; 
	//NuiSetDeviceStatusCallback(KinectStatusProc,NULL);
	for( int i = 0; i < m_deviceCount; ++i )
    {
		m_skeletonReady[i] = false;
		m_faceTrackingReady[i] =false;
		//NuiSetDeviceStatusCallback(KinectStatusProc,NULL);
		//NuiSetDeviceStatusCallback((NuiStatusProc)&KinectStatusProc, NULL);
		//declared as QVector< KinectDevice* > m_kinects;
		
		//the infrared//color input can not be switched during the process so should be set in the init or start;
		m_kinects.at(i)->m_infrared=getInfrared();
		m_kinects.at(i)->m_highres=getHighres();
		//facetrack as well...
		m_kinects.at(i)->m_facetracking=getFacetrack();
		m_kinects.at(i)->m_skeletontracking= getSkeletons();

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
			//sorry again for the ugliness
			switch(i) {
				case 0:
					m_kinects.at(i)->setCutXL(m_cutxl);
					m_kinects.at(i)->setCutXR(m_cutxr);
					m_kinects.at(i)->setCutYU(m_cutyu);
					m_kinects.at(i)->setCutYD(m_cutyd);
					m_kinects.at(i)->setCutZ(m_cutz);
					break;
				case 1:
					m_kinects.at(i)->setCutXL(m_cutxl1);
					m_kinects.at(i)->setCutXR(m_cutxr1);
					m_kinects.at(i)->setCutYU(m_cutyu1);
					m_kinects.at(i)->setCutYD(m_cutyd1);
					m_kinects.at(i)->setCutZ(m_cutz1);
					break;
				case 2:
					m_kinects.at(i)->setCutXL(m_cutxl2);
					m_kinects.at(i)->setCutXR(m_cutxr2);
					m_kinects.at(i)->setCutYU(m_cutyu2);
					m_kinects.at(i)->setCutYD(m_cutyd2);
					m_kinects.at(i)->setCutZ(m_cutz2);
					break;
				case 3:
					m_kinects.at(i)->setCutXL(m_cutxl3);
					m_kinects.at(i)->setCutXR(m_cutxr3);
					m_kinects.at(i)->setCutYU(m_cutyu3);
					m_kinects.at(i)->setCutYD(m_cutyd3);
					m_kinects.at(i)->setCutZ(m_cutz3);
					break;
			}
		}

		//check the gui and set the facetracker if neccesary
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
		//TODO I assume one skeleton tracking device this might change in the future
		m_skeletonReady[i] = false;
		m_faceTrackingReady[i]=false;
        m_kinects.at(i)->start();
		/*if (i==1) setAngleKinect1((int) getAngleKinect(i));
		if (i==2) setAngleKinect2((int) getAngleKinect(i));
		if (i==3) setAngleKinect3((int) getAngleKinect(i));
		if (i==4) setAngleKinect4((int) getAngleKinect(i));
		*///setAngleKinect1((int) getAngleKinect(1));
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
	
	//for multiple Kinects this might give errors, as skeletonready 
    for( int i = 0; i < m_deviceCount; ++i )
    {
        m_outputPinsDepth.at(i)->put( m_depthFrames.at(i) );
		
		//added temp! draw on the video when size of features > 0
		//TODO add a seperate pin
		plv::CvMatData drawface = m_videoFrames.at(i);
		cv::Mat& image = drawface;
		//? To check is this thread safe?
		//if (m_faceFeatureVector[i].size()>0)
		if (m_faceTrackingReady[i])
		{
			for( int j = 0; j < m_faceFeatureVector[i].size(); j++ )
			{	
				//circle( image, cv::Point((int) m_faceFeatureVector[i].at(j).x,(int) m_faceFeatureVector[i].at(j).y), 5, cv::Scalar(0,255,255), -1, 8);
				circle( image, cv::Point((int) m_faceFeatureVector[i].at(j).x,(int) m_faceFeatureVector[i].at(j).y), 2, cv::Scalar(0,255,255), 1, 8);
			}
			m_outputPinsFaceRotation.at(i)->put( m_rotationFrames.at(i) );
			m_outputPinsFaceTranslation.at(i)->put( m_translationFrames.at(i) );
			m_outputPinsFacePoints.at(i)->put( m_faceFeatureFrames.at(i) );
			m_faceTrackingReady[i] = false;
			//TODO make thread safe and in proper format
			//m_rotationFrames[i] = QString("0.0 \t 0.0 \t 0.0");
			//m_translationFrames[i] = QString("0.0 \t 0.0 \t 0.0");
		}
		/////////
		//TODO seperate the copy with facefeatures from the normal rgb image
		//m_outputPinsVideo.at(i)->put( m_videoFrames.at(i) );
		m_outputPinsVideo.at(i)->put( image );
		

		//empty
		//?threadsafety?
		//TO check why do I need to reset these frames?
		m_depthFrames[i] = cv::Mat();
        m_videoFrames[i] = cv::Mat();
		
		//TO check do we need to reset these values?
		//m_bonePositionFrames[i] = QString("0.0 \t 0.0 \t 0.0");
		//m_faceFeatureFrames[i] = QString("0.0 \t 0.0 \t 0.0");

		//replaced if( m_skeletonFrames.at(i).isValid() )
		
		if( m_skeletonReady[i] )
        {
            m_outputPinsSkeleton.at(i)->put( m_skeletonFrames.at(i) );
            m_skeletonFrames[i] = NUI_SKELETON_FRAME();
			m_outputPinsBonesPositions.at(i)->put( m_bonePositionFrames.at(i) );
			//specific bone
			m_outputSpecifiedBonePositions.at(i)->put(m_specificBoneFrames.at(i));
			m_outputSpecifiedBoneRotations.at(i)->put(m_specificBoneRotationFrames.at(i));

			m_skeletonReady[i] =false;
		}
		else
		{
			//send empty line to also trigger events when no bone info is there, probably needed for keystrokes to work
			//TODO make this choice a bool in the GUI
			QVector4D neues;
			
			m_outputSpecifiedBonePositions.at(i)->put(neues);
			m_outputSpecifiedBoneRotations.at(i)->put(neues);
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

//replaced
void MSKinectProducer::newSkeletonFrame( int deviceIndex, NUI_SKELETON_FRAME frame )
{
    QMutexLocker lock( &m_kinectProducerMutex );
    assert( deviceIndex > -1 && deviceIndex < m_deviceCount );
    m_skeletonFrames[deviceIndex] = frame;
	m_skeletonReady[deviceIndex] = true;
}

void MSKinectProducer::newFaceRotation(int deviceIndex, float x, float y, float z)
{
	QMutexLocker lock( &m_kinectProducerMutex );
	assert( deviceIndex > -1 && deviceIndex < m_deviceCount ); 
	//TODO 
	QString str = QString::number(x).append("\t").append(QString::number(y)).append("\t").append(QString::number(z));
	m_rotationFrames[deviceIndex] = str;
}

void MSKinectProducer::newFaceTranslation(int deviceIndex, float x, float y, float z)
{
	QMutexLocker lock( &m_kinectProducerMutex );
	assert( deviceIndex > -1 && deviceIndex < m_deviceCount ); 
	//TODO 
	QString str = QString::number(x).append("\t").append(QString::number(y)).append("\t").append(QString::number(z));
	m_translationFrames[deviceIndex] = str;
}

//TODO this is inefficient, I recast it twice now
//take into account that the i != the deviceindex!
void MSKinectProducer::newFaceFeatures(int deviceIndex, QVector<cv::Point2f> faceFeaturePoints)
{
	QMutexLocker lock( &m_kinectProducerMutex );
	assert( deviceIndex > -1 && deviceIndex < m_deviceCount ); 
	//qDebug() << "facefeatures arrive in producer";	
	QString str;
	for (int i = 0;i<faceFeaturePoints.size();i++)
	{
		//QString tempString = QString::number(m_UpperBodyPoints[0].x).append("\t").append(QString::number(m_UpperBodyPoints[0].y)).append("\t").append(QString::number(m_UpperBodyPoints[0].z));
		
		//qDebug() << "bone" << i << " x: " << skeletonArray[i].x() << "\t ,y:" << skeletonArray[i].y() << "\t ,z:" <<skeletonArray[i].z();
		if (i<faceFeaturePoints.size()-1)
			str = str.append( QString::number(faceFeaturePoints[i].x).append("\t").append(QString::number(faceFeaturePoints[i].y)).append("\t"));
		else
			str = str.append( QString::number(faceFeaturePoints[i].x).append("\t").append(QString::number(faceFeaturePoints[i].y)));
		//line-end added automatically in qdebug() functions
	}
	//end is added in the string to file processor
	//str.append("\n");
	m_faceFeatureVector[deviceIndex] = faceFeaturePoints;
	m_faceFeatureFrames[deviceIndex] = str;
	m_faceTrackingReady[deviceIndex] = true;
}

void MSKinectProducer::newSkeletonPoints( int deviceIndex, QVector<QVector4D> skeletonArray) //QString skeletonArray)
{
	//TODO make this a pin with \n and \t string formated bones so they can be read or.... make a new proccesorr 
	QMutexLocker lock( &m_kinectProducerMutex );
	assert( deviceIndex > -1 && deviceIndex < m_deviceCount ); 
	
	//qDebug() << "got a skeleton with size " << skeletonArray.size();
	QString str;
	for (int i = 0;i<skeletonArray.size();i++)
	{
		//QString tempString = QString::number(m_UpperBodyPoints[0].x).append("\t").append(QString::number(m_UpperBodyPoints[0].y)).append("\t").append(QString::number(m_UpperBodyPoints[0].z));
		
		//qDebug() << "bone" << i << " x: " << skeletonArray[i].x() << "\t ,y:" << skeletonArray[i].y() << "\t ,z:" <<skeletonArray[i].z();
		
		//make a tabdelimited list of bones, every bone seperated by a tab, the last line should not end with an empty field
		if(i<skeletonArray.size()-1)
			str = str.append( QString::number(skeletonArray[i].x()).append("\t").append(QString::number(skeletonArray[i].y())).append("\t").append(QString::number(skeletonArray[i].z())).append("\t"));
		else
			str = str.append( QString::number(skeletonArray[i].x()).append("\t").append(QString::number(skeletonArray[i].y())).append("\t").append(QString::number(skeletonArray[i].z())));
		//line-end added automatically in qdebug() functions
		
		//QVector4D checkthis = skeletonArray.at(i);
		//specific bone
		if (i == m_specifiedbone)
			m_specificBoneFrames[deviceIndex] = skeletonArray[i];
	}
	m_bonePositionFrames[deviceIndex] = str;
	
	//for (int i = 0;i<NUI_SKELETON_COUNT;i++)
	//{
	//}
	//m_skeletonPointsFrame 
}


void MSKinectProducer::newSkeletonRotationSignal( int deviceIndex, QVector<QVector4D> skeletonRotationArray) //QString skeletonArray)
{
	//TODO make this a pin with \n and \t string formated bones so they can be read or.... make a new proccesorr 
	QMutexLocker lock( &m_kinectProducerMutex );
	assert( deviceIndex > -1 && deviceIndex < m_deviceCount ); 
	
	//qDebug() << "got a skeleton with size " << skeletonArray.size();
	QString str;
	for (int i = 0;i<skeletonRotationArray.size();i++)
	{
		//QString tempString = QString::number(m_UpperBodyPoints[0].x).append("\t").append(QString::number(m_UpperBodyPoints[0].y)).append("\t").append(QString::number(m_UpperBodyPoints[0].z));
		
		//qDebug() << "bone" << i << " x: " << skeletonArray[i].x() << "\t ,y:" << skeletonArray[i].y() << "\t ,z:" <<skeletonArray[i].z();
		
		//make a tabdelimited list of bones, every bone seperated by a tab, the last line should not end with an empty field
		if(i<skeletonRotationArray.size()-1)
			str = str.append( QString::number(skeletonRotationArray[i].x()).append("\t").append(QString::number(skeletonRotationArray[i].y())).append("\t").append(QString::number(skeletonRotationArray[i].z())).append("\t"));
		else
			str = str.append( QString::number(skeletonRotationArray[i].x()).append("\t").append(QString::number(skeletonRotationArray[i].y())).append("\t").append(QString::number(skeletonRotationArray[i].z())));
		//line-end added automatically in qdebug() functions
		
		//QVector4D checkthis = skeletonArray.at(i);
		//specific bone
		if (i == m_specifiedbone)
			m_specificBoneRotationFrames[deviceIndex] = skeletonRotationArray[i];
	}
	m_boneRotationFrames[deviceIndex] = str;
	
	//for (int i = 0;i<NUI_SKELETON_COUNT;i++)
	//{
	//}
	//m_skeletonPointsFrame 
}

void MSKinectProducer::kinectFinished( int deviceIndex )
{
	qDebug() << "kinectFinished signal"; 
    if( getState() > PLE_INITIALIZED )
    {
        setError( PlvNonFatalError, tr("Kinect with ID %1 quit unexpectedly.").arg(deviceIndex) );
        emit onError( PlvNonFatalError, this );
    }
}

void MSKinectProducer::setMaxScale()
{
	int templ;
	int tempr;
	int tempu;
	int tempd;
	m_maxScaleX =0;
	m_maxScaleY =0;

	//need to ttest all the values every time one changes
	if (m_cutxl>0)
	{
		templ = m_cutxl;
	}
	else
	{
		templ = 4480;
	}
	
	if (m_cutxr>0)
	{
		tempr = m_cutxr;
	}
	else {
		tempr = 4480;
	}

	if (m_cutyu>0)
	{
		tempu = m_cutyu;
	}
	else
	{
		tempu = 3363;
	}

	if (m_cutyd>0)
	{
		tempd = m_cutyd;
	}
	else
	{
		tempd = 3363;
	}
	
	if ((templ+tempr)>m_maxScaleX) {m_maxScaleX=templ+tempr;};
	if ((tempu+tempd)>m_maxScaleY) {m_maxScaleY=tempu+tempd;};

	if (m_cutxl1>0)
	{
		templ = m_cutxl1;
	}
	else
	{
		templ = 4480;
	}
	
	if (m_cutxr1>0)
	{
		tempr = m_cutxr1;
	}
	else {
		tempr = 4480;
	}

	if (m_cutyu1>0)
	{
		tempu = m_cutyu1;
	}
	else
	{
		tempu = 3363;
	}

	if (m_cutyd1>0)
	{
		tempd = m_cutyd1;
	}
	else
	{
		tempd = 3363;
	}

	if ( m_deviceCount>1)
	{
		if ((templ+tempr)>m_maxScaleX) m_maxScaleX=templ+tempr;
		if ((tempu+tempd)>m_maxScaleY) m_maxScaleY=tempu+tempd;
	}

	if (m_cutxl2>0)
	{
		templ = m_cutxl2;
	}
	else
	{
		templ = 4480;
	}
	
	if (m_cutxr2>0)
	{
		tempr = m_cutxr2;
	}
	else {
		tempr = 4480;
	}

	if (m_cutyu2>0)
	{
		tempu = m_cutyu2;
	}
	else
	{
		tempu = 3363;
	}

	if (m_cutyd2>0)
	{
		tempd = m_cutyd2;
	}
	else
	{
		tempd = 3363;
	}

	if ( m_deviceCount>2)
	{
		if ((templ+tempr)>m_maxScaleX) m_maxScaleX=templ+tempr;
		if ((tempu+tempd)>m_maxScaleY) m_maxScaleY=tempu+tempd;
	}

	if (m_cutxl3>0)
	{
		templ = m_cutxl3;
	}
	else
	{
		templ = 4480;
	}
	
	if (m_cutxr3>0)
	{
		tempr = m_cutxr3;
	}
	else {
		tempr = 4480;
	}

	if (m_cutyu3>0)
	{
		tempu = m_cutyu3;
	}
	else
	{
		tempu = 3363;
	}

	if (m_cutyd3>0)
	{
		tempd = m_cutyd3;
	}
	else
	{
		tempd = 3363;
	}
	
	if ( m_deviceCount>3)
	{
		if ((templ+tempr)>m_maxScaleX) m_maxScaleX=templ+tempr;
		if ((tempu+tempd)>m_maxScaleY) m_maxScaleY=tempu+tempd;
	}

	for (int i=0;i<m_deviceCount;i++)
	{
		m_kinects.at(i)->setMaxScale(m_maxScaleX,m_maxScaleY);
		qDebug() << "set scale in producer" << m_maxScaleX << "y is "<< m_maxScaleY;
	}

}

void MSKinectProducer::setRealWorldCoord(bool change)
{
	QMutexLocker lock(m_propertyMutex);
	if (!getFacetrack() && !getSkeletons())
	{
		m_realWorldCoord = change; 
		for (int i=0;i<m_deviceCount;i++)
		{
			m_kinects.at(i)->setRealWorldCoord(change);
		}
		emit (realWorldCoordChanged(change));
	}
	else
	{
		qDebug()<< "realwordcoord can't be used for now when in facetrack mode"; 
		m_realWorldCoord = false;
		emit (realWorldCoordChanged(false));
	}
}

void MSKinectProducer::setFacetrack(bool change)
{
	QMutexLocker lock(m_propertyMutex);
	qDebug()<< "restart pipeline to incorporate facetrack change, realwordcoord can't be used for now in facetrack mode"; 
	if (change)
	{
		setRealWorldCoord(false);
		setSkeletons(true);
		//emit (realWorldCoordChanged(false));
	}
	
	m_facetrack = change; 

	for (int i=0;i<m_deviceCount;i++)
	{
		//only at start not at runtime as it creates threads etc. 
		//m_kinects.at(i)->setRealWorldCoord(change);
	}

	emit facetrackChanged(change);

}

void MSKinectProducer::setSkeletons(bool change)
{
	QMutexLocker lock(m_propertyMutex);
	qDebug()<< "restart pipeline to incorporate change in skeleton tracking, realwordcoord can't be used for now in skeleton tracking mode"; 
	if (change)
	{
		setRealWorldCoord(false);
		//emit (realWorldCoordChanged(false));
	}
	//we can only track face if we track skeletons
	else
	{
		setFacetrack(false);
	}
	
	m_skeletons = change; 

	for (int i=0;i<m_deviceCount;i++)
	{
		//only at start not at runtime as it creates threads etc. 
		//m_kinects.at(i)->setRealWorldCoord(change);
	}

	emit skeletonsChanged(change);

}

////make clear to user that we don't use realworld coordinates if we are tracking faces
//		if (change)
//			m_kinects.at(i)->setRealWorldCoord(change);

void MSKinectProducer::setCutXL(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutxl = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==0)
		{
			m_kinects.at(i)->setCutXL(value);
		}
	}
	setMaxScale();
	emit (cutXChangedL(value));
}

void MSKinectProducer::setCutXR(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutxr = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==0)
		{
			m_kinects.at(i)->setCutXR(value);
		}
	}
	emit (cutXChangedR(value));
	setMaxScale();
}

void MSKinectProducer::setCutYU(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutyu = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==0)
		{
			m_kinects.at(i)->setCutYU(value);
		}
	}
	emit (cutYChangedU(value));
	setMaxScale();
}

void MSKinectProducer::setCutYD(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutyd = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==0)
		{
			m_kinects.at(i)->setCutYD(value);
		}
	}
	emit (cutYChangedD(value));
	setMaxScale();
}


void MSKinectProducer::setCutZ(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutz = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==0)
		{
			m_kinects.at(i)->setCutZ(value);
		}
	}
	emit (cutZChanged(value));
	
}

////KINECT 2

void MSKinectProducer::setCutXL1(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutxl1 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==1)
		{
			m_kinects.at(i)->setCutXL(value);
		}
	}
	emit (cutXChangedL1(value));
	setMaxScale();
}

void MSKinectProducer::setCutXR1(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutxr1 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==1)
		{
			m_kinects.at(i)->setCutXR(value);
		}
	}
	emit (cutXChangedR1(value));
	setMaxScale();
}

void MSKinectProducer::setCutYU1(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutyu1 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==1)
		{
			m_kinects.at(i)->setCutYU(value);
		}
	}
	emit (cutYChangedU1(value));
	setMaxScale();
}

void MSKinectProducer::setCutYD1(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutyd1 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==1)
		{
			m_kinects.at(i)->setCutYD(value);
		}
	}
	emit (cutYChangedD1(value));
	setMaxScale();
}


void MSKinectProducer::setCutZ1(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutz1 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==1)
		{
			m_kinects.at(i)->setCutZ(value);
		}
	}
	emit (cutZChanged1(value));

}

///KINECT 3
void MSKinectProducer::setCutXL2(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutxl2 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==2)
		{
			m_kinects.at(i)->setCutXL(value);
		}
	}
	emit (cutXChangedL2(value));
	setMaxScale();
}

void MSKinectProducer::setCutXR2(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutxr2 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==2)
		{
			m_kinects.at(i)->setCutXR(value);
		}
	}
	emit (cutXChangedR2(value));
	setMaxScale();
}

void MSKinectProducer::setCutYU2(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutyu2 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==2)
		{
			m_kinects.at(i)->setCutYU(value);
		}
	}
	emit (cutYChangedU2(value));
	setMaxScale();
}

void MSKinectProducer::setCutYD2(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutyd2 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==2)
		{
			m_kinects.at(i)->setCutYD(value);
		}
	}
	emit (cutYChangedD2(value));
	setMaxScale();
}


void MSKinectProducer::setCutZ2(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutz2 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==2)
		{
			m_kinects.at(i)->setCutZ(value);
		}
	}
	emit (cutZChanged2(value));
}

//KINECT 4
void MSKinectProducer::setCutXL3(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutxl3 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==3)
		{
			m_kinects.at(i)->setCutXL(value);
		}
	}
	emit (cutXChangedL3(value));
	setMaxScale();
}

void MSKinectProducer::setCutXR3(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutxr3 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==3)
		{
			m_kinects.at(i)->setCutXR(value);
		}
	}
	emit (cutXChangedR3(value));
	setMaxScale();
}

void MSKinectProducer::setCutYU3(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutyu3 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==3)
		{
			m_kinects.at(i)->setCutYU(value);
		}
	}
	emit (cutYChangedU3(value));
	setMaxScale();
}

void MSKinectProducer::setCutYD3(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutyd3 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==3)
		{
			m_kinects.at(i)->setCutYD(value);
		}
	}
	emit (cutYChangedD3(value));
	setMaxScale();
}


void MSKinectProducer::setCutZ3(int value)
{
	QMutexLocker lock(m_propertyMutex);
	m_cutz3 = value; 
	for (int i=0;i<m_deviceCount;i++)
	{
		if(i ==3)
		{
			m_kinects.at(i)->setCutZ(value);
		}
	}
	emit (cutZChanged3(value));
}

//might want to add a limitation e.g no more than 2*27 degrees
void MSKinectProducer::setAngleKinect1(int angle)
{
	QMutexLocker lock(m_propertyMutex);
	m_angleKinect1 = angle;
	angleKinect1Changed(m_angleKinect1);
	qDebug() << "actual angle" << getAngleKinect(0);
}

void MSKinectProducer::setAngleKinect2(int angle)
{
	QMutexLocker lock(m_propertyMutex);
	m_angleKinect2 = angle;
	angleKinect2Changed(m_angleKinect2);
	qDebug() << "actual angle" << getAngleKinect(1);
}

void MSKinectProducer::setAngleKinect3(int angle)
{
	QMutexLocker lock(m_propertyMutex);
	m_angleKinect3 = angle;
	angleKinect3Changed(m_angleKinect3);
	qDebug() << "actual angle" << getAngleKinect(2);
}

void MSKinectProducer::setAngleKinect4(int angle)
{
	QMutexLocker lock(m_propertyMutex);
	m_angleKinect4 = angle;
	angleKinect4Changed(m_angleKinect4);
	qDebug() << "actual angle" << getAngleKinect(3);
}

//seperate method to get actual angle by sensor
int MSKinectProducer::getAngleKinect(int device)
{
	//???-1 or 0 TODO fix this tempfix, it would now be undoable to rotate the first kinect.
	if (false) //(device>0 && device <m_deviceCount)
	{
 		return m_kinects.at(device)->getAngle();
	}
	else
	{
		qDebug() << "you requested/set a Kinect angle that is unavailable";
		return 0;
	}
}


void MSKinectProducer::setBoneEnum(plv::Enum bone)
{
    QMutexLocker lock(m_propertyMutex);
    m_boneEnum = bone;
	m_specifiedbone = m_boneEnum.getSelectedIndex();
	qDebug()<< "set bone to " << m_specifiedbone << "by setting " << m_boneEnum.getSelectedItemName();
    emit boneEnumChanged( bone);
}

plv::Enum MSKinectProducer::getBoneEnum() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_boneEnum;
}

void MSKinectProducer::rotateKinect(int device, int angle)
{
	qDebug() << device;
	//? -1 or 0
	if (false)//(device>0 && device <m_deviceCount)
	{
		qDebug() << "trying to rotate the kinect to the set value";
		//you can't know what the relative angle is as you do not know if it is tilted or not!
		if ( abs(angle-getAngleKinect(device))<56 )
		{
			m_kinects.at(device)->setAngle(angle);
			qDebug() << "new set angle" << getAngleKinect(device);
		}
		else
		{
			qDebug() << "can't rotate that much" << ", the current angle is" << getAngleKinect(device);
		}
	}
	else
	{
		qDebug() << "you requested to rotate a Kinect device that is unavailable";
	}
}
