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
        m_someInt(800),
        m_someInt2(8000),
        m_someBool(true)
{
    m_inputPin = createCvMatDataInputPin("input", this);
    m_outputPin = createCvMatDataOutputPin("output", this);
	m_outputPin2 = createCvMatDataOutputPin("output greyvalues", this);
	
	m_inputPin->addAllChannels();
	m_inputPin->addAllDepths();

	//TODO Change
	m_outputPin->addAllDepths();
	m_outputPin->addAllChannels();

	m_outputPin2->addAllDepths();
	m_outputPin2->addAllChannels();
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
	CvMatData out = CvMatData::create(in.properties());
	cv::Mat& mat = out;

	videoMatCache = cv::Mat(Size(matin.cols,matin.rows),CV_8UC3,Scalar(0));
	//videoMatCache = cv::Mat(Size(matin.cols,matin.rows),CV_32FC3,Scalar(0));
	//videoMatCache = cv::Mat(Size(640,480),CV_32FC1);
	//convert into output pin data
    //a 32FC1 goes upto 512 a 8uc3 goes upto 256.
	//Point3_<uchar>* p = image.ptr<Point3_<uchar> >(y,x);
	//videoMatCache.rows 

	//probably better to use UINT8 etc
	int val = 0;
	unsigned short greyval = 0;
//	double maxval = 0;
	int cb = 0;
	int cr = 0;
	int bcol = 0;
	int gcol = 0;
	int rcol = 0;
	int yvalue = 0;
	
	//int numberofbands = 6; //42.67f
	float division = 34.36f;//42.67f //5bands, 256colours : 51.2f; // (65536/number of bands of change) / 256
	
	unsigned int min = getSomeInt()<<4; // bitshift transforms greyval into meters or in this case m into greyval, we need to bitshift 3 times to get from kinectdeth to m and once to get to a larger spectrum of greyvalues
	unsigned int max = getSomeInt2()<<4;
	//int spectrum = max-min;
	// float division2 = (float) spectrum/(255*6);
	
	//unsigned int bandwidth = (unsigned int) 65536/6;
	//qDebug() << in.type();
	//depth=2
	//camera =16
	for (int y = 0; y < (matin.rows); ++y )
		{
			//for (int x = 0; x < (matin.cols); ++x )
			for (int x = 0; x < (matin.cols); ++x )
			{
				switch(in.type())
				{
					case 2:
						//mapp the 16bit values to rgb
						//greyval = matin.at<cv::Vec3b>(y,3)[x];
						//v = src.at<unsigned short>(j,i); //j=y, i=x
						//http://en.wikipedia.org/wiki/HSL_and_HSV#From_HSV
						//use only purple to red 
						//maps to 5 areas 0-300degrees. 65536
						//2^16 /5 = 13107, 26214, 39321, 52428, 65535 
						//2^16 /6 = 10922.5 , 21845, 32767.5, 43690, 54612.5, 65535
						greyval = matin.at<unsigned short>(y,x);
						//bgr
						//theoretical minimum if input is bitshifted to 16bit values: 800*16=12800;
						//52736/6 --> 52740/6=8790
						//*6 12794

						//if sentinel values are set:
						//if (greyval ==0)
						//{
						//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(200);
						//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(200);
						//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(200);
						//}
						///*if (greyval ==0)
						//{
						//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(200);
						//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(200);
						//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(200);
						//}*/
						//else 
						//get out the black values:
						if (greyval < 10)
						{
							videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(0);
							videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(0);
							videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(0);
						}
						//values that are not supposed to be visible turn into grey <21845 the additional /(2) keeps them grey and not white.
						else if (greyval < 12794)
						{
							videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)((greyval)/(255*2));
							videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)((greyval)/(255*2));
							videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)((greyval)/(255*2));
						}
						//red to purple near
						else if (greyval < 21584) //purple to blue (not there)
						{
							videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)((greyval-12794)/division);
							videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(0);
							videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(255);
						}
						//TODO rescale as below 800 no measurements thus <12800 everything will be black
						else if (greyval < (30374)) //purple to blue (not there)
						{
							videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(255);
							videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(0);
							videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(255-((greyval-21584)/division));
						}
						else if (greyval < 39164) //blue to turkoois
						{
							videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(255);
							videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)((greyval-30374)/division);
							videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(0);
						}
						else if (greyval < 47954) //turkoise to green //third 
						{
							videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(255-(greyval-39164)/division);
							videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(255);
							videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(0);
						}
						else if (greyval < 56744)
						{
							videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(0);
							videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(255);
							videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)((greyval-47954)/division);
						}
						//if sentinel values are set this indicates too far:
				/*		else if (greyval == 65520)
						{
							qDebug() << y << "," << x << "was to far";
							videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(255);
							videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(255);
							videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(255);
						}	*/					
						else
						{
							videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(0);
							videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(255-((greyval-56744)/division));
							videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(255);
						}
						//} 
						if (getSomeBool())
						{
							if (greyval < min || greyval > max )
							{
								mat.at<USHORT>(y,x) = 0;
							} 
							else
							{
								//mat.at<USHORT>(y,x) = (*pBufferRun) << 1;
								mat.at<USHORT>(y,x) = (65536*(greyval-min))/(max-min);
							}

							//in colour spectrum
							//if (greyval < min || greyval > max )
							//{
							//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(0);
							//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(0);
							//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(0);
							//}
							////values that are not supposed to be visible turn into grey <21845 the additional /(2) keeps them grey and not white.
							//else if (greyval < (min+spectrum/6))
							//{
							//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)((greyval)/(255*2));
							//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)((greyval)/(255*2));
							//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)((greyval)/(255*2));
							//}
							////red to purple near min..min+spectrum/6
							//else if (greyval < (min+spectrum/6)) //purple to blue (not there)
							//{
							//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)((greyval-min)/division2);
							//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(0);
							//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(255);
							//}
							////TODO rescale as below 800 no measurements thus <12800 everything will be black
							//else if (greyval < ((min+2*spectrum/6))) //purple to blue (not there)
							//{
							//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(255);
							//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(0);
							//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(255-((greyval-(min+spectrum/6))/division2));
							//}
							//else if (greyval < (min+3*spectrum/6)) //blue to turkoois
							//{
							//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(255);
							//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)((greyval-(min+2*spectrum/6))/division2);
							//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(0);
							//}
							//else if (greyval < (min+4*spectrum/6)) //turkoise to green //third 
							//{
							//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(255-(greyval-(min+3*spectrum/6))/division2);
							//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(255);
							//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(0);
							//}
							//else if (greyval < (min+5*spectrum/6))
							//{
							//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(0);
							//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(255);
							//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)((greyval-(min+4*spectrum/6))/division2);
							//}
							////if sentinel values are set this indicates too far:
							//		
							//else
							//{
							//	videoMatCache.at<cv::Vec3b>(y,x)[0] = (int)(0);
							//	videoMatCache.at<cv::Vec3b>(y,x)[1] = (int)(255-((greyval-(min+5*spectrum/6))/division2));
							//	videoMatCache.at<cv::Vec3b>(y,x)[2] = (int)(255);
							//}
						}

						break;
						//bgr?
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
	m_outputPin2->put(out);

    // update our "frame counter" 
	//wtf?
    //this->setSomeInt(this->getSomeInt()+1);

    return true;
}

void MSKinectFakeColor::setSomeInt(int i)
{
	
	QMutexLocker lock(m_propertyMutex);
	if (i < getSomeInt2() && i+1>0 ) 
	{
		m_someInt = i ; 		
	} 
	
	emit someIntChanged(m_someInt);
}

void MSKinectFakeColor::setSomeInt2(int i)
{
	QMutexLocker lock(m_propertyMutex);
	if (i > getSomeInt()) 
	{
		m_someInt2 = i ; 
	} 
	
	emit someInt2Changed(m_someInt2);
}