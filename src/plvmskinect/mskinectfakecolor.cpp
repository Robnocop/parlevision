#include "mskinectfakecolor.h"
//#include "helloworldproccesor.h"
#include <QDebug>

//#include "HelloWorldProcessor.h"
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

using namespace plv;
using namespace cv;
using namespace plvmskinect;

//TODO reset plvcore and plvgui as dependencies unset to debug faster using only rebuild instead of rebuild project only.
MSKinectFakeColor::MSKinectFakeColor() :
        m_someInt(1337),
        m_someInt2(1),
        m_someBool(true),
        m_someString("hello")
{
    m_inputPin = createCvMatDataInputPin("input", this);
    m_outputPin = createCvMatDataOutputPin("output", this);
	
	m_inputPin->addAllChannels();
	m_inputPin->addAllDepths();

	m_outputPin->addAllDepths();
	m_outputPin->addAllChannels();
}

MSKinectFakeColor::~MSKinectFakeColor()
{
}

bool MSKinectFakeColor::init()
{
    return true;
}

bool MSKinectFakeColor::deinit() throw ()
{
    return true;
}

bool MSKinectFakeColor::start()
{
    return true;
}

bool MSKinectFakeColor::stop()
{
    return true;
}

bool MSKinectFakeColor::process()
{
    assert(m_inputPin != 0);
    assert(m_outputPin != 0);

	//from src to in
    CvMatData in = m_inputPin->get();
	const cv::Mat& matin = in;

	//qDebug() << tr("type %1").arg(in.type());
	//qDebug() << tr("type %1 %2 in %3 %4").arg(matin.cols).arg(matin.rows).arg(in.width()).arg(in.height());
	
	//wont work	cv::Mat in = m_inputPin->get();
	//added
	//CvMatData out = CvMatData::create(in.properties());
	videoMatCache = cv::Mat(Size(matin.cols,matin.rows),CV_8UC3,Scalar(0));
	
	//videoMatCache = cv::Mat(Size(640,480),CV_32FC1);
	//convert into output pin data
    //a 32FC1 goes upto 512 a 8uc3 goes upto 256.
	//Point3_<uchar>* p = image.ptr<Point3_<uchar> >(y,x);
	//videoMatCache.rows 

	//probably better to use UINT8 etc
	int val = 0;
//	double maxval = 0;
	int cb = 0;
	int cr = 0;
	int bcol = 0;
	int gcol = 0;
	int rcol = 0;
	int yvalue = 0;
	
	for (int y = 0; y < (matin.rows); ++y )
		{
			//for (int x = 0; x < (matin.cols); ++x )
			for (int x = 0; x < (matin.cols); ++x )
			{
				switch(in.type())
				{
					//GRAY
					case 0:
						//BGR format
						val = matin.at<cv::Vec3b>(y,3)[x];
						cb = 256-val;
						cr = val;
						yvalue = getSomeInt2();
						//R=Y + 1.403*(Cr - 128)
						//G=Y - 0.344*(Cr - 128) - 0.714*(Cb - 128)
						//B=Y + 1.773*(Cb - 128)
						//videoMatCache.at<cv::Vec3b>(y,x)[0] = (float)val;
						bcol = yvalue + 1.7710*cb;
						rcol = yvalue + 1.4022*cr;
						gcol = 1.7047*yvalue - 0.1952*bcol - 0.5647*rcol;
						
						//videoMatCache.at<cv::Vec3b>(y,x)[0] = (float)(bcol);
						//videoMatCache.at<cv::Vec3b>(y,x)[1] = (float)(gcol);
						//videoMatCache.at<cv::Vec3b>(y,x)[2] = (float)(rcol);
						//0 is black 255 is white
						//values with a high value get overlighten also without the -128.. need to find a proper solution
						videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(yvalue + 1.773*(cb - 128));
						videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(yvalue - 0.344*(cb - 128) - 0.714*(cr - 128));
						videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(yvalue + 1.403*(cr - 128));
						//if (val > maxval)
						//{
						//	maxval = val;
						//}
						//val = matin.at<cv::Vec3b>(y,3)[x];
						break;
					//RGB
					case 16:
						val = (matin.at<cv::Vec3b>(y,x)[0]+matin.at<cv::Vec3b>(y,x)[1]+matin.at<cv::Vec3b>(y,x)[2])/3;
						videoMatCache.at<cv::Vec3b>(y,x)[1] = (float)val;
						break;
					//RGBA
					case 24:
						//TODO fix for e.g. RGBA channels, it will only show 3/4th of the image.
						val = (matin.at<cv::Vec3b>(y,x)[0]+matin.at<cv::Vec3b>(y,x)[1]+matin.at<cv::Vec3b>(y,x)[2])/3;
						videoMatCache.at<cv::Vec3b>(y,x)[1] = (float)val;
						break;
				}
				//uint16_t val = videoMatCache.at<uint16_t>(y,x);
				//videoMatCache.at<float>(y,x) = val/2048.0f; 
				//ORDERED AS BGR probably [0] = b, [1] = g so [2] = r
				//videoMatCache.at<cv::Vec3b>(y,x)[1] = (float)(this->getSomeInt()) * 1.0f; //2048.0f;
				//strange stuff happends when adding differnet kinds of inputs, normal input one fourth gets duplicated
				//to rgb will duplicat it four times in a scaled manner.
			
				//val = matin.at<int>(y,x);
				//WORKS FOR THE RGB2GRAY-ed image val = matin.at<cv::Vec3b>(y,3)[x];
				//val = matin.at<cv::Vec3b>(y,3)[x];
				//WORKS FOR COLOR camera probably takes one channel?
				//val = matin.at<cv::Vec3b>(y,x)[(int)(this->getSomeDouble())];
				//e.g. make a simple gray mean value.
				// type 0 
				//val = (matin.at<cv::Vec3b>(y,x)[0]+matin.at<cv::Vec3b>(y,x)[1]+matin.at<cv::Vec3b>(y,x)[2])/3;
				//videoMatCache.at<cv::Vec3b>(y,x)[1] = (float)val; //(float)(this->getSomeInt()) * 1.0f; //2048.0f; 
			}
		}
		//need TO CONVERT 1-channel to yuv(eye dependent representation) and then back to rgb for proper 
//	Y=0.299*R + 0.587*G + 0.114*B
//Cr=(R-Y)*0.713 + 128
//Cb=(B-Y)*0.564 + 128
//
//R=Y + 1.403*(Cr - 128)
//G=Y - 0.344*(Cr - 128) - 0.714*(Cb - 128)
//B=Y + 1.773*(Cb - 128)
	//qDebug() << tr("max %1").arg(maxval);
	//change to useable CvMatadata structure to show it.
	CvMatData a(videoMatCache,true);
	
    // publish the new image
    //TEMP m_outputPin->put( target );
	m_outputPin->put( a );

    // update our "frame counter"
    this->setSomeInt(this->getSomeInt()+1);

    return true;
}
