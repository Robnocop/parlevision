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

		void FrontalImage(cv::Mat& projection, Vector4 realWorldCoord, USHORT bufferpoint, int cxl, int cxr,int cyu ,int cyd, int cz);
		//void FrontalImage(cv::Mat& projection, Vector4 realWorldCoord);
		Vector4 TransformationToRealworldEucledianPoints(int x,int y, USHORT z);
		int TransformationToRealworldEucledianPointX(int x, USHORT z);
		int TransformationToRealworldEucledianPointY(int x, USHORT z);

    signals:
        void newDepthFrame( int deviceIndex, plv::CvMatData frame );
        void newVideoFrame( int deviceIndex, plv::CvMatData frame );
        void newSkeletonFrame( int deviceIndex, plvmskinect::SkeletonFrame frame );
        void deviceFinished( int deviceIndex );

    public slots:
        virtual void start();
        virtual void stop();
        void threadFinished();
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
		
    };
	////callback in plvm
	//void CALLBACK    KinectStatusProc(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* deviceName);
}

#endif // PLVKINECTDEVICE_H

