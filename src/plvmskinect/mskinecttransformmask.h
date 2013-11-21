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

#ifndef PLVKinectTransformmask_H
#define PLVKinectTransformmask_H

#include "mskinectplugin_global.h"
#include "mskinectdatatypes.h"
#include "D:\ddevelop\mrKinectSDK\inc\NuiSensor.h"
//#include <QThread>
#include <plvcore/CvMatData.h>
#include <QMutex>

#include <plvcore/PipelineProcessor.h>
#include <plvcore/RefPtr.h>
#include <plvcore/Pin.h>
#include <plvcore/Enum.h>

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

namespace plvmskinect
{
    class KinectTransformmask : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( KinectTransformmask )
		
		Q_CLASSINFO("author", "Your Name")
		Q_CLASSINFO("name", "MSKinectTransformmask")
		Q_CLASSINFO("description", "Shows the greyvalues in a spectrum more suitable for the huma n eye to make distinctions, e.g. to analyze something. Techinfo future pipleines: we bitshifted 3 times to get from kinectdeth to m, but allready shifted once to the other side to get to a larger spectrum of greyvalues, thus bitshift 4 times to get to m from the kinectdevice")
	
		Q_PROPERTY( int cutXL READ getCutXL WRITE setCutXL NOTIFY cutXChangedL  )
		Q_PROPERTY( int cutXR READ getCutXR WRITE setCutXR NOTIFY cutXChangedR  )
		Q_PROPERTY( int cutYU READ getCutYU WRITE setCutYU NOTIFY cutYChangedU  )
		Q_PROPERTY( int cutYD READ getCutYD WRITE setCutYD NOTIFY cutYChangedD  )
		Q_PROPERTY( int cutZ READ getCutZ WRITE setCutZ NOTIFY cutZChanged  )

		PLV_PIPELINE_PROCESSOR

		

    public:
		
		int getCutXL() {return m_cutxl;};
		int getCutXR() {return m_cutxr;};
		int getCutYU() {return m_cutyu;};
		int getCutYD() {return m_cutyd;};
		int getCutZ() {return m_cutz;};

		//odd two times publice definition 2nd one seems better
        /** Possible camera states */
     /*   enum KinectState {
            KINECT_UNINITIALIZED,
            KINECT_INITIALIZED,
            KINECT_RUNNING,
            KINECT_STOP_REQUESTED
        };*/

        KinectTransformmask();
        virtual ~KinectTransformmask();

		/** these methods can be overridden if they are necessary for
			your processor */
		virtual bool init();
		virtual bool deinit() throw();
		virtual bool start();
		virtual bool stop();
        /*bool init();
        bool deinit();*/

        //void setState( KinectState state );
        
		//KinectState getState() const;
        int getId() const;
		int width() const;
        int height() const;
		
		//settings
		//int m_infrared;
		//int m_highres;
		int m_xcutoff;
		int m_ycutoff;
		int m_zcutoff;
		
		void setRealWorldCoord(bool change) {m_realcoord= change;};
		
//		int getAngle();
	//	void setAngle(int angle);
		void setMaxScale(int x, int y);

        virtual void run();
		//void CALLBACK    KinectStatusProc(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* deviceName);
	
	public slots:
		void setCutXL(int value);
		void setCutXR(int value);
		void setCutYU(int value);
		void setCutYD(int value);
		void setCutZ(int value);


    private:
		plv::CvMatDataInputPin* m_inputPin;
		plv::CvMatDataOutputPin* m_outputPin;
		
        int m_id;
		bool m_realcoord;
		int m_cutxl;
		int m_cutxr;
		int m_cutyu;
		int m_cutyd;
		int m_cutz;
		int m_maxscalex;
		int m_maxscaley;

		//KinectState m_state;
        //old INuiInstance* m_nuiInstance;
        //INuiSensor*	m_nuiInstance;

        void zero();

//		RGBQUAD Nui_ShortToQuad_DepthAndPlayerIndex( USHORT s );
		//BYTE Nui_ShortToIntensity( USHORT s); 
		bool getRealWorldCoord() {return m_realcoord;};
		
		

		int getMaxScaleX() {return m_maxscalex;};
		int getMaxScaleY() {return m_maxscaley;};

		void FrontalImage(cv::Mat& projection, Vector4 realWorldCoord, USHORT bufferpoint, int cxl, int cxr,int cyu ,int cyd);
		void Nui_GotDepthAlert();
		RGBQUAD Nui_ShortToQuad_DepthAndPlayerIndex(USHORT z);
		//void FrontalImage(cv::Mat& projection, Vector4 realWorldCoord);
		Vector4 TransformationToRealworldEucledianPoints(int x,int y, USHORT z);
		int TransformationToRealworldEucledianPointX(int x, USHORT z);
		int TransformationToRealworldEucledianPointY(int x, USHORT z);
		void transformCalls(int x, int y, int realz, cv::Mat &matout, INuiCoordinateMapper* &pMapperT, bool mssdk);
		//BYTE Nui_ShortToIntensity( USHORT s);

    signals:
        void newDepthFrame( int deviceIndex, plv::CvMatData frame );
        void newVideoFrame( int deviceIndex, plv::CvMatData frame );
        void newSkeletonFrame( int deviceIndex, plvmskinect::SkeletonFrame frame );
        void deviceFinished( int deviceIndex );

		void cutZChanged(int value);
		void cutXChangedL(int value);
		void cutXChangedR(int value);
		void cutYChangedU(int value);
		void cutYChangedD(int value);

    public slots:
        //virtual void start();
        //virtual void stop();
        void threadFinished();

    };
	////callback in plvm
	//void CALLBACK    KinectStatusProc(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* deviceName);
}

#endif // PLVKinectTransformmask_H

