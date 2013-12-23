#include "FaceTrackCheck.h"; 
#include <QDebug>;

#ifdef SAMPLE_OPTIONS
#include "Options.h"
#else
PVOID _opt = NULL;
#endif

using namespace plv;
using namespace plvmskinect;

FaceTrackCheck::FaceTrackCheck(KinectDevice* kd) :
//FaceTrackCheck::FaceTrackCheck() :
//seems risky to let these pointers be linked to another thread should probably copy the values
    QObject(0),
	m_yourfather(kd),
	m_pFTResult(NULL)
{
	qDebug()<<"init thingy of this facetrack class";
 
	//connect( this, SIGNAL( scheduleSend()), this, SLOT(sendData()), Qt::QueuedConnection );
}



void FaceTrackCheck::start()
{
	qDebug()<< "enter facetracker start";
	m_pFaceTracker = FTCreateFaceTracker(_opt);
	if (!m_pFaceTracker)
	{
	    qDebug() << tr("Could not create the face tracker.\n Face Tracker Initialization Error\n");
		//dont block init on failing initialisation of facetracker, create a seperate method
		//return false;
	}
	else
	{
		qDebug() << tr("Created the ftcreatefacetracker");
	}

	m_LastTrackSucceeded = false;
	FT_CAMERA_CONFIG videoConfig;
	FT_CAMERA_CONFIG depthConfig;
	FT_CAMERA_CONFIG* pDepthConfig = NULL;

	//qDebug() << "videoconfig";
	GetVideoConfiguration(&videoConfig);
    GetDepthConfiguration(&depthConfig);
	m_hint3D[0] = m_hint3D[1] = FT_VECTOR3D(0, 0, 0);
    pDepthConfig = &depthConfig;
		
	//qDebug() << "initializing facetracker";
	HRESULT hr = m_pFaceTracker->Initialize(&videoConfig, pDepthConfig, NULL, NULL); 
	if (FAILED(hr))
	{
		qDebug() << tr("Could not initialize the face tracker.\n Face Tracker Initialization Error\n");
//		return false;
	}
	else
	{
			qDebug() << "initialized the facetracker";
	}

	//qDebug() << "creating ftresult";
	hr = m_pFaceTracker->CreateFTResult(&m_pFTResult);
	if (FAILED(hr) || !m_pFTResult)
	{
		qDebug() << tr("Could not initialize the face tracker result.\n Face Tracker Initialization Error\n");
//       return false;
	}
	else
	{
		qDebug() << "initialized the ft result";
	}

	// Initialize the RGB for FT image.
	m_colorImage = FTCreateImage();
	if (!m_colorImage || FAILED(hr = m_colorImage->Allocate(videoConfig.Width, videoConfig.Height, FTIMAGEFORMAT_UINT8_B8G8R8X8)))
	{
			qDebug() << tr("Could not alllocated with width ") << videoConfig.Width << videoConfig.Height;
		//return 5;
	}
	else
	{
			qDebug() << tr("alllocated with ") << videoConfig.Width << " and height" << videoConfig.Height;
	}


		// Initialize the depth for FT image.
	if (pDepthConfig)
	{
		m_depthImage = FTCreateImage();
		if (!m_depthImage || FAILED(hr = m_depthImage->Allocate(depthConfig.Width, depthConfig.Height, FTIMAGEFORMAT_UINT16_D13P3)))
		{
			//return 6;

			//retrun false
			qDebug() << "could not ftcreate the depthimage";
		}
	}

	SetCenterOfImage(NULL);
	m_LastTrackSucceeded = false;
	m_ApplicationIsRunning = true;

	while (m_ApplicationIsRunning)
    {
        CheckCameraInput();
		Sleep(16);
	}
}

//based on facetracking example
void FaceTrackCheck::CheckCameraInput()
{
	//COPY 

	//qDebug() << "entering camera input";
    HRESULT hrFT = E_FAIL;
	//IFTImage  videoBufferCopy; 
	//m_yourfather->GetVideoBuffer()

    //if ((m_yourfather->getState()==KINECT_RUNNING ||  m_yourfather->getState()==KINECT_INITIALIZED) && m_yourfather->GetVideoBuffer())
	if (m_yourfather->getState()==KINECT_RUNNING ||  m_yourfather->getState()==KINECT_INITIALIZED) 
	{
		HRESULT hrCopy = m_yourfather->GetVideoBuffer()->CopyTo(m_colorImage, NULL, 0, 0);
		//split this test if (SUCCEEDED(hrCopy) && GetDepthBuffer())
		if(SUCCEEDED(hrCopy))
        {
			//m_VideoBuffer = m_yourfather->GetVideoBuffer();
        }
		else
		{
			qDebug() << "FAILED at videobuffer before getting depthbuffer";
		}

		//qDebug() << "get depthbuffer";
		hrCopy = m_yourfather->GetDepthBuffer()->CopyTo(m_depthImage, NULL, 0, 0);
		 // Do face tracking
        if (SUCCEEDED(hrCopy))
        {
			m_DepthBuffer  = m_yourfather->GetDepthBuffer();
			//it works: qDebug() << "start facetracking got videobuffer and got depthbuffer";
            FT_SENSOR_DATA sensorData(m_colorImage, m_depthImage, m_yourfather->GetZoomFactor(), m_yourfather->GetViewOffSet());
			//qDebug() << "zoomfactor " << m_yourfather->GetZoomFactor() << " viewoffset" << m_yourfather->GetViewOffSet();
            FT_VECTOR3D* hint = NULL;
            if (SUCCEEDED(m_yourfather->GetClosestHint(m_hint3D)))
            {
				//qDebug() << "set the hint succesfully";
                hint = m_hint3D;
            }
            
			if (m_LastTrackSucceeded)
            {
				///// the hint is 
				/// <param name="headPoints[2]">
				/// Optional, NULL if not provided. Array that contains two 3D points in camera space if known (for example, from a Kinect skeleton). 
				/// The first element is the neck position and the second element is the head center position. 
				/// The camera space is defined as: right handed, the origin at the camera optical center; Y points up; units are in meters. 
				//qDebug() << "last track did allready succeed";
				hrFT = m_pFaceTracker->ContinueTracking(&sensorData, hint, m_pFTResult);
				//hrFT = m_pFaceTracker->ContinueTracking(&sensorData, NULL, m_pFTResult);
            }
            else
            {
				//qDebug() << "lasttrackdid not succeed";
                hrFT = m_pFaceTracker->StartTracking(&sensorData, NULL, hint, m_pFTResult);
				//hrFT = m_pFaceTracker->StartTracking(&sensorData, NULL, NULL, m_pFTResult);
            }
        }
		else
		{
			qDebug() << "FAILED at getting depthbuffer";
		}

		 m_LastTrackSucceeded = SUCCEEDED(hrFT) && SUCCEEDED(m_pFTResult->GetStatus());
		if (m_LastTrackSucceeded)
		{
			//qDebug() << "last track succeeded after all";
			SubmitFraceTrackingResult(m_pFTResult);
		}
		else
		{
			//qDebug() << "failed with track at end of camerinput";
			m_pFTResult->Reset();
			if (SUCCEEDED(m_pFTResult->GetStatus()))
				qDebug() << "no tracking but a status";
		}
		SetCenterOfImage(m_pFTResult);
	}
	else
	{
		//TODO find a better solution, I couldn't get the kinectthread to stop otherwise, this however will only work if the kinect is initialized when the ft is called for the very first time.
		qDebug() << "kinect not running";
		emit finished();
		//stop();
	}
}


void FaceTrackCheck::stop()
{
   qDebug() << "stop signal for facetrackcheck";
   m_ApplicationIsRunning = false;
   //emit finished();
}

FaceTrackCheck::~FaceTrackCheck()
{
    
}

////from facetracking example
void FaceTrackCheck::setAcknowledgeNeeded(bool acknowledge)
{
	m_acknowledge = acknowledge;
}

////from facetracking example
BOOL FaceTrackCheck::SubmitFraceTrackingResult(IFTResult* pResult)
{
	FLOAT scale;
    FLOAT rotationXYZ[3];
    FLOAT translationXYZ[3];
    pResult->Get3DPose(&scale, rotationXYZ, translationXYZ);
	
	qDebug() << "submiting facetracking results" << "rotation x" << rotationXYZ[0] << " y " << rotationXYZ[1] << " z" << rotationXYZ[2];
    if (pResult != NULL && SUCCEEDED(pResult->GetStatus()))
    {
		//commented out for our purposes
       /* if (m_CallBack)
        {
            (*m_CallBack)(m_CallBackParam);
        }
		*/
       // m_DrawMask = true;

		if (m_DrawMask)
        {
            FLOAT* pSU = NULL;
            UINT numSU;
            BOOL suConverged;
            m_pFaceTracker->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);
            POINT viewOffset = {0, 0};
            FT_CAMERA_CONFIG cameraConfig;

            //we do not end up here if there is no sensor present
			/*if (m_KinectSensorPresent)
            {
                m_KinectSensor.GetVideoConfiguration(&cameraConfig);
            }*/
			//instead check if we get reasonable videoconfig and otherwise set to standard values;
            if( GetVideoConfiguration(&cameraConfig)) 
			{
                //m_KinectSensor.GetVideoConfiguration(&cameraConfig);
            }
            else
            {
                cameraConfig.Width = 640;
                cameraConfig.Height = 480;
                cameraConfig.FocalLength = 500.0f;
            }

			//visualize face model
           // IFTModel* ftModel;
            //HRESULT hr = m_pFaceTracker->GetFaceModel(&ftModel);
            //if (SUCCEEDED(hr))
            //{
                //hr = VisualizeFaceModel(m_colorImage, ftModel, &cameraConfig, pSU, 1.0, viewOffset, pResult, 0x00FFFF00);
                //ftModel->Release();
            //}
        }
    }
    return TRUE;
}

// We compute here the nominal "center of attention" that is used when zooming the presented image.
void FaceTrackCheck::SetCenterOfImage(IFTResult* pResult)
{
    float centerX = ((float)m_colorImage->GetWidth())/2.0f;
    float centerY = ((float)m_colorImage->GetHeight())/2.0f;
    if (pResult)
    {
        if (SUCCEEDED(pResult->GetStatus()))
        {
            RECT faceRect;
            pResult->GetFaceRect(&faceRect);
            centerX = (faceRect.left+faceRect.right)/2.0f;
            centerY = (faceRect.top+faceRect.bottom)/2.0f;
        }
        m_XCenterFace += 0.02f*(centerX-m_XCenterFace);
        m_YCenterFace += 0.02f*(centerY-m_YCenterFace);
    }
    else
    {
        m_XCenterFace = centerX;
        m_YCenterFace = centerY;
    }
}

////from facetracking example
HRESULT FaceTrackCheck::GetVideoConfiguration(FT_CAMERA_CONFIG* videoConfig)
{
	qDebug() << "enterint get video"; 

    if (!videoConfig)
    {
		qDebug() << "novideoconfig return e pointer"; 
        return E_POINTER;
    }
	else
	{
		qDebug() << "there is a videoconfig"; 
	}

	//TODO for now it might be unsafe to use the m_videobuffer at this videoConfig we just assume 640x 480! 
	//TODO perhaps use 
	UINT width = 640;// m_VideoBuffer ? m_VideoBuffer->GetWidth() : 0;
    UINT height = 480;//  m_VideoBuffer ? m_VideoBuffer->GetHeight() : 0;
    FLOAT focalLength = 0.f;

	if(width == 0 && height == 0)
	{
		qDebug() << "video indeed width and height are 0"; 
	}
    else if(width == 640 && height == 480)
    {
		qDebug() << "indeed video width 640 height 480";
        focalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
    }
    else if(width == 1280 && height == 960)
    {
        focalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS * 2.f;
    }

	videoConfig->FocalLength = focalLength;
    videoConfig->Width = width;
    videoConfig->Height = height;

    if(focalLength == 0.f)
    {
		qDebug() << "throw expected unexpected in videoconfig"; 
        return E_UNEXPECTED;
    }

	qDebug() << "videoconfig ok";
        

    return(S_OK);
}
//
HRESULT FaceTrackCheck::GetDepthConfiguration(FT_CAMERA_CONFIG* depthConfig)
{
    if (!depthConfig)
    {
		qDebug() << "there is no depthconfig"; 
        return E_POINTER;
    }
	else
	{
		qDebug() << "there is a depthconfig"; 
	}

	//TODO make this correct assume the set 320 x240 for now
	//320,240
    UINT width = 640;// m_DepthBuffer ? m_DepthBuffer->GetWidth() : 0;
    UINT height = 480; //m_DepthBuffer ? m_DepthBuffer->GetHeight() : 0;
    FLOAT focalLength = 0.f;
	
	if(width == 0 && height == 0)
	{
		qDebug() << "shit width 0 height 0";
	}
	else if(width == 80 && height == 60)
    {
        focalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS / 4.f;
    }
    else if(width == 320 && height == 240)
    {
        focalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
    }
    else if(width == 640 && height == 480)
    {
		qDebug() << "indeed width 640 height 480";
        focalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS * 2.f;
    }

    depthConfig->FocalLength = focalLength;
    depthConfig->Width = width;
    depthConfig->Height = height;

	if(focalLength == 0.f)
    {
		qDebug() << "unexpected 0.f focallength";
        return E_UNEXPECTED;
    }

	qDebug() << "ok depth";

    return S_OK;
}
