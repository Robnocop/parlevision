/**
  * Copyright (C)2011 by Richard Loos
  * All rights reserved.
  *
  * This file is part of the plvmskinect module of ParleVision.
  *
  * ParleVision is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * ParleVision is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * A copy of the GNU General Public License can be found in the root
  * of this software package directory in the file LICENSE.LGPL.
  * If not, see <http://www.gnu.org/licenses/>.
  */

#include "mskinectdevice.h"
#include "NuiSensor.h"
#include "NuiImageCamera.h"
#define PI 3.14159265
#define PINT 31416 //to use as an integer unsigned __int16 : 0 to 65,535 

#include <QDebug>
#include <QMutexLocker>

using namespace plv;
using namespace plvmskinect;
//TODO add newer high resolution for the kinect for windows unit.
//


KinectDevice::KinectDevice(int id, QObject* parent) :
    QThread(parent)
{
    zero();
	//cast to OLECHAR???
	m_id = id;
	m_realcoord = false;
	m_cutxl = 0;
	m_cutxr = 0;
	m_cutyu = 0;
	m_cutyd = 0;
	m_cutz = 0;
	m_maxscalex= 8960;
	m_maxscaley= 6726;
	m_infrared = false;
	m_highres = false;
    connect( this, SIGNAL( finished()), this, SLOT( threadFinished()) );
}

KinectDevice::~KinectDevice()
{
	
}

void KinectDevice::zero()
{
	m_state = KINECT_UNINITIALIZED;

    m_nuiInstance          = NULL;
    m_hNextDepthFrameEvent = NULL;
    m_hNextVideoFrameEvent = NULL;
    m_hNextSkeletonEvent   = NULL;
    m_pDepthStreamHandle   = NULL;
	//full depth
	m_pDepthStreamHandle2 = NULL;
    m_pVideoStreamHandle   = NULL;
 //   m_hThNuiProcess        = NULL;
    //m_hEvNuiProcessStop    = NULL;

    //ZeroMemory(m_pen, sizeof(m_pen));
    //m_SkeletonDC           = NULL;
    //m_SkeletonBMP          = NULL;
    //m_SkeletonOldObj       = NULL;
    //m_PensTotal            = 6;
    //ZeroMemory(m_Points,sizeof(m_Points));

    //m_LastSkeletonFoundTime = -1;
    //m_bScreenBlanked        = false;
    //m_FramesTotal           = 0;
    //m_LastFPStime           = -1;
    //m_LastFramesTotal       = 0;
}

//in case of multiple kinects mskinectproducer will create multiple devices by running it multiple times
// use m_kinects, a QVector with kinectdevices, to retrieve a specific device. But be carefull the vector might be empty

bool KinectDevice::init()
{
	// kinect must not already be initialized
	assert( getState() == KINECT_UNINITIALIZED );
    
	QMutexLocker lock( &m_deviceMutex );
    
    // initialize all variables except ID to default values
    zero();

    HRESULT hr;

    //hr = MSR_NuiCreateInstanceByIndex( m_id, &m_nuiInstance );
	//new use a callbacck (stop has a bug) NuiCreateSensorByIndex
	//need to implement the callback method, probably because the destroy instance is not available anymore:
	//see: http://www.microsoft.com/en-us/kinectforwindows/develop/release-notes.aspx#_6._known_issues 
	//and http://social.msdn.microsoft.com/Forums/en-US/kinectsdknuiapi/thread/0c62b444-bf05-4700-a1e7-a9b3a1a2dcec
	//I needed it to run outside KinectDevice otherwise
//	NuiSetDeviceStatusCallback((NuiStatusProc)&KinectStatusProc, NULL);
	hr = NuiCreateSensorByIndex( m_id, &m_nuiInstance );
    if( FAILED( hr ) )
    {
        qDebug() << tr("Kinect device with id %1 failed MSR_NuiCreateInstanceByIndex.").arg(m_id);
        return false;
		//return hr used in nuiimpl
    }

	//for current Kinect declared in header
	//we changed name of nuisensor to nuiinstance
	//used in nui_statusproc in the producer claas that deals with status changes

	//    INuiSensor *            m_pNuiSensor;
	//    BSTR                    m_instanceId;
	//SysFreeString(m_instanceId);
	//m_instanceId = m_nuiInstance->NuiDeviceConnectionId();

    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextVideoFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextSkeletonEvent   = CreateEvent( NULL, TRUE, FALSE, NULL );
   // m_hEvNuiProcessStop    = CreateEvent( NULL, FALSE,FALSE, NULL);

	//    hr = NuiInitialize( NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX |
	//                        NUI_INITIALIZE_FLAG_USES_SKELETON |
	//                        NUI_INITIALIZE_FLAG_USES_COLOR );

    //hr = m_nuiinstance->nuiinitialize( nui_initialize_flag_uses_depth
    //                    // | nui_initialize_flag_uses_skeleton
    //                     | nui_initialize_flag_uses_color

	DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH |  NUI_INITIALIZE_FLAG_USES_COLOR;
	//DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
    hr = m_nuiInstance->NuiInitialize( nuiFlags);
	//old fail protocol
	
    //if( FAILED( hr ) )
    //{
    //    //MessageBoxResource(m_hWnd,IDS_ERROR_NUIINIT,MB_OK | MB_ICONHAND);
    //    qDebug() << tr("Kinect device with id %1 failed NuiInitialize with return handle %2.").arg(m_id).arg(hr);
    //    return false;
    //}
	
   // hr = m_pNuiSensor->NuiInitialize( nuiFlags );
    if ( E_NUI_SKELETAL_ENGINE_BUSY == hr )
    {
        qDebug()<<"error in skeletal engine bussy";
		nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH |  NUI_INITIALIZE_FLAG_USES_COLOR;
        hr = m_nuiInstance->NuiInitialize( nuiFlags) ;
    }
  
    if ( FAILED( hr ) )
    {
        if ( E_NUI_DEVICE_IN_USE == hr )
        {
            //MessageBoxResource( IDS_ERROR_IN_USE, MB_OK | MB_ICONHAND );
			qDebug()<<"error NUI in use";
        }
        else
        {
            //MessageBoxResource( IDS_ERROR_NUIINIT, MB_OK | MB_ICONHAND );
			qDebug()<<"error somewhere else in init";
        }
        return hr;
    }

    //hr = m_nuiInstance->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
	//turned of skeleton shit hr =  m_nuiInstance->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
    //if( FAILED( hr ) )
    //{
    //    //MessageBoxResource(m_hWnd,IDS_ERROR_SKELETONTRACKING,MB_OK | MB_ICONHAND);
    //    qDebug() << tr("Failed to open skeleton stream on Kinect with id %1 and with return handle %2.").arg(m_id).arg(hr);
    //    return false;
    //}

	//NUI_IMAGE_TYPE_COLOR_INFRARED
	//NUI_IMAGE_TYPE_COLOR
	if(m_infrared)
    {
		hr = m_nuiInstance->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_COLOR_INFRARED,
			NUI_IMAGE_RESOLUTION_640x480,
			0,
			2,
			m_hNextVideoFrameEvent,
			&m_pVideoStreamHandle );
	}
	else
	{
		if(m_highres)
		{
			hr = m_nuiInstance->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_COLOR,
			NUI_IMAGE_RESOLUTION_1280x960, //
			0,
			2,
			m_hNextVideoFrameEvent,
			&m_pVideoStreamHandle );
		}
		else
		{
			hr = m_nuiInstance->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_COLOR,
			NUI_IMAGE_RESOLUTION_640x480, 
			0,
			2,
			m_hNextVideoFrameEvent,
			&m_pVideoStreamHandle );
		}
	}
	if( FAILED( hr ) )
    {
        //MessageBoxResource(m_hWnd,IDS_ERROR_VIDEOSTREAM,MB_OK | MB_ICONHAND);
        qDebug() << tr("Failed to open video stream on Kinect with id %1.").arg(m_id);
        return false;
    }

//    hr = NuiImageStreamOpen(
//        NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
//        NUI_IMAGE_RESOLUTION_320x240,
//        0,
//        2,
//        m_hNextDepthFrameEvent,
//        &m_pDepthStreamHandle );
//	OLD
	
    hr = m_nuiInstance->NuiImageStreamOpen(
        NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_640x480, //not for depth http://msdn.microsoft.com/en-us/library/microsoft.kinect.depthimageformat.aspx NUI_IMAGE_RESOLUTION_1280x960
        0,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );
		//Might be neccesary but was also used in beta sdk example
  /*  hr = m_nuiInstance->NuiImageStreamOpen(
        HasSkeletalEngine(m_nuiInstance) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_320x240,
        0,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );
	*/
	if( FAILED( hr ) )
    {
        //MessageBoxResource(m_hWnd,IDS_ERROR_DEPTHSTREAM,MB_OK | MB_ICONHAND);
        qDebug() << tr("Failed to open depth stream on Kinect with id %1.").arg(m_id);
        return false;
    }
	//else 
	////include sentinel values too near too far: http://social.msdn.microsoft.com/Forums/en-US/kinectsdknuiapi/thread/3fe21ce5-4b75-4b31-b73d-2ff48adfdf52/
	//// Too near: 0x0000 Too far: 0x7ff8 [32760 = 2^15-2^8 (^2=65520)] Unknown: 0xfff8 [65528 = 2^16-2^3). does not work!
	//{
	//	//from some code http://forum.libcinder.org/topic/kinect-sdk-block mSensor->NuiImageStreamSetImageFrameFlags(mDepthStreamHandle,NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE | NUI_IMAGE_STREAM_FLAG_DISTINCT_OVERFLOW_DEPTH_VALUES);
	//	m_nuiInstance->NuiImageStreamSetImageFrameFlags(&m_pDepthStreamHandle,NUI_IMAGE_STREAM_FLAG_DISTINCT_OVERFLOW_DEPTH_VALUES);
	//}

	// In the skeletalviewer example the following lines will start the Nui processing thread 
	// We did and will not do this here as it should be handled by Qt efficiently
    //m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
   // m_hThNuiProcess = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );

	//TODO
	//m_angleKinect1 = 0;

	setState( KINECT_INITIALIZED );
	
    return true;
}

bool KinectDevice::deinit()
{
	//in the nuiimpl they also delete other skeleton stuff
	//reodered part of deinit to bottom of deinit
	//in the nuiimpl they do not use compare to NULL
	//in the old nuiimpl neither, only in the parlevision v

	if ( m_nuiInstance != NULL)
	{
		m_nuiInstance->NuiShutdown( );
	}
	else
    {
		qDebug() << "nui instance was null on shutdown!";
    }

	if( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }
    if( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }
    if( m_hNextVideoFrameEvent && ( m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextVideoFrameEvent );
        m_hNextVideoFrameEvent = NULL;
    }
    
	//second part, was split into two as in the new nuiimp but added compare to NULL
	if( m_nuiInstance != NULL )
    {
       	m_nuiInstance->Release();
        m_nuiInstance = NULL;
        //old kinectplugin had a destroy instance this is not available in new SDK thus needing thecallback funtion
		//MSR_NuiDestroyInstance( m_nuiInstance );
		//NuiSetDeviceStatusCallback(0, NULL);
		//qDebug() << "finish release and null kinect";
	}
	else
    {
		qDebug() << "nui instance is null on release";
    }
		
	setState( KINECT_UNINITIALIZED );

    return true;
}

int KinectDevice::getId() const
{
	return m_id;
}

int KinectDevice::width() const
{
    return 0;
}

int KinectDevice::height() const
{
    return 0;
}

void KinectDevice::start()
{
	QMutexLocker lock( &m_deviceMutex );
	switch( getState() )
    {
    case KINECT_UNINITIALIZED:
        // TODO throw exception?
		
        break;
    case KINECT_INITIALIZED:
        // Start thread
		
		QThread::start();
		break;
    case KINECT_RUNNING:
		
    default:
        // Do nothing
        return;
    }
}

void KinectDevice::stop()
{
	QMutexLocker lock( &m_deviceMutex );

    switch( getState() )
    {
    case KINECT_RUNNING:
        // Stop the Nui processing thread
        // Signal the thread
		qDebug() << "i will stop the run and set case to stop request";
        setState( KINECT_STOP_REQUESTED );
        // switching m_state to KINECT_STOP_REQUESTED
        // will cause the run loop to exit eventually.
        // wait for that here.
        QThread::wait();
        //fallthrough
    case KINECT_INITIALIZED:
        // the thread is not running, so it's safe to release the capture device.
        // fallthrough
    case KINECT_UNINITIALIZED:
    case KINECT_STOP_REQUESTED:
        return;
    }
}

void KinectDevice::run()
{
	setState(KINECT_RUNNING);
    	
    // Configure events to be listened on
    HANDLE hEvents[3];
    hEvents[0] = this->m_hNextDepthFrameEvent;
    hEvents[1] = this->m_hNextVideoFrameEvent;
    hEvents[2] = this->m_hNextSkeletonEvent;

    // Main thread loop
    while(true)
    {
        // Wait for an event to be signalled
       	int nEventIdx = WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]),hEvents,FALSE,100);

        // If the stop event, stop looping and exit
        //if( getState() == KINECT_STOP_REQUESTED )
        //    break;

        if( getState() == KINECT_STOP_REQUESTED )
        { 
			qDebug() << "kinectstop requested";			
			break;
		}


		// Perform FPS processing
//        t = timeGetTime( );
//        if( this->m_LastFPStime == -1 )
//        {
//            this->m_LastFPStime = t;
//            this->m_LastFramesTotal = this->m_FramesTotal;
//        }
//        dt = t - this->m_LastFPStime;
//        if( dt > 1000 )
//        {
//            this->m_LastFPStime = t;
//            int FrameDelta = this->m_FramesTotal - this->m_LastFramesTotal;
//            this->m_LastFramesTotal = this->m_FramesTotal;
//            SetDlgItemInt( this->m_hWnd, IDC_FPS, FrameDelta,FALSE );
//        }

        // Perform skeletal panel blanking
//        if( this->m_LastSkeletonFoundTime == -1 )
//            this->m_LastSkeletonFoundTime = t;
//        dt = t - this->m_LastSkeletonFoundTime;
//        if( dt > 250 )
//        {
//            if( !this->m_bScreenBlanked )
//            {
//                this->Nui_BlankSkeletonScreen( GetDlgItem( this->m_hWnd, IDC_SKELETALVIEW ) );
//                this->m_bScreenBlanked = true;
//            }
//        }

        // Process signal events
        switch(nEventIdx)
        {
            case 0:
                this->Nui_GotDepthAlert();
                //this->m_FramesTotal++;
                break;

            case 1:
                this->Nui_GotVideoAlert();
                break;

            case 2:
                this->Nui_GotSkeletonAlert();
                break;
        }
    }
	//apparently is set to unitialised at another point.
	setState( KINECT_INITIALIZED );
}

void KinectDevice::Nui_GotDepthAlert()
{
    //changed old const NUI_IMAGE_FRAME * pImageFrame = NULL;
	//NUI_IMAGE_FRAME pImageFrame;
	//for full depth not const
	//const NUI_IMAGE_FRAME * pImageFrame = NULL;
	NUI_IMAGE_FRAME pImageFrame;
		 
	HRESULT hr = m_nuiInstance->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &pImageFrame);
	//get full depth
	//NUI_DEPTH_IMAGE_PIXEL *Iout;
	BOOL bNearMode = false;
	INuiFrameTexture * pTexture = NULL;
	hr = m_nuiInstance->NuiImageFrameGetDepthImagePixelFrameTexture(m_pDepthStreamHandle, &pImageFrame, &bNearMode, &pTexture);
	if( FAILED( hr ) )
    {
        return;
    }

	//no need to check, if( pImageFrame->eResolution == NUI_IMAGE_RESOLUTION_320x240 ) etc. Already set this flag to this resolution earlier on.
	int width = 640;
    int height= 480;

    //old SDK:
	//NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
    //KINECT_LOCKED_RECT LockedRect;
	
	//removed for full depth:
	//INuiFrameTexture *  pTexture = pImageFrame->pFrameTexture;

	NUI_LOCKED_RECT LockedRect;
   
	//full depth
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    //instead of 
	//BYTE* pBuffer = 0;
    
	 if( LockedRect.Pitch != 0 )
	 {
		NUI_DEPTH_IMAGE_PIXEL * pBuffer =  (NUI_DEPTH_IMAGE_PIXEL *) LockedRect.pBits;
		INuiCoordinateMapper* pMapper;
		m_nuiInstance->NuiGetCoordinateMapper(&pMapper);
		//check if int maxval < 307200 (640*480)
		int j = 0;
	
		//USHORT* pBuffer =  (USHORT*) LockedRect.pBits;
		//pBuffer = (BYTE*) LockedRect.pBits;
 
		CvMatData img;
		img = CvMatData::create(width, height, CV_16U, 1);
		//If your application included NUI_INITIALIZE_FLAG_USES_DEPTH in the dwFlags argument to NuiInitialize, depth data is returned as a 16-bit value in which the low-order 12 bits (bits 0–11) contain the depth value in millimeters.
		//might be remark of the old nui changed from beta to 1.0
		
		//full high depth
		if(pImageFrame.eImageType == NUI_IMAGE_TYPE_DEPTH )
		{
			// todo should be faster with memcpy something with 8-bit description

			//full depth is also 16 bit but in a struct [depth & playerindex]
			//img = CvMatData::create(width, height, CV_16U, 1);
		
			// draw the bits to the bitmap needs to be casted to unsigned shorts as sdk gives a signed short by default
			//USHORT* pBufferRun = (USHORT*) pBuffer;
			cv::Mat& mat = img;
			bool realcoordtemp= getRealWorldCoord(); 
			int cxl = getCutXL();
			int cxr = getCutXR();
			int cyu = getCutYU();
			int cyd = getCutYD();
			int cz = getCutZ();
			//check if values are properly transmitted qDebug() << "x " <<cx << "y " << cy << "z "<<  cz; 

			//int realx;
			//int realy;
			int realz; //? is this helping needed for the bithshift by one to meters
			//Vector4 realPoints;
			//Vector4 realPointsKinect;
			Vector4 realPointsKinect2;
			NUI_DEPTH_IMAGE_POINT test;
			//floating point /single-value notation is unnecesary
			
			//old sdk: 
			//faq gives sthe answer >>3 to get depth, actually upto 12 correct bits and max of 12 bits but given in 13bits and  in mm so I don't alter this here?
			//Too near: 0x0000
			//Too far: 0x7ff8
			//Unknown: 0xfff8
			//unsigned short maxValue = 0;
			//unsigned short minValue = 15000; //should be 2^13 8192, unshifted it can become 31800 (<< 3 = 3975) thus bitshift once to create a better viewable image.
			////maxvalue will then be around 62000
			//http://www.i-programmer.info/ebooks/practical-windows-kinect-in-c/3802-using-the-kinect-depth-sensor.html?start=1
				
			//defined: ushort 0 - 65535
			//int max = 32767, uint=65535, schar=127, uchar= 255, although std http://msdn.microsoft.com/en-us/library/s086ab1z(v=vs.71).aspx
        
			////to check real space dependencies
			//float minx = 15000.0f;
			//float maxx = 0.0f;
			//float miny = 15000.0f;
			//float maxy = 0.0f;
			//float minz = 15000.0f;
			//float maxz = 0.0f;

			// todo should be faster with memcpy
			if (realcoordtemp)
			{
				//for always gettig the closest point it is wise to start with x,y 0 and loop the two ways, it is probably faster to draw over than it is to check whether the point has been drawn and is closer.
				for( int y = height/2 ; y < height ; y++ )
				{
					for( int x = width/2 ; x < width ; x++ )	
					{
						//I(robby) did the following for new sdk 
						//the number of bits has been changed (and probably significant/unsignificant bits changed?) showing 15bit max , accroding to doc still the lower 12 bits are used, but faq says higher 13-bits and typo of doc
						//anyway to convert to m use >> 4 in rest of program, which throws away the lower values which should not exist due to minimum range. 
						//optimalisation could be to use an 8 bit image (representing values 2^8-2^16) and if transformation to meters has to be done
						//to show it we use more clearly in viewer we use << 1.
						
						//TODO DANGEROUS functions one of the two results in crash, switched x and y value.
						//TODO  calculate values by own function, the realpoint values are probably influenced by the floor cut-off plane which will be inconsistent for the kinects
						//realPoints = NuiTransformDepthImageToSkeleton(y,x,pBuffer[j].depth);
						//realz = pBuffer[j].depth>>1;
						//assume it is actual depth in mm
						//j = y*width+x
						//realz = pBuffer[j].depth>>1;
						realz = pBuffer[y*width+x].depth; 
						//based on emperical results with cutz value a cz would cut image after 2m 
						if ((realz > 0) && !(((realz) > cz) && (cz > 0)) )
						{
							//KINECT realpoint transformation just give it up!
							//maybe we should transorm by 3 instead of two so assuming depth is allready in mm
							//using the non bitshifted function for the other method: NOT >>3, NOT >>2, not <<3, 
							/*realPointsKinect = NuiTransformDepthImageToSkeleton(x,y,pBuffer[j].depth<<2);
							realPointsKinect.x = realPointsKinect.x *1000;
							realPointsKinect.y = realPointsKinect.y *1000;*/

							//http://msdn.microsoft.com/en-us/library/nuisensor.inuicoordinatemapper.mapdepthpointtoskeletonpoint.aspx
							//this actually works and gives same results as own functions but with y flipped
							
							test.x = x;
							test.y = y;
							test.depth = pBuffer[y*width+x].depth; //commentout the bitshift >>1
							// INuiCoordinateMapper* pMapper;
							// m_nuiInstance->NuiGetCoordinateMapper(&pMapper);
							pMapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, &test, &realPointsKinect2);

							//if (pBuffer[j].depth == 0)
							if (pBuffer[y*width+x].depth == 0)
							{ 
								//debug: check if if-statement is behaving as it should
								qDebug() << "WTF @ " << x << " " << y;
							}

							// to check if we can loop from center outward
							//if (j != y*width+x) qDebug() << "j is " << j << "x is" << x << "y is" << y;
							//j max is 307199
							
							//optimize the cutoff!
							//TODO if realx is beyond point dont calc y, remove check from frontal, if y beyond cy dont do frontal.
							//realPoints = TransformationToRealworldEucledianPoints(x,y,realz); //apperantly bitshift by one (smaller) to get mm
							//realx = TransformationToRealworldEucledianPointX(x,realz);
							//realy = TransformationToRealworldEucledianPointY(y,realz);
							
							//debug the values
							/*if (x>600 && x < 620 && y< 330 && y>310 )
							{
								qDebug() << "check x" << realPoints.x << "kinectx " << realPointsKinect.x << "mapped" << realPointsKinect2.x *1000 << "int x" << realx; 
								qDebug() << "check y" << realPoints.y << "kinecty " << realPointsKinect.y << "mapped" << realPointsKinect2.y *1000 << "int y" << realy << "z "<< realz; 
							}*/
							
							FrontalImage(mat, realPointsKinect2, pBuffer[y*width+x].depth, cxl, cxr, cyu, cyd);
						}
					
						/*if (realPoints.x < minx)
						{
							minx = realPoints.x;
						}
						if (realPoints.x > maxx)
						{
							maxx = realPoints.x;
						}
						if (realPoints.y > maxy)
						{
							maxy = realPoints.y;
						}
						if (realPoints.y < miny)
						{
							miny = realPoints.y;
						}
						if (realPoints.z < minz && realPoints.z > 0)
						{
							minz = realPoints.z;
						}
						if (realPoints.z > maxz)
						{
							maxz = realPoints.z;
						}*/
						
						//j++;

						//pre-full depth:
						//realPoints = NuiTransformDepthImageToSkeleton(y,x,(*pBufferRun));
						//FrontalImage(mat, realPoints, *pBufferRun); 
						//pBuffer++;

						//mat.at<USHORT>(y,x)  = Nui_ShortToIntensity(*pBufferRun);
						//pBufferRun++;
					}
					for( int x = width/2 ; x > 0; x-- )	
					{
						//neater to make a function of it as it has to be done 4 times
						realz = pBuffer[y*width+x].depth;
						if ((realz > 0) && !(((realz) > cz) && (cz > 0)) )
						{
							test.x = x;
							test.y = y;
							test.depth = pBuffer[y*width+x].depth;
							pMapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, &test, &realPointsKinect2);
							FrontalImage(mat, realPointsKinect2, pBuffer[y*width+x].depth, cxl, cxr, cyu, cyd);
						}
						//j++;
					}
				}
				for( int y = height/2 ; y>0 ; y-- )
				{

					for( int x = width/2 ; x<width; x++ )	
					{
						//neater to make a function of it as it has to be done 4 times
						realz = pBuffer[y*width+x].depth;
						if ((realz > 0) && !(((realz) > cz) && (cz > 0)) )
						{
							test.x = x;
							test.y = y;
							test.depth = pBuffer[y*width+x].depth;
							pMapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, &test, &realPointsKinect2);
							FrontalImage(mat, realPointsKinect2, pBuffer[y*width+x].depth, cxl, cxr, cyu, cyd);
						}
						//j++;
					}

					for( int x = width/2 ; x > 0; x-- )	
					{
						//neater to make a function of it as it has to be done 4 times
						realz = pBuffer[y*width+x].depth;
						if ((realz > 0) && !(((realz) > cz) && (cz > 0)) )
						{
							test.x = x;
							test.y = y;
							test.depth = pBuffer[y*width+x].depth;
							pMapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, &test, &realPointsKinect2);
							FrontalImage(mat, realPointsKinect2, pBuffer[y*width+x].depth, cxl, cxr, cyu, cyd);
						}
						//j++;
					}
				}
			} //end if realcoord
			else
			{
				for( int y = 0 ; y < height ; y++ )
				{
					for( int x = 0 ; x < width ; x++ )	
					{
						//bitshift: << bigger , >>smaller
						//full depth
						//USHORT* pBufferRun = (USHORT*) pBuffer;
						
						//mat.at<USHORT>(y,x) = (*pBufferRun) << 1;
						//TODO check if the 16-bit depth isn't actually in meters, then probably only first 13 bits are used and one should bitshift by 5 to make it viewable
						//proabaly/it seems stull bitshift by 3 for viewing by << 1 for meters
						mat.at<USHORT>(y,x) = pBuffer[j].depth << 3;
						j++;
						//pBufferRun++;
						//pBuffer++;
						//mat.at<USHORT>(y,x) = (*pBufferRun) << 1;
					}
				}
			}
			
			/*if (realcoordtemp)
			{
				qDebug() << tr("minx is %1 maxx %2 min y %3 max y %4 min z %5 max z%6").arg(minx).arg(maxx).arg(miny).arg(maxy).arg(minz).arg(maxz);
			}*/

			//if not bishifted:  "maxvalue is 31800 ...0"  qdebug 2^15=32768 what happend to the remaining 968? 2^10=1024

			// the last 8 pixels are black.
			// From http://groups.google.com/group/openkinect/browse_thread/thread/6539281cf451ae9e
			// Turns out the scaled down raw IR image that we can stream from the
			// Kinect is 640x488, so it loses 8 pixels in both the X and Y dimensions.
			// We just don't see the lost Y because the image is truncated there, while
			// the missing X pixels are padded.

			// The actual raw IR acquisition image is likely 1280x976 (from a 1280x1024
			// sensor, windowing off the extra Y pixels), and from that it derives a
			// 632x480 depth map at 1:2 ratio and using 16 extra source pixels in X and Y. 
		} //end of if statement: pImageFrame.eImageType == NUI_IMAGE_TYPE_DEPTH 
		//If you included NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX in the dwFlags argument to NuiInitialize and did not use NuiImageFrameGetDepthImagePixelFrameTexture to get all depth data, then, depth data is returned as a 16-bit value that contains the following information:
		//The low-order three bits (bits 0–2) contain the skeleton (player) ID.
		//The high-order bits (bits 3–15) contain the depth value in millimeters. A depth data value of zero indicates that no depth data is available at that position because all of the objects were either too close to the camera or too far away from it.
		//this is different for the beta sdk
		else if( pImageFrame.eImageType == NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX )
		{
			qDebug() << "entered IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX "; //this is exceptional not intended to happen in the current version
			img = CvMatData::create(width, height, CV_8U, 3);

			// draw the bits to the bitmap
			USHORT* pBufferRun = (USHORT*) pBuffer;
			cv::Mat& mat = img;

			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
				{
					RGBQUAD quad = Nui_ShortToQuad_DepthAndPlayerIndex( *pBufferRun );

					pBufferRun++;
					mat.at<cv::Vec3b>(y,x)[0] = quad.rgbBlue;
					mat.at<cv::Vec3b>(y,x)[1] = quad.rgbGreen;
					mat.at<cv::Vec3b>(y,x)[2] = quad.rgbRed;
				}
			}
		}
		emit newDepthFrame( m_id, img );
	} //end of if LockedRect.Pitch != 0
	else 
	{
		OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
		return;
	}
	//changed back to old SDK way:
	m_nuiInstance->NuiImageStreamReleaseFrame( m_pDepthStreamHandle, &pImageFrame);
	//NuiImageStreamReleaseFrame( m_pDepthStreamHandle, pImageFrame );
}

void KinectDevice::setMaxScale(int x, int y)
{
	m_maxscalex = x;
	m_maxscaley = y;
	qDebug() << "set scale in device" << m_maxscalex << "y is " <<m_maxscaley;
}


//z-depth is planar distance so always zreal in eucleudian distances,
//fov vertical is 45.6 and 58.5 horizontal for depth according to API constant http://msdn.microsoft.com/en-us/library/hh855368
//half is 29.25 22.8
//as maximum depth is 8m or 8000mm half of the field recognised for y=tan(45.6/2)*8000=3362.890 by x=tan(58.5/2)*8000=4480.215, 6725.781 x 8960.431 mm
//the angle made to the lens essential for calculating the realpoint, assume a non-distorted lens/depth channel or at least a linear distrubution of angles over pixels, and take positive and negative pixel values according to center, so minus maxvalue/2 
//angle from camera recognised by pixels --> tan(alpha)= (pixelvalue-maxpixelvalue)/(maxpixelvalue) * (fov)/2 = (p/m - 1) * fov/2 = fov *p /m /2 - fov/2 
//keep into account premature rounding of integers
//pixelvalue x 0-639
//pixelvalue y 0-479
//maxpixelvalue-x =320 
//maxpixelvalue-y =240 
// Xreal = zreal* tan(alpha)
//--> Xreal = zreal(in mm) * tan ( (xpixel-maxpixelvaluex) * 2925/32000 ) 
// will changed in to floats anyway
//TODO optimalise use an array of ints or vector with ints, instead of floats
//tan is in radian so alpha*PI/180

//optimalisation make an x and an y make it ints and make a check inbetween this reduces the need for calculation of y if x is outside range and vice versa.
Vector4 KinectDevice::TransformationToRealworldEucledianPoints(int x,int y, USHORT z)
{
	Vector4 RealworldEucladianPoint; //floats so
	//int maxpixelvaluex = 320;
	//int maxpixelvaluey = 240;
	//todo check what hppens due to 639/2 vs 640/2
	RealworldEucladianPoint.x= (float) z* tan(PI*(x-320.0f)/180 * 2925/32000);
	//RealworldEucladianPoint.x= (float) z* tan((x-320.0f)*.0015953400194);
	RealworldEucladianPoint.y= (float) z* tan(PI*(y-240.0f)/180 * 228/2400);
	//RealworldEucladianPoint.y= (float) z* tan((y-240.0f)*.0012435470925);
	//not used
	//RealworldEucladianPoint.z= (float) z; //probably bitshift
	return RealworldEucladianPoint;
}

int KinectDevice::TransformationToRealworldEucledianPointX(int x, USHORT z)
{
	x = (int) z* tan(PI*(x-320)/180 * 2925/32000);
	return x;
}

int KinectDevice::TransformationToRealworldEucledianPointY(int y, USHORT z)
{
	y = (int) z* tan(PI*(y-240)/180 * 228/2400);
	return y;
}


//need to make this function absolute by "painitng the bitmap from the inside outward
//odd cutxl and cutxt seem to be switched. 
//TODO x is also mirrored so simplest solution is to switch the values here for the time being
//int cutxr, int cutxl
void KinectDevice::FrontalImage(cv::Mat& projection, Vector4 realWorldCoord, USHORT bufferpoint, int cutxr, int cutxl, int cutyu, int cutyd)
//void KinectDevice::FrontalImage(cv::Mat& projection, Vector4 realWorldCoord)
{
	//is a coordinate mapper faster: http://msdn.microsoft.com/en-us/library/nuisensor.nuicreatecoordinatemapperfromparameters.aspx
	// http://msdn.microsoft.com/en-us/library/nuisensor.inuicoordinatemapper.mapdepthframetoskeletonframe.aspx
	//http://msdn.microsoft.com/en-us/library/hh973078.aspx#Converting
	// yes it is!!!

	//forum how to use skeletonpoints http://social.msdn.microsoft.com/Forums/en-US/kinectsdk/thread/99b0aa9e-3e8a-41bf-8014-b90e00b8c0ea
	//some sort of platform http://code.google.com/p/ubidisplays/source/browse/trunk/src/UbiDisplays/bin/Debug/Microsoft.Kinect.xml?r=3

	//half values so 8m: 4480 & 3362
	//float maxx = 4480.215;
	//float maxy = 3362.89;
	//for ease of proigramming keep it as meters from left and right
	/*float actualcutxl = 2240.17f;
	float actualcutxr = 2240.17f;
	float actualcutyu = 1681.45f;
	float actualcutyd = 1681.45f;*/
	unsigned short actualcutxl = 4480;
	unsigned short actualcutxr = 4480;
	unsigned short actualcutyu = 3363;
	unsigned short actualcutyd = 3363;
	//let the cutoff be in mm as well
	
	if (cutxl>0)
	{
		actualcutxl = cutxl;
	}
	if (cutxr>0)
	{
		actualcutxr = cutxr;
	}
	if (cutyu>0)
	{
		actualcutyu = cutyu;
	}
	if (cutyd>0)
	{
		actualcutyd = cutyd;
	}
	
	bufferpoint = bufferpoint << 3;
	//cutz = cutz << 4; //one bitshift smaller makes m so three makes it maximum visibility so 4 to get from depth to 

	//temp solution
	realWorldCoord.x  = realWorldCoord.x*1000;
	realWorldCoord.y  = realWorldCoord.y*1000;
	
	//the x,y cut offs are variable
	//int y= (int) ((realWorldCoord.y+ 1.78f)* (480/3.56f));
	if (bufferpoint >0 && 
		!(realWorldCoord.x<0 && ((0-cutxl)>(realWorldCoord.x)  && cutxl>0)) && 
		!(realWorldCoord.x>0 && (cutxr < (realWorldCoord.x)    && cutxr>0)) && 
		!((-1* realWorldCoord.y >0) && cutyu <realWorldCoord.y && cutyu >0)  && 
		!((-1* realWorldCoord.y <0) && ((0-cutyd) > realWorldCoord.y) && cutyd >0) )
		// allready incorrect: !(cutx<realWorldCoord.x && cutx>0) && !(cuty<realWorldCoord.y && cuty>0))
	{
		//checked before entering this method
		//if (cutz == 0 || ((cutz>0) && (bufferpoint<cutz)) )
		//{
			//int y= (int) ((realWorldCoord.y+ maxy)* 240/maxy);
			int y= (int) ((realWorldCoord.y+ actualcutyd)* 480/(getMaxScaleY())); //actualcutyu+actualcutyd
			if (y>-1 && y<480)
			{
				//int x= (int) ((realWorldCoord.x+ 2.2f)* (640/4.4f));
				int x= (int) ((realWorldCoord.x+ actualcutxl)* 640/(getMaxScaleX()));
				if (x>-1 && x<640)
				{
					//flipped???? why does the SDK continu to do this, strange!
					//full depth if (bufferpoint < projection.at<unsigned short>(y,x) // why do you need this second part || projection.at<unsigned short>(y,x)<40)
					
					//no longer neccesary
					//if ( ( (bufferpoint) < projection.at<unsigned short>(479-y,x)) || projection.at<unsigned short>(479-y,x)<40) 
					//{
						//bufferpoint is not sure whether it will maintain within USHORT range
						//projection.at<USHORT>((479-y),x) = (USHORT)(bufferpoint) << 3;
						//x is also mirrored
						projection.at<USHORT>(479-y,639-x) = (bufferpoint);
					//}
				}
			}
		//}
	}
}

//TEMP solution 
//BYTE KinectDevice::Nui_ShortToIntensity( USHORT s )
//{
//    USHORT RealDepth = NuiDepthPixelToDepth(s);
//    USHORT Player    = NuiDepthPixelToPlayerIndex(s);
//
//    // transform 13-bit depth information into an 8-bit intensity appropriate
//    // for display (we disregard information in most significant bit)
//    BYTE intensity = (BYTE)~(RealDepth >> 4);
//
//    // tint the intensity by dividing by per-player values
//  //  RGBQUAD color;
// //   color.rgbRed   = intensity >> g_IntensityShiftByPlayerR[Player];
// //   color.rgbGreen = intensity >> g_IntensityShiftByPlayerG[Player];
// //   color.rgbBlue  = intensity >> g_IntensityShiftByPlayerB[Player];
//
//    return intensity;
//}


void KinectDevice::Nui_GotVideoAlert()
{
	const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame( m_pVideoStreamHandle, 0, &pImageFrame );
    if( FAILED( hr ) )
    {
        return;
    }

    //old NuiImageBuffer* pTexture = pImageFrame->pFrameTexture;
	//new??
	INuiFrameTexture*  pTexture = pImageFrame->pFrameTexture;
    //old KINECT_LOCKED_RECT LockedRect;
	NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 )
    {
		BYTE * pBuffer = (BYTE*) LockedRect.pBits;
		if(!m_infrared)
		{
			// draw the bits to the bitmap
			RGBQUAD* pBufferRun = (RGBQUAD*) pBuffer;
			// new res!! 640x480
			plv::CvMatData img = plv::CvMatData::create(640, 480, CV_8UC3);
			if(m_highres)
			{
				img = plv::CvMatData::create( 1280, 960, CV_8UC3 );
			}
			cv::Mat& mat = img;

			if (m_highres)
			{
				for( int y = 0 ; y < 960 ; y++ )
				{
					for( int x = 0 ; x < 1280 ; x++ )
					{
						RGBQUAD quad = *pBufferRun;
						pBufferRun++;
						mat.at<cv::Vec3b>(y,x)[0] = quad.rgbBlue;
						mat.at<cv::Vec3b>(y,x)[1] = quad.rgbGreen;
						mat.at<cv::Vec3b>(y,x)[2] = quad.rgbRed;
					}
				}
			}	
			else	
			{
				for( int y = 0 ; y < 480 ; y++ )
				{
					for( int x = 0 ; x < 640 ; x++ )
					{
						RGBQUAD quad = *pBufferRun;
						pBufferRun++;
						mat.at<cv::Vec3b>(y,x)[0] = quad.rgbBlue;
						mat.at<cv::Vec3b>(y,x)[1] = quad.rgbGreen;
						mat.at<cv::Vec3b>(y,x)[2] = quad.rgbRed;
					}
				}
			}

			emit newVideoFrame( m_id, img );
		}
	   //////////////////////////////////////////////start of IR attempt/////////////////////////////
		else
		{
			/*int width;
			int height;
			if( pImageFrame->eResolution == NUI_IMAGE_RESOLUTION_320x240 )
			{
				width = 320;
				height = 240;
			}
			else
			{*/
			//new res
			int	width = 640;
			int height = 480;
			//}
			CvMatData img;
			img = CvMatData::create(width, height, CV_16U, 1);
				
			// draw the bits to the bitmap needs to be casted to unsigned shorts as sdk gives a signed short by default
			USHORT* pBufferRun = (USHORT*) pBuffer;
			cv::Mat& mat = img;
	
			// todo should be faster with memcpy
			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
				{
					//bitshift: << bigger , >>smaller
					mat.at<USHORT>(y,x) = (*pBufferRun);
					pBufferRun++;
					//mat.at<USHORT>(y,x)  = Nui_ShortToIntensity(*pBufferRun);
					//pBufferRun++;
				}
			}
			emit newVideoFrame( m_id, img );
		}
		///////////////////////////////////////////////end of attempt/////////////////////////////
	}
    else
    {
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

    NuiImageStreamReleaseFrame( m_pVideoStreamHandle, pImageFrame );
}

void KinectDevice::Nui_GotSkeletonAlert()
{
    SkeletonFrame sf;
    HRESULT hr = NuiSkeletonGetNextFrame( 0, sf.getNuiSkeletonFramePointer() );
    if( FAILED( hr ) )
    {
        return;
    }

    bool foundSkeleton = false;
    for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
    {
        if( sf.getNuiSkeletonFramePointer()->SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
        {
            foundSkeleton = true;
        }
    }

    // no skeletons!
    if( !foundSkeleton )
    {
        return;
    }
    else
    {
        sf.setValid();
    }

    // smooth out the skeleton data
    NuiTransformSmooth( sf.getNuiSkeletonFramePointer(), NULL );

    emit newSkeletonFrame( m_id, sf );
}

KinectDevice::KinectState KinectDevice::getState() const
{
    QMutexLocker lock( &m_stateMutex );
	return m_state;
}

void KinectDevice::setState( KinectDevice::KinectState state )
{
    QMutexLocker lock( &m_stateMutex );
    m_state = state;
}

RGBQUAD KinectDevice::Nui_ShortToQuad_DepthAndPlayerIndex( USHORT s )
{
    USHORT RealDepth = (s & 0xfff8) >> 3; //65528 ?2^16=65536??
    USHORT Player = s & 7; //bitwise and operation

    // transform 13-bit depth information into an 8-bit intensity appropriate
    // for display (we disregard information in most significant bit)
    BYTE l = 255 - (BYTE)(256*RealDepth/0x0fff);

    RGBQUAD q;
    q.rgbRed = q.rgbBlue = q.rgbGreen = 0;

    switch( Player )
    {
    case 0:
        q.rgbRed = l / 2;
        q.rgbBlue = l / 2;
        q.rgbGreen = l / 2;
        break;
    case 1:
        q.rgbRed = l;
        break;
    case 2:
        q.rgbGreen = l;
        break;
    case 3:
        q.rgbRed = l / 4;
        q.rgbGreen = l;
        q.rgbBlue = l;
        break;
    case 4:
        q.rgbRed = l;
        q.rgbGreen = l;
        q.rgbBlue = l / 4;
        break;
    case 5:
        q.rgbRed = l;
        q.rgbGreen = l / 4;
        q.rgbBlue = l;
        break;
    case 6:
        q.rgbRed = l / 2;
        q.rgbGreen = l / 2;
        q.rgbBlue = l;
        break;
    case 7:
        q.rgbRed = 255 - ( l / 2 );
        q.rgbGreen = 255 - ( l / 2 );
        q.rgbBlue = 255 - ( l / 2 );
    }

    return q;
}

int KinectDevice::getAngle()
{
	long angle = 0;
	//new way of initialisation due to depth requires new way of this as well
	//m_nuiInstance->NuiCameraElevationGetAngle(&angle);
	HRESULT hr = m_nuiInstance->NuiCameraElevationGetAngle(&angle);
	if ( hr == E_NUI_DEVICE_NOT_READY)
	{
		qDebug() << "Kinect has not been initialized in getAngle() of kinect.";
	}
	else if ( hr == E_POINTER)
	{
		qDebug() << "Pointer error in getAngle() of kinect.";
	}

	//NuiCameraElevationGetAngle(&angle);
	//TEMP also send the vecotr 4 values
	//Vector4 gravityvector;
	//this is supposed to require windows SDK 1.6, please remove for older sdk compatibility. However with 1.6 i didn't even get it working
	//can't call this method for some reason: 
	//NuiAccelerometerGetCurrentReading(&gravityvector);
	//qDebug() << "x:" << gravityvector.x << "y:" << gravityvector.y << "z:" << gravityvector.z;
	return (int) angle;
}

void KinectDevice::setAngle(int angle)
{
	long tempanglelong = (long) angle;
	//KINECT API:
	//You should tilt the Kinect as few times as possible, to minimize wear on the sensor and to minimize tilting time. 
	//The tilt motor is not designed for constant or repetitive movement, and attempts to use it that way can cause degradation of motor function. 
	//To reduce wear, your application should change the elevation angle no more than once per second. 
	//In addition, you must allow at least 20 seconds of rest after 15 consecutive changes. 
	//If your application exceeds these limits, the tilt motor may experience a lock period during which attempting to set the elevation angle will result in an error code.
	//HRESULT kinectmove = NuiCameraElevationSetAngle(tempanglelong);
	HRESULT kinectmove = m_nuiInstance->NuiCameraElevationSetAngle(tempanglelong);

	if (kinectmove == ERROR_RETRY)
	{
		qDebug() << "The time interval between subsequent calls to NuiCameraElevationSetAngle was too short.";
	}
	else if (kinectmove == ERROR_TOO_MANY_CMDS)
	{
		qDebug() << "Too many calls to NuiCameraElevationSetAngle were issued in a short time interval.";
	}
	else if (kinectmove == E_NUI_DEVICE_NOT_READY)
	{
		qDebug() << "Kinect has not been initialized.";
	}
	else if (kinectmove != S_OK)
	{
		qDebug() << "there was an error with code" << kinectmove;
	}
}

void KinectDevice::threadFinished()
{
	emit deviceFinished( m_id );
}


