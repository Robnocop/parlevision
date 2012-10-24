#include "TouchBlobAndStringConverter.h"
#include <QDebug>

using namespace plv;
using namespace plvblobtracker;
//for map
//using namespace std;

int widthpane = 640;
int heightpane = 480;
int fingerlength = 40;

#include <opencv/cv.h>
#include <stdlib.h>
#include <stdio.h>
//#include <map>

TouchBlobAndStringConverter::TouchBlobAndStringConverter() : 
	m_minZ(880),
	m_maxZ(1395)
{
    m_inputBlobs = createInputPin< QList<Blob> >( "input", this );
    m_inputImage = createCvMatDataInputPin( "image", this );

    m_inputImage->addAllChannels();
    m_inputImage->addAllDepths();

    m_outputPin = createOutputPin<QString>( "output", this );
	m_outputImage2 = createCvMatDataOutputPin( "closest blob image", this);
	m_outputImage3 = createCvMatDataOutputPin( "all blobs contours", this);
}

TouchBlobAndStringConverter::~TouchBlobAndStringConverter()
{
}

//bool TouchBlobAndStringConverter::init()
//{
//	qDebug() << tr("i start a touchblob");
//}

bool TouchBlobAndStringConverter::process()
{
	cv::Scalar red   = CV_RGB( 255, 0, 0 );
    cv::Scalar green = CV_RGB( 0, 255, 0 );
    cv::Scalar blue  = CV_RGB( 0, 0, 255 );
    cv::Scalar white = CV_RGB( 255, 255, 255 );

	//map the values?
	//map<int, cv::Point3i> arms;
	//int counter = 0;

    CvMatData image = m_inputImage->get();
    const cv::Mat& src = image;
	//picture to show selected blob always set to 640x480 for some reason
	CvMatData out2 = CvMatData::create(widthpane,heightpane,16);
	cv::Mat& dst2 = out2;
	
	CvMatData out3 = CvMatData::create(widthpane,heightpane,16);
	cv::Mat& dst3 = out3;
	
	QList<plvblobtracker::Blob> blobs = m_inputBlobs->get();

    QString out = QString("FRAME:%1#").arg( this->getProcessingSerial() );

	//assume the closest blob is the one we are interested in, so save the r with ymax 
	int ymax = 0;
	cv::Point3i r(0.0f, 0.0f, 0.0f);

	//TODO SELECT closest type x,y,z + inverted
	//TODO add a averaging algorithm of the front pixels (finger) not of the entire blob

    // convert coordinates to what the virtual playground expects
	int averagex = 0;
	int averagey = heightpane+1;
	int averagez = 0;

	//as we want to send int not float:
	int rx = 0;
	int ry = 0;
	int rz = 0;

    foreach( Blob b , blobs )
    {
        
		//qDebug() << "the point is correct (100 110 120" << r.x << r.y << r.z;
		cv::Point p = b.getCenterOfGravity();
		p.y = image.height() - p.y; //so when entering +-471 and when touching approx 320 half of the height as closest will be in as well

		//maybe only measure the front pixels as body is not that interesting but will count cog non the less, so need the if (p.y<ymin) after averaging
		
		const cv::Rect& rect = b.getBoundingRect();
		//b.drawRect(out2,cv::Scalar(0,0,255),true);
		//b.drawBoundingRect(out2, cv::Scalar(0,0,255));
		//evt bijv tl -- tl+20
		averagex = 0;
		averagey = 0; //heightpane+1;
		averagez = 0;
		int totalx = 0;
		int totalz = 0;
		int totaly = -1;//heightpane+1;
		//int pixels = 0;
		int pixelsYX = 0;
		int v = 0;
		int xp2 = 0;
				
		for( int i=rect.tl().x; i < rect.br().x; ++i )
		{
			//y is mirrored ?
			//for( int j=rect.tl().y; j < rect.br().y; ++j )
			//top 20
			//for( int j=rect.tl().y; j < rect.br().y && j < rect.tl().y+fingerlength; j++ )
			//  is further away
			for( int j=rect.br().y; j > rect.tl().y && j > rect.br().y-fingerlength; j-- )
			{
				/*v = src.at<int>(i,j);
				v = v >> 4;*/
				/*if ( v > 0 )
				{
					total = total + v; 
					pixels++;
				}*/
				
				v = src.at<unsigned short>(j,i);
				//v = v >> 4; //makes the number smalle
				//assume everything in the blob besides the finger is black as it is thresholded in the pipeline
				if ( v > 0 )
				{
					totalx = totalx + i;
					totalz = totalz + v; 
					//totaly = totaly + (image.height() - j);
					totaly = totaly + j;
					pixelsYX++;
					//cv::circle(dst3, b.getCenterOfGravity(), 2, cv::Scalar(0,255,0), -1, 8,0 );
				}
			}
		}
		//don't divide by zero
			//pixels++;
		pixelsYX++;
		//bitshift to get approx m 
		//totalz = totalz >> 4;

		//more accurate if we do it later on 
		averagez = (totalz/ pixelsYX);
		averagez = averagez >> 4;
			
		//???
		//heightpane+1
		if (totaly != -1) 
		{
			averagey=  ((totaly-heightpane+1)/ pixelsYX);
			averagex = (totalx/ pixelsYX);
		}
				
		//green CV_RGB( 0, 255, 0 );
		//white 
		//.arg(b.getCenterOfGravity().x)
					//.arg(b.getCenterOfGravity().y)
					//.arg(averagez);
		b.drawContour(dst3, green, true);
		QString info = QString("pos:%1,%2 /n zpos:%3")
					.arg(averagex)
					.arg(averagey)
					.arg(averagez);
		b.drawString(dst3, info, white);

		//need to reset to scale to the cropped image?
		//src.rows are y (<=480)
		//src.cols are x (<=640)
				
		//only sent first y?
		if (averagey >ymax)
		{
			ymax = averagey;

			//x should be account for the field of view.
			//we assume the finger to be flat 
			//deformation of y doesn't really matter 1) only used for proximity so doesn't need to be that accurate and 2) will be near the middle where there is small deformation anyway
			//xp2 = ( (averagex-src.cols/2) * (getMaxZ()-getMinZ()) )/  getMinZ();
			
			//get the amount of pixels it is from middle (- or + ...)
			int xx = (averagex-src.cols/2);
			
			//if xx< 0 ?? left of the middle?
			// does it account for pointing above the screen as xp2 will be negative seems correct in second opinion
			xp2 = (averagez* xx - (xx*getMinZ())) / getMinZ();
			
			//should have been using function of kiect might also take into account the lens distortions?
			// realPoints = NuiTransformDepthImageToSkeleton(x,y,(*pBufferRun)) //bufferrun is z

			if(xx>0)
			{
				rx = xp2+averagex;
			}
			else
			{
			    rx = averagex-xp2;
			}

			//TODO change higher values do make sense for some animation, so don't set those values to max and min values only set in the point vector this isnt used anymore
			if (averagex+xp2 < 0 )
			{
				r.x = 0;
			}
				else if (averagex+xp2 > 640 )
			{
				r.x = 640;
			}
			else
			{
				r.x = averagex+xp2;
			}
			
			//p.x = image.width() - p.x;
			r.y = averagey;
			ry = averagey;
			r.z = averagez; //src.at<int>(p.x,p.y) >> 8; //bitshift
			rz = averagez;
			//cv::circle(dst2, cv::Point(b.getCenterOfGravity().x *widthpane  / src.cols, b.getCenterOfGravity().y*heightpane/ src.rows), 20, cv::Scalar(255,0,0), -1, 8,0 ); //draw the center of gravity
			
		}
				
		//save to map
		//counter++
		//arms.insert(pair<int,cv::Point3i>(counter, r));
    } //and of loop through blobs

    // end TCP frame with newline
	
	//only send the s values <-???
	if (ymax!=0)
	{
		//WHY the F is this sent with floats and in percentage?
		//QString blobString = QString("BEGIN_TOUCH#TOUCH_CENTER_X:%1#TOUCH_CENTER_Y:%2#TOUCH_CENTER_Z:%3#").arg(r.x *widthpane  / src.cols).arg(r.y*heightpane/ src.rows).arg(r.z);
		//lets send it in promiles and int
		QString blobString = QString("BEGIN_TOUCH#TOUCH_CENTER_X:%1#TOUCH_CENTER_Y:%2#TOUCH_CENTER_Z:%3#").arg(rx*1000/src.cols).arg(ry*1000/src.rows).arg(rz);
		out.append(blobString);
		out.append("\n");
		rz = heightpane * (rz - getMinZ()) / (getMaxZ()-getMinZ()) ;
		//divding by src clos and multiplying makes sense for displaying it here
		cv::circle(dst2, cv::Point(rx *widthpane  / src.cols, rz ), 20, cv::Scalar(255,0,0), -1, 8,0 ); //draw the center of gravity
	}

    m_outputPin->put(out);
	m_outputImage2 -> put(out2);
	m_outputImage3 -> put(out3);

    return true;
}

void TouchBlobAndStringConverter::setMinZ(int size)
{
    QMutexLocker lock( m_propertyMutex );
    if( size >= 0 )
        m_minZ = size;
    emit minZChanged(m_minZ);
}

void TouchBlobAndStringConverter::setMaxZ(int size)
{
    QMutexLocker lock( m_propertyMutex );
    if( size >= 0 )
        m_maxZ = size;
    emit maxZChanged(m_maxZ);
}

int TouchBlobAndStringConverter::getMinZ() const
{
    QMutexLocker lock( m_propertyMutex );
    return m_minZ;
}

int TouchBlobAndStringConverter::getMaxZ() const
{
    QMutexLocker lock( m_propertyMutex );
    return m_maxZ;
}

