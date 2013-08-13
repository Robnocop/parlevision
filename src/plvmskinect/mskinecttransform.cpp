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

#include "mskinecttransform.h"
#include "NuiSensor.h"
#include "NuiImageCamera.h"
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>

//#ifndef PI
#define PI 3.14159265
//#define PINT 31416 //to use as an integer unsigned __int16 : 0 to 65,535 

#include <QDebug>
#include <QMutexLocker>
#include <QFile>

using namespace plv;
using namespace plvmskinect;
//TODO add newer high resolution for the kinect for windows unit.
//

//KinectTransform::KinectTransform(int id, QObject* parent) :
//    QThread(parent)

KinectTransform::KinectTransform() :
    //cast to OLECHAR???
	//tolookat
	m_id(0),
	m_realcoord( false),
	m_cutxl(0),
	m_cutxr(0),
	m_cutyu(0),
	m_cutyd(0),
	m_cutz( 0),
	m_maxscalex(8960),
	m_maxscaley(6726)
//	m_infrared = false;
//	m_highres = false;
    //connect( this, SIGNAL( finished()), this, SLOT( threadFinished()) );
{
    m_inputPin = createCvMatDataInputPin("input", this);
    m_outputPin = createCvMatDataOutputPin("output", this);
	m_inputPin->addAllChannels();
	m_inputPin->addAllDepths();

	//TOLOOKAT
	m_outputPin->addAllDepths();
	m_outputPin->addAllChannels();
}

KinectTransform::~KinectTransform()
{
	
}

void KinectTransform::zero()
{
	//m_state = KINECT_UNINITIALIZED;

 //   m_nuiInstance          = NULL;
 //   m_hNextDepthFrameEvent = NULL;
 //   m_hNextVideoFrameEvent = NULL;
 //   m_hNextSkeletonEvent   = NULL;
 //   m_pDepthStreamHandle   = NULL;
	////full depth
	//m_pDepthStreamHandle2 = NULL;
 //   m_pVideoStreamHandle   = NULL;
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

bool KinectTransform::init()
{
	zero();

    return true;
}

bool KinectTransform::deinit()
{
	//in the nuiimpl they also delete other skeleton stuff
	//reodered part of deinit to bottom of deinit
	//in the nuiimpl they do not use compare to NULL
	//in the old nuiimpl neither, only in the parlevision v

	
    return true;
}

int KinectTransform::getId() const
{
	return m_id;
}

int KinectTransform::width() const
{
    return 0;
}

int KinectTransform::height() const
{
    return 0;
}

bool KinectTransform::start() 
{
	return true;
}

bool KinectTransform::stop() 
{
	return true;
}

//void KinectTransform::start()
//{
	//TOLOOKAT
	//uses threads, might need to add this for transformation as well
	
	//QMutexLocker lock( &m_deviceMutex );
	//switch( getState() )
 //   {
 //   case KINECT_UNINITIALIZED:
 //       // TODO throw exception?
	//	
 //       break;
 //   case KINECT_INITIALIZED:
 //       // Start thread
	//	
	//	QThread::start();
	//	break;
 //   case KINECT_RUNNING:
	//	
 //   default:
 //       // Do nothing
 //       return;
 //   }
//}

//void KinectTransform::stop()
//{
	//TOLOOKAT
	//QMutexLocker lock( &m_deviceMutex );

 //   switch( getState() )
 //   {
 //   case KINECT_RUNNING:
 //       // Stop the Nui processing thread
 //       // Signal the thread
	//	qDebug() << "i will stop the run and set case to stop request";
 //       setState( KINECT_STOP_REQUESTED );
 //       // switching m_state to KINECT_STOP_REQUESTED
 //       // will cause the run loop to exit eventually.
 //       // wait for that here.
 //       QThread::wait();
 //       //fallthrough
 //   case KINECT_INITIALIZED:
 //       // the thread is not running, so it's safe to release the capture device.
 //       // fallthrough
 //   case KINECT_UNINITIALIZED:
 //   case KINECT_STOP_REQUESTED:
 //       return;
 //   }
//}

void KinectTransform::run()
{
	//TOLOOKAT
	//setState(KINECT_RUNNING);
  
    // Main thread loop
  //while(true)
 /*if( getState() == KINECT_STOP_REQUESTED )
        { 
			qDebug() << "kinectstop requested";			
			break;
		}
*/
	return;
//apparently is set to unitialised at another point.
//	setState( KINECT_INITIALIZED );
}

bool KinectTransform::process()
{
    //changed old const NUI_IMAGE_FRAME * pImageFrame = NULL;
	//NUI_IMAGE_FRAME pImageFrame;
	//for full depth not const
	//const NUI_IMAGE_FRAME * pImageFrame = NULL;
	assert(m_inputPin != 0);
    
	//from src to in
    CvMatData in = m_inputPin->get();
	const cv::Mat& matin = in;

	//no need to check, if( pImageFrame->eResolution == NUI_IMAGE_RESOLUTION_320x240 ) etc. Already set this flag to this resolution earlier on.
	//also we want the two things to be independent, output eucledian and input raw depth fov
	int width = 640;
    int height= 480;

	//CvMatData out = CvMatData::create(in.properties());
	CvMatData out = CvMatData::create(width, height, CV_16U, 1);
	cv::Mat& matout = out;

	//NUI_IMAGE_FRAME pImageFrame;
	//HRESULT tst = m_nuiInstance->N
	//HRESULT hr = m_nuiInstance->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &pImageFrame);
	//get full depth
	//NUI_DEPTH_IMAGE_PIXEL *Iout;
	//BOOL bNearMode = false;
	//INuiFrameTexture * pTexture = NULL;
	//hr = m_nuiInstance->NuiImageFrameGetDepthImagePixelFrameTexture(m_pDepthStreamHandle, &pImageFrame, &bNearMode, &pTexture);
	/*if( FAILED( hr ) )
    {
        return;
    }*/

	//to use http://www.daniweb.com/software-development/c/threads/169747/writing-the-data-of-a-void-pointer-to-a-file
    INuiCoordinateMapper* pMapper;

	//FILE *fp = fopen("ulong.dat", "rb");
	//unsigned long kinectulong = 0;
	//fscanf(fp, "%x", &kinectulong);
	////http://www.cplusplus.com/reference/cstdio/scanf/
	//
	
	//qDebug() << "kinect ulong" << kinectulong;

	//http://doc.qt.digia.com/4.7/qdatastream.html#readRawData
	
	quint32 magic =0;
	QFile file("ulong.dat");
	//file.open(QIODevice::ReadOnly);
	
	//probably wait for is better in init or so;
	if (file.open(QIODevice::ReadWrite)) 
	{
			QDataStream indatastream(&file);
			// qDebug() << "file stream is open";
			
			indatastream >> magic;
			qDebug() << "magic is started" << magic;
			file.close();
	}

	// if (magic !=  12604)
	// {
	//	 //if (resulthr != 12604) 
	////{
	//	qDebug() << "length was not the same" << magic;
	//} 
	//else 
	//{
	//	kinectulong = (unsigned int) magic;
	//	qDebug() << "succeeded length was the same" << kinectulong;
	//}

	//FILE *fp2 = fopen("kinectrelationalparameters.dat", "rb");
	//void* pDataLoad;
	//size_t resulthr = fread(pDataLoad, 1, kinectulong, fp2);
	//fclose(fp2);
	
	void* pDataLoad;
	char* pDataLoadT;
	void* pDataVoid;
	
	ULONG kinectulong = 12604;
	uint& kinectuint = (uint&) magic;
	if (magic>0)
	{
		//should be save, is the same type maybe other format.
		kinectulong = (ULONG) magic;
		uint& kinectuint = (uint&) magic;
		qDebug() << "kinectulong is set" << kinectulong << "uint ptr" << kinectuint;
	}

	
	QFile file2("kinectrelationalparameters.dat");
	//file.open(QIODevice::ReadOnly);
	
	//probably wait for is better in init or so;
	if (file2.open(QIODevice::ReadWrite)) 
	{
			QDataStream indatastream2(&file);
			// qDebug() << "file stream is open";

			///PROBABLY WRONG HERE!!!
			indatastream2.readRawData(pDataLoadT,kinectulong);
			//blabla cant convert readbytes uint * to uint &
			//indatastream2.readBytes(pDataLoadT,kinectuint);
			//pDataLoad;
			//qDebug() << "data is loaded";
			file2.close();
	}

	//pDataLoad = pDataVoid;
	//pDataLoad = (void*) pDataLoadT; 
	//pDataLoad = (void*) pDataLoadT; 

	//for(i = 0; i < 10; i++) printf("%lf\n", y[i]);
	//qDebug() << "pointer _r" << (unsigned long) ptr_r;
	
//	HRESULT NuiCreateCoordinateMapperFromParameters(
//         ULONG dataByteCount,
//         void *pData,
//         INuiCoordinateMapper **ppCoordinateMapper
//)

//		dataByteCountType: ULONG
//[in] The number of bytes of data in the supplied parameters.pDataType: void
//[in] A pointer to the supplied parameters.ppCoordinateMapperType: INuiCoordinateMapper
//[out] A pointer to an INuiCoordinateMapper interface.
	bool cancelline = true;
	HRESULT hr = NuiCreateCoordinateMapperFromParameters( kinectulong, pDataLoad, &pMapper);
	//returns -2147024809 
	// which means a function gets a wrong passed values using incorrect data or datatype
	if( FAILED( hr ) )
    {
		cancelline = true;
		qDebug() << hr;
        return false;
    }
	else
	{
		cancelline = false;
	}



		//NuiGetCoordinateMapper(&pMapper);
		//m_nuiInstance->NuiGetCoordinateMapper(&pMapper);
		//check if int maxval < 307200 (640*480)
	
	if (!cancelline)
{
	//TODOLOOKAT
	//bool realcoordtemp= true; 
	int cxl = getCutXL();
	int cxr = getCutXR();
	int cyu = getCutYU();
	int cyd = getCutYD();
	int cz = getCutZ();
	int realz; //? is this helping needed for the bithshift by one to meters
	//Vector4 realPoints;
	//Vector4 realPointsKinect;
	Vector4 realPointsKinect2;
	NUI_DEPTH_IMAGE_POINT test;
	//floating point /single-value notation is unnecesary
	//for always gettig the closest point it is wise to start with x,y 0 and loop the two ways, it is probably faster to draw over than it is to check whether the point has been drawn and is closer.
	for( int y = height/2 ; y < height ; y++ )
	{
		for( int x = width/2 ; x < width ; x++ )	
		{
			//bitshift?
			realz = matin.at<unsigned short>(y,x) >> 3;
					
			//based on emperical results with cutz value a cz would cut image after 2m 
			if ((realz > 0) && !(((realz) > cz) && (cz > 0)) )
			{
				//http://msdn.microsoft.com/en-us/library/nuisensor.inuicoordinatemapper.mapdepthpointtoskeletonpoint.aspx
				//this actually works and gives same results as own functions but with y flipped
							
				test.x = x;
				test.y = y;
				test.depth = realz;
				//test.depth = pBuffer[y*width+x].depth; //commentout the bitshift >>1
				
				// INuiCoordinateMapper* pMapper;
				// m_nuiInstance->NuiGetCoordinateMapper(&pMapper);
				pMapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, &test, &realPointsKinect2);
				//optimize the cutoff!
				//TODO if realx is beyond point dont calc y, remove check from frontal, if y beyond cy dont do frontal.
				//realPoints = TransformationToRealworldEucledianPoints(x,y,realz); //apperantly bitshift by one (smaller) to get mm
				//realx = TransformationToRealworldEucledianPointX(x,realz);
				//realy = TransformationToRealworldEucledianPointY(y,realz);
				FrontalImage(matout, realPointsKinect2, realz, cxl, cxr, cyu, cyd);
			}
		}
		for( int x = width/2 ; x > 0; x-- )	
		{
			//neater to make a function of it as it has to be done 4 times
			//realz = pBuffer[y*width+x].depth;
			realz = matin.at<unsigned short>(y,x) >> 3;
					
			if ((realz > 0) && !(((realz) > cz) && (cz > 0)) )
			{
				test.x = x;
				test.y = y;
				test.depth = realz;
				pMapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, &test, &realPointsKinect2);
				FrontalImage(matout, realPointsKinect2, realz, cxl, cxr, cyu, cyd);
			}
			//j++;
		}
	}
	for( int y = height/2 ; y>0 ; y-- )
	{

		for( int x = width/2 ; x<width; x++ )	
		{
			realz = matin.at<unsigned short>(y,x) >>3;
			if ((realz > 0) && !(((realz) > cz) && (cz > 0)) )
			{
				test.x = x;
				test.y = y;
				test.depth = realz;
				pMapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, &test, &realPointsKinect2);
				FrontalImage(matout, realPointsKinect2, realz, cxl, cxr, cyu, cyd);
			}
		}

		for( int x = width/2 ; x > 0; x-- )	
		{
			realz = matin.at<unsigned short>(y,x) >>3;
					
			if ((realz > 0) && !(((realz) > cz) && (cz > 0)) )
			{
				test.x = x;
				test.y = y;
				test.depth = realz;
				pMapper->MapDepthPointToSkeletonPoint(NUI_IMAGE_RESOLUTION_640x480, &test, &realPointsKinect2);
				//FrontalImage(matout, realPointsKinect2, realz, cxl, cxr, cyu, cyd);
			}
		}
	}
	//? out = outmat?
}	
	m_outputPin->put( out );
	return true;
}

void KinectTransform::setMaxScale(int x, int y)
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
Vector4 KinectTransform::TransformationToRealworldEucledianPoints(int x,int y, USHORT z)
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

int KinectTransform::TransformationToRealworldEucledianPointX(int x, USHORT z)
{
	x = (int) z* tan(PI*(x-320)/180 * 2925/32000);
	return x;
}

int KinectTransform::TransformationToRealworldEucledianPointY(int y, USHORT z)
{
	y = (int) z* tan(PI*(y-240)/180 * 228/2400);
	return y;
}


//need to make this function absolute by "painitng the bitmap from the inside outward
//odd cutxl and cutxt seem to be switched. 
//TODO x is also mirrored so simplest solution is to switch the values here for the time being
//int cutxr, int cutxl
void KinectTransform::FrontalImage(cv::Mat& projection, Vector4 realWorldCoord, USHORT bufferpoint, int cutxr, int cutxl, int cutyu, int cutyd)
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


//KinectTransform::KinectState KinectTransform::getState() const
//{
//    QMutexLocker lock( &m_stateMutex );
//	return m_state;
//}
//
//void KinectTransform::setState( KinectTransform::KinectState state )
//{
//    QMutexLocker lock( &m_stateMutex );
//    m_state = state;
//}

RGBQUAD KinectTransform::Nui_ShortToQuad_DepthAndPlayerIndex( USHORT s )
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


void KinectTransform::threadFinished()
{
	emit deviceFinished( m_id );
}


