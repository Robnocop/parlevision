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

#ifndef PLVFACETRACKCHECK_H
#define PLVFACETRACKCHECK_H

//#include "mskinectplugin_global.h"
#include "mskinectdatatypes.h"
#include "D:\ddevelop\mrKinectSDK\inc\NuiSensor.h"
#include <QThread>
#include <QMutex>
#include <plvcore/CvMatData.h>
#include <plvcore/plvglobal.h>

#include "mskinectdevice.h"
//for facetracking include C:\Program Files\Microsoft SDKs\Kinect\Developer Toolkit v1.8.0\inc or something similar
#include "FaceTrackLib.h"
//#include "C:\Program Files\Microsoft SDKs\Kinect\Developer Toolkit v1.8.0\inc\FaceTrackLib.h"
//#include <NuiApi.h>


namespace plvmskinect
{
    //class FaceTrackCheck : public QThread
    class FaceTrackCheck : public QObject
	{
        Q_OBJECT
        //Q_DISABLE_COPY( FaceTrackCheck )

    private:
        
		//added and removed
		BSTR		m_instanceId;

		mutable QMutex m_stateMutex;
        mutable QMutex m_deviceMutex;

		//removed first two
        HANDLE      m_hNextDepthFrameEvent;
        HANDLE      m_hNextVideoFrameEvent;
        HANDLE      m_hNextSkeletonEvent;
        HANDLE      m_pDepthStreamHandle;
        //full depth
		HANDLE		m_pDepthStreamHandle2; 
		HANDLE      m_pVideoStreamHandle;


        void zero();
        //void Nui_GotDepthAlert();
        //void Nui_GotVideoAlert();
        //void Nui_GotSkeletonAlert();
		RGBQUAD Nui_ShortToQuad_DepthAndPlayerIndex( USHORT s );
		//BYTE Nui_ShortToIntensity( USHORT s); 
		//int getMaxScaleX() {return m_maxscalex;};
		//int getMaxScaleY() {return m_maxscaley;};

		//checkcamera is the main thing!
		void CheckCameraInput();
		////an added boolean to check whether facetracking is even needed
		
		bool m_LastTrackSucceeded;
		FLOAT       m_ZoomFactor;   // video frame zoom factor (it is 1.0f if there is no zoom)
		POINT       m_ViewOffset;
		IFTImage*   m_DepthBuffer;
		IFTImage*   m_VideoBuffer;
		IFTResult*                  m_pFTResult;
		IFTFaceTracker*             m_pFaceTracker;
		FT_VECTOR3D* pHint3D;

		FT_CAMERA_CONFIG m_videoConfig;
		FT_CAMERA_CONFIG m_depthConfig;
		FT_CAMERA_CONFIG* m_pDepthConfig;
				
		INuiSensor*	m_nuiInstance;
		
		 //IFTIMAGE is not exactly the same type used for opencv/parlevision so we copy it
		//one easily switch from iftimage to iplimage/cvmat data with http://www.benbarbour.com/Convert_Kinect_Color_IFTImage_To_IplImage
		
		//8u???
		//cvCreateImage(cvSize(kinectImage->getWidth(), kinectImage->getHeight(), IPL_DEPTH_8U, 4);
		//memcpy(img->imageData, kinectImage->GetBuffer(), kinectImage->GetBufferSize());
		IFTImage*                   m_colorImage;
		IFTImage*                   m_depthImage;
		float                       m_XCenterFace;
		float                       m_YCenterFace;
		FT_VECTOR3D                 m_hint3D[2];
		bool m_DrawMask;
		NUI_IMAGE_RESOLUTION m_depthRes;

		bool m_acknowledge;
		bool m_ApplicationIsRunning;
		
		KinectDevice* m_yourfather; 
		
		/** Possible camera states */
        enum KinectState {
            KINECT_UNINITIALIZED,
            KINECT_INITIALIZED,
            KINECT_RUNNING,
            KINECT_STOP_REQUESTED
        };
		
		//methods:
		/*float GetXCenterFace()      { return(m_XCenterFace);}
		float GetYCenterFace()      { return(m_YCenterFace);}
		*/////keep in mind that there is a difference between BOOL and bool
		HRESULT GetClosestHint(FT_VECTOR3D* pHint3D);
		BOOL SubmitFraceTrackingResult(IFTResult* pResult);
		void SetCenterOfImage(IFTResult* pResult);
		HRESULT GetVideoConfiguration(FT_CAMERA_CONFIG* videoConfig);
		HRESULT GetDepthConfiguration(FT_CAMERA_CONFIG* depthConfig);

    signals:
        /*void newDepthFrame( int deviceIndex, plv::CvMatData frame );
        void newSkeletonFrame( int deviceIndex, plvmskinect::SkeletonFrame frame );
        void deviceFinished( int deviceIndex );
*/
		void finished();

    public slots:
        virtual void start();
        virtual void stop();
        /*void threadFinished();
		 bool init();
        bool deinit();
*/
        //virtual void run();

		//void CALLBACK    KinectStatusProc(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* deviceName);
		//ADDED
	//temps solution
	public:		
		//for facetracking
		//void setFaceTracking(bool change) {m_facetracking = change;};
		void setAcknowledgeNeeded(bool b);
		//KinectDevice( int id, QObject* parent = 0);
		FaceTrackCheck(KinectDevice* kd);
		virtual ~FaceTrackCheck();
    };
	////callback in plvm
	//void CALLBACK    KinectStatusProc(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* deviceName);


}

#endif // PLVKINECTDEVICE_H

