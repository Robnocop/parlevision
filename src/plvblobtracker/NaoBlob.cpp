#include "NaoBlob.h"
#include <QDebug>

using namespace plv;
using namespace plvblobtracker;

#include <opencv/cv.h>

NaoBlob::NaoBlob()
{
    m_inputBlobs = createInputPin< QList<Blob> >( "input", this );
    m_inputImage = createCvMatDataInputPin( "image", this );

    m_inputImage->addAllChannels();
    m_inputImage->addAllDepths();

    m_outputPin = createOutputPin<QString>( "output", this );
	m_outputImage2 = createCvMatDataOutputPin( "selected blob image", this);
	m_outputImage3 = createCvMatDataOutputPin( "selected blob image", this);
}

NaoBlob::~NaoBlob()
{
}

//bool TouchBlobAndStringConverter::init()
//{
//	qDebug() << tr("i start a touchblob");
//}

bool NaoBlob::process()
{
	
    CvMatData image = m_inputImage->get();
    const cv::Mat& src = image;
	//picture to show selected blob
	CvMatData out2 = CvMatData::create(640,480,16);
	cv::Mat& dst2 = out2;
	
	CvMatData out3 = CvMatData::create(640,480,16);
	cv::Mat& dst3 = out3;
	
	QList<plvblobtracker::Blob> blobs = m_inputBlobs->get();

    QString out = QString("FRAME:%1#").arg( this->getProcessingSerial() );

	//TODO SELECT closest type x,y,z + inverted
	//TODO add a averaging algorithm of the front pixels (finger) not of the entire blob

    // convert coordinates to what the virtual playground expects
    foreach( Blob b , blobs )
    {
        cv::Point3i r(0.0f, 0.0f, 0.0f);
		//qDebug() << "the point is correct (100 110 120" << r.x << r.y << r.z;
		cv::Point p = b.getCenterOfGravity();
		r.x = p.x;
		//p.x = image.width() - p.x;
        p.y = image.height() - p.y;
		r.y = p.y;

		const cv::Rect& rect = b.getBoundingRect();
		//b.drawRect(out2,cv::Scalar(0,0,255),true);
		//b.drawBoundingRect(out2, cv::Scalar(0,0,255));
		//evt bijv tl -- tl+20
		int average = 0;
		int total = 0;
		//int pixels = 0;
		int pixelsYX = 0;
		int v = 0;

		for( int i=rect.tl().x; i < rect.br().x; ++i )
		{
			for( int j=rect.tl().y; j < rect.br().y; ++j )
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
				if ( v > 0 )
				{
					total = total + v; 
					pixelsYX++;
					cv::circle(dst3, b.getCenterOfGravity(), 2, cv::Scalar(0,255,0), -1, 8,0 );
				}
			}
		}
		//don't divide by zero
		//pixels++;
		pixelsYX++;
		qDebug() << "total" << total ; //"nr of pixels" << pixels 
		qDebug() << "nr of pixelsYX" << pixelsYX ; //"nr of pixels" << pixels 
		total = total >> 4;
		average = (total/ pixelsYX);

		cv::circle(dst2, b.getCenterOfGravity(), 20, cv::Scalar(255,0,0), -1, 8,0 ); //draw the center of gravity
		r.z = average; //src.at<int>(p.x,p.y) >> 8; //bitshift
		QString blobString = QString("BEGIN_TOUCH#TOUCH_ID:%1#TOUCH_CENTER_X:%2#TOUCH_CENTER_Y:%3#TOUCH_CENTER_Z:%4#").arg(0).arg(r.x).arg(r.y).arg(r.z);
		out.append(blobString);
		out.append("\n");
    } //and of loop through blobs

    // end TCP frame with newline
    
    m_outputPin->put(out);
	m_outputImage2 -> put(out2);
	m_outputImage3 -> put(out3);

    return true;
}
