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

#ifndef PLVKINECTDEVICE_H
#define PLVKINECTDEVICE_H

#include "mskinectplugin_global.h"
#include "mskinectdatatypes.h"
#include "D:\ddevelop\mrKinectSDK\inc\NuiSensor.h"
#include <QThread>
#include <plvcore/CvMatData.h>
#include <QMutex>

//for facetracking include C:\Program Files\Microsoft SDKs\Kinect\Developer Toolkit v1.8.0\inc or something similar
#include "FaceTrackLib.h"
#include <QThread>
//#include "C:\Program Files\Microsoft SDKs\Kinect\Developer Toolkit v1.8.0\inc\FaceTrackLib.h"
//#include <NuiApi.h>

#include <QVariant>
#include <QVector4D>

namespace plvmskinect
{
    class KinectDevice : public QThread
    {
        Q_OBJECT
        Q_DISABLE_COPY( KinectDevice )

    public:
		//odd two times publice definition 2nd one seems better
        /** Possible camera states */
        enum KinectState {
            KINECT_UNINITIALIZED,
            KINECT_INITIALIZED,
            KINECT_RUNNING,
            KINECT_STOP_REQUESTED
        };

        KinectDevice( int id, QObject* parent = 0);
        virtual ~KinectDevice();

        bool init();
        bool deinit();

        void setState( KinectState state );
        
		KinectState getState() const;
        int getId() const;
		int width() const;
        int height() const;
		
		//settings
		int m_infrared;
		int m_highres;
		////an added boolean to check whether facetracking is even needed
		//somehow other were int bool m_facetracking;
		////an added boolean to check whether facetracking is even needed
		int m_facetracking;
		//ADDED to keep skeleton without facetracking
		bool m_skeletontracking;

		int m_xcutoff;
		int m_ycutoff;
		int m_zcutoff;

        virtual void run();
		//void CALLBACK    KinectStatusProc(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* deviceName);

    private:
        int         m_id;
		bool m_realcoord;
		int m_cutxl;
		int m_cutxr;
		int m_cutyu;
		int m_cutyd;
		int m_cutz;
		int m_maxscalex;
		int m_maxscaley;

		
		KinectState m_state;
        //old INuiInstance* m_nuiInstance;
        INuiSensor*	m_nuiInstance;

		//added and removed
		BSTR		m_instanceId;

		mutable QMutex m_stateMutex;
        mutable QMutex m_deviceMutex;

		//removed first two
        //HANDLE      m_hThNuiProcess;
        //HANDLE      m_hEvNuiProcessStop;
        HANDLE      m_hNextDepthFrameEvent;
        HANDLE      m_hNextVideoFrameEvent;
        HANDLE      m_hNextSkeletonEvent;
        HANDLE      m_pDepthStreamHandle;
        //full depth
		HANDLE		m_pDepthStreamHandle2; 
		HANDLE      m_pVideoStreamHandle;
//        HFONT       m_hFontFPS;

//        HDC         m_SkeletonDC;
//        HBITMAP     m_SkeletonBMP;
//        HGDIOBJ     m_SkeletonOldObj;
//        int         m_PensTotal;

//        RGBQUAD     m_rgbWk[640*480];
//        int         m_LastSkeletonFoundTime;
//        bool        m_bScreenBlanked;
//        int         m_FramesTotal;
//        int         m_LastFPStime;
//        int         m_LastFramesTotal;

        void zero();
        void Nui_GotDepthAlert();
        void Nui_GotVideoAlert();
        void Nui_GotSkeletonAlert();
		RGBQUAD Nui_ShortToQuad_DepthAndPlayerIndex( USHORT s );
		//BYTE Nui_ShortToIntensity( USHORT s); 
		bool getRealWorldCoord() {return m_realcoord;};
		int getCutXL() {return m_cutxl;};
		int getCutXR() {return m_cutxr;};
		int getCutYU() {return m_cutyu;};
		int getCutYD() {return m_cutyd;};
		int getCutZ() {return m_cutz;};

		int getMaxScaleX() {return m_maxscalex;};
		int getMaxScaleY() {return m_maxscaley;};

		void FrontalImage(cv::Mat& projection, Vector4 realWorldCoord, USHORT bufferpoint, int cxl, int cxr,int cyu ,int cyd);
		//void FrontalImage(cv::Mat& projection, Vector4 realWorldCoord);
		Vector4 TransformationToRealworldEucledianPoints(int x,int y, USHORT z);
		int TransformationToRealworldEucledianPointX(int x, USHORT z);
		int TransformationToRealworldEucledianPointY(int x, USHORT z);
		BYTE Nui_ShortToIntensity( USHORT s);
		
		////for facetracking
		void CheckCameraInput();
		
		
		bool m_LastTrackSucceeded;
		FLOAT       m_ZoomFactor;   // video frame zoom factor (it is 1.0f if there is no zoom)
		POINT       m_ViewOffset;
		IFTImage*   m_DepthBuffer;
		IFTImage*   m_VideoBuffer;
		IFTResult*                  m_pFTResult;
		IFTFaceTracker*             m_pFaceTracker;
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
		mutable QMutex m_ftMutex;

		//methods:
		HRESULT GetVideoConfiguration(FT_CAMERA_CONFIG* videoConfig);
		HRESULT GetDepthConfiguration(FT_CAMERA_CONFIG* depthConfig);
		
		float GetXCenterFace()      { return(m_XCenterFace);}
		float GetYCenterFace()      { return(m_YCenterFace);}
		////keep in min that there is a difference between BOOL and bool
		BOOL SubmitFraceTrackingResult(IFTResult* pResult);
		void SetCenterOfImage(IFTResult* pResult);
		//void faceUpdate(FLOAT* f);
		
		FT_VECTOR3D m_HeadPoint[NUI_SKELETON_COUNT];
		FT_VECTOR3D m_NeckPoint[NUI_SKELETON_COUNT];
		
		//TODO tempsolution add the bones from one skeleton as an array of floats
		//upperbody bones only preferably;
		//0-11 or 1-11
		QVector<Vector4> m_UpperBodyPoints;
		QVector<Vector4> m_UpperBodyRotation;
		QVector<QVector4D> m_SendUpperBodyRotations;
		QVector<QVector4D> m_SendUpperBodyPoints;
		bool m_UpperBodyPointsTracked[NUI_SKELETON_POSITION_COUNT];
		bool m_SkeletonTracked[NUI_SKELETON_COUNT];
		
		//from kinectmodule kinecthelper, needed to send it over signal
		// Convert Kinect SDK Vector4 to QVector4D.
        /// @param Vector4 Kinect SDK Vector4.
        /// @return QVector4D The Converted vector.
        QVector4D ConvertVector4ToQVector4D(::Vector4 kinectVector);
		

    signals:
        void newDepthFrame( int deviceIndex, plv::CvMatData frame );
        void newVideoFrame( int deviceIndex, plv::CvMatData frame );
        //FUCK THE OLD VERSION:
		//void newSkeletonFrame( int deviceIndex, plvmskinect::SkeletonFrame frame );
		void newSkeletonFrame( int deviceIndex, NUI_SKELETON_FRAME frame );

        void deviceFinished( int deviceIndex );
		//TODO check type
		void newSkeletonPointsSignal( int deviceIndex, QVector<QVector4D> frame);
		void newSkeletonRotationSignal(int deviceIndex, QVector<QVector4D> frame);
		//void newSkeletonPointsSignal( int deviceIndex, QString frame);


		 /** stops the facetracker thread */
		void stopFaceTracker();
		//forward rotation and translation from facetracker through the face..update slot
		void newfacerotation(int a, float x, float y, float z);
		void newfacetranslation(int a, float x, float y, float z);
		void newfacefeatures(int id, QVector<cv::Point2f> p);

    public slots:
        virtual void start();
        virtual void stop();
        void threadFinished();
		//void faceUpdate(FLOAT *rotationxyz);
		void faceRotUpdate(float x, float y, float z);
		void faceTransUpdate(float x, float y, float z);
		void faceFeatureUpdate(QVector<cv::Point2f> p);

		//ADDED
	//temps solution
	public:		
		void setRealWorldCoord(bool change) {m_realcoord= change;};
		void setCutXL(int change) {m_cutxl = change;};
		void setCutXR(int change) {m_cutxr = change;};
		void setCutYU(int change) {m_cutyu = change;};
		void setCutYD(int change) {m_cutyd = change;};
		void setCutZ(int change) {m_cutz = change;};
		int getAngle();
		void setAngle(int angle);
		void setMaxScale(int x, int y);
		
		//for facetracking
		//void setFaceTracking(bool change) {m_facetracking = change;};
		IFTImage*   GetVideoBuffer(){ return(m_VideoBuffer); };
		IFTImage*   GetDepthBuffer(){ return(m_DepthBuffer); };
		float       GetZoomFactor() { return(m_ZoomFactor); };
		POINT*      GetViewOffSet() { return(&m_ViewOffset); };
		HRESULT     GetClosestHint(FT_VECTOR3D* pHint3D);



    };
	////callback in plvm
	//void CALLBACK    KinectStatusProc(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* deviceName);


}

//no clue why this should be here but it is in the server with qtthreading
//class FaceTrackCheck;

/** Helper class for a QThread to run its own event loop */
class QThreadEx : public QThread
{
protected:
    void run() { exec(); }
};

#endif // PLVKINECTDEVICE_H

