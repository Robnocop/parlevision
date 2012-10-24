//?? why is it case depended?
#include "plvsynchronyprocessor.h"

#include <QDebug>

#include "PlvSynchronyProcessor.h"
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

using namespace plv;

//constructor don't understand how it sees its ending
PlvSynchronyProcessor::PlvSynchronyProcessor() :
        m_winSize(40), //standard 40 --> 40 frames at 10fps 4s in org application and in Reidsma paper, also seen 1,2,4 and 5
        m_winInc(4),
        m_maxLag(20),
        m_lagShift(4),
        //actually static values that are probably based on the src img so might want to include the original image to automaticaaly recognize its size depth etc.
        //remaining from helloworld
        m_someInt(1337),
        m_someDouble(1.23456),
        m_someBool(true),
        m_someString2("ok")
{

       // m_inputPin = createCvMatDataInputPin("input 1", this);
        //m_inputPin2 = createCvMatDataInputPin("input 2", this);
    //TODO check if double streams are correct or should have been double*
        m_doubleInF = createInputPin<double>("double", this);
        m_doubleInG = createInputPin<double>("double2", this);
        dst = NULL;
        m_outputPin = createCvMatDataOutputPin("output", this);
	//reset values at beginning
        double* fHistory[10000];
        double* gHistory[10000];

        m_currentDisplayLine = 0;
        //to be reset
        m_nrOfValuesStored = 0;


    for (int i = 0; i<m_MAXVALS; i++)
	{
                fHistory[i] = 0;
                gHistory[i] = 0;
	}
}

//WTF ????? is called destructor :S
PlvSynchronyProcessor::~PlvSynchronyProcessor()
{
    // publish the new image
        //this was placed in the destructor
//        if (dst != NULL)
//        {
//                cvReleaseImage(&dst);
//        }
}

bool PlvSynchronyProcessor::init()
{
    return true;
}

bool PlvSynchronyProcessor::deinit() throw ()
{
    return true;
}

bool PlvSynchronyProcessor::start()
{
    return true;
}

bool PlvSynchronyProcessor::stop()
{
    return true;
}

bool PlvSynchronyProcessor::process()
{
//    assert(m_inputPin != 0);
    assert(m_outputPin != 0);

  //  assert(m_inputPin2 != 0);
    assert(m_doubleInF != 0);
    assert(m_doubleInG != 0);

    //get a double value during several seconds???
    // for each person the movements as a pixel difference are taken
    //TODO oi the duoble* is changed to double which cant be good
    const double fInput = m_doubleInF->get();
    const double gInput = m_doubleInG->get();

    int winSize = getWinSize();
    int maxLag = getMaxLag();
    int MAXVALS = m_MAXVALS;

    //TODO make this value chaneble or read automatically
//    int currentDisplayLine = m_currentDisplayline;

            //might want to change this to a set get structure
    //add values to buffer
    //    m_fHistory[m_nrOfValuesStored]=*fInput;
    m_fHistory[getNrOfValuesStored()]=fInput;
    m_gHistory[getNrOfValuesStored()]=gInput;

    //TODO use changeble variables with defaults instead of getting source for output viewing properties
    //and create a viewable output
    //CvMatData src = m_inputPin->get();
   // CvMatData out = CvMatData::create(src.properties());


    //need a iplimage or cvarr for cvcircle
//    IplImage* dst;

    // allocate a target buffer
//    CvMatData target;
//    target = NULL;
//    target.create( src.width(), src.height(), src.type() );
    int imgwidth = 640; //src.width();
    //not used
    int imgheight = 480; //src.height();
   // int imgdepth = 16; //src.depth(); //does not support an CV_8U nor is 0 or 4 supported
    //webcam 640 480 0
//depth options depth: pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16U,
//    IPL_DEPTH_16S, IPL_DEPTH_32S, IPL_DEPTH_32F, IPL_DEPTH_64F

    if (dst == NULL)
    {
            dst = cvCreateImage(cvSize(imgwidth, imgheight), IPL_DEPTH_8U, 3);
    }

    // do a flip of the image
//    const cv::Mat in = src;

//    cv::flip( in, out, (int)m_someBool);

    //for updating frame
//this->setSomeInt(this->getSomeInt()+1);
    this->setNrOfValuesStored(this->getNrOfValuesStored()+1);
    if (getNrOfValuesStored()>=MAXVALS)
    {
            //copy exactly enough values for a full lagged analysis from end of histories to start;
            //needed: winsize+maxlag frames from both functions
            for (int i = 0; i < winSize+maxLag; i++) {
                    m_fHistory[i] = m_fHistory[MAXVALS-winSize-maxLag+i];
                    m_gHistory[i] = m_gHistory[MAXVALS-winSize-maxLag+i];
            }
            //reset nrOfValuesStored to appropriate new value
            setNrOfValuesStored(winSize+maxLag);
    }

    this->setCurrentDisplayLine(this->getCurrentDisplayLine()+1 % imgheight);
//org
//    currentDisplayLine = (currentDisplayLine+1) % imgheight;

//    //if enough values have been filled, start calculating lagged crosscorrelations
    if (getNrOfValuesStored() >= winSize+maxLag)
    {
            double r = 0;
            int lag = 0;
            int pixel = 0;
            int val=0;
        //for each pixel:
            for (pixel=0; pixel < imgwidth/2; pixel++)
            {
                    //calculate lag for this pixel:
                    //-center displays lag=0
                    //-left pixel displays correlation with f and g-lag
                    //-right pixel displays correlation with f-lag and g
                    lag = (int)((double)(pixel*2*maxLag)/(double)imgwidth);
                    //draw result directly on screen.

                    r = corr(getNrOfValuesStored()-lag-winSize,getNrOfValuesStored()-winSize);
                    val = (int)abs(((double)255)*r*r);
                    cvCircle(dst, cvPoint(imgwidth/2-pixel,getCurrentDisplayLine()), 1, CV_RGB(val,val,val), -1);
                    r = corr(getNrOfValuesStored()-winSize,getNrOfValuesStored()-lag-winSize);
                    val = (int)abs(((double)255)*r*r);
                    cvCircle(dst, cvPoint(imgwidth/2+pixel,getCurrentDisplayLine()), 1, CV_RGB(val,val,val), -1);

            }

    }
    else //not enough values yet: draw black/white pattern
    {
            int pixel = 0;
            for (pixel = 0; pixel < imgwidth/2; pixel++)
            {
                    cvCircle(dst, cvPoint(pixel,getCurrentDisplayLine()), 1, CV_RGB(255*(pixel%2),255*(pixel%2),255*(pixel%2)), -1);
                    cvCircle(dst, cvPoint(imgwidth-pixel,getCurrentDisplayLine()), 1, CV_RGB(255*(pixel%2),255*(pixel%2),255*(pixel%2)), -1);
            }
    }
    return true;

    //REMAINS FROM HELLOWORLD
    /////////////////////////////////////
//    assert(m_inputPin != 0);
//    assert(m_outputPin != 0);

//    CvMatData src = m_inputPin->get();

//    // allocate a target buffer
//    CvMatData target;
//
//    .create( src.width(), src.height(), src.type() );

//    // do a flip of the image
//    const cv::Mat in = src;
//    cv::Mat out = target;
//    cv::flip( in, out, (int)m_someBool);

//    // publish the new image
//	//this was placed in the destructor
//	//if (outputPreview != NULL)
//	//	cvReleaseImage(&outputPreview);
    m_outputPin->put( dst );

    // update our "frame counter"
    this->setSomeInt(this->getSomeInt()+1);
    /////////////////////////////////////////////

    return true;

}

// from Dennis Reidsma ParleVision 4.0
// TODO analyse on correctness wth paper and logic and appropriatness of using the algorithm

///////////////////////////////////////////////////////////////////////////////////////////////////
// non-AbstractProcessor interface specific functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/* calculate correlation for two windows of data, which starting at w1 and w2 
//r =(n*sum(xy)-sum(x)*sum(y))/sqrt((n*sum(x^2)-sum(x)^2)*(n*sum(y^2)-sum(y)^2)) */
double PlvSynchronyProcessor::corr(long fIndex, long gIndex)
{
        //http://algorithmsanalyzed.blogspot.com/2008/07/bellkor-algorithm-pearson-correlation.html
        double sumF = 0;
        double sumG = 0;
        double sumFG = 0;
        double sumSqrF = 0;
        double sumSqrG = 0;
//        double fHistory[10000] = getFHistory();
//        double gHistory[10000] = getGHistory();
        int winSize = getWinSize();
        for (int i = 0; i <  winSize; i++)
        {
                sumF += m_fHistory[fIndex+i];
                sumG += m_gHistory[gIndex+i];
                sumFG += m_fHistory[fIndex+i]*m_gHistory[gIndex+i];
                sumSqrF += m_fHistory[fIndex+i]*m_fHistory[fIndex+i];
                sumSqrG += m_gHistory[gIndex+i]*m_gHistory[gIndex+i];
        }
        double pearson = (sumFG-(sumF*sumG)/winSize)/sqrt((sumSqrF-sumF*sumF/winSize)*(sumSqrG-sumG*sumG/winSize));
        return pearson;
}

// namespace Processors {
