
#include "freenectproducer.h"

#include <QDebug>
#include <QStringBuilder>
#include <QVariant>

#include <plvcore/CvMatDataPin.h>
#include "hmifreenect.h"
//#include "libfreenect.h"
#include "libfreenectplugin.h"

using namespace plv;
using namespace cv;
using namespace plvfreenect;

FreenectProducer::FreenectProducer():
    m_tiltDegree(0),
    m_kinectId(0),
    b_initialized(false)
{

    int vmc = freenect_get_video_mode_count();
    for (int i = 0; i < vmc; i++)
    {
        PLV_ENUM_ADD(m_videoFormat, freenect_get_video_mode(i).video_format);
    }

    setVideoFormat( m_videoFormat );

    // we have two output pins (color and depth)
    m_outputPinVideo = createCvMatDataOutputPin( "video", this );
    m_outputPinDepth = createCvMatDataOutputPin( "depth", this );

    m_outputPinVideo->addSupportedChannels(3);
    m_outputPinVideo->addSupportedDepth(CV_8U);
    m_outputPinDepth->addSupportedChannels(1);
    m_outputPinDepth->addSupportedDepth(CV_16U);

}

FreenectProducer::~FreenectProducer()
{
}

void FreenectProducer::setTiltDegree(double d)
{
    if (d>30.0)d=30.0;
    if (d<-30.0)d=-30.0;
    m_tiltDegree = d;
    emit(tiltDegreeChanged(d));
    if (b_initialized)theDevice->setTiltDegrees(m_tiltDegree);
}

//todo prevent a second kinectproducer at launch to have same id or bigger id than count
void FreenectProducer::setKinectId(int i)
{
    if (b_initialized)
    {
        QString msg = "Cannot change Kinect Id while pipeline is running";
        message( PlvWarningMessage, msg);
        emit(kinectIdChanged(m_kinectId));
    }
    else
    {
        if (i<0) i = 0; 
        if (i>theFreenect.deviceCount()-1)i=theFreenect.deviceCount()-1;
        m_kinectId = i;
        emit(kinectIdChanged(i));
    }
}

bool FreenectProducer::readyToProduce() const
{
    //QString msg = "ready?";
    //qDebug() << msg;
    QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
    if(freenect_process_events(theFreenect.m_ctx) < 0) throw std::runtime_error("Cannot process freenect events");

    return b_initialized && (theDevice->newVideoAvailable()) && (theDevice->newDepthAvailable());
}

bool FreenectProducer::produce()
{
    //QString msg = "produce";
    //qDebug() << msg;
    if (!theDevice->getVideo(videoMatFromKinect)) throw std::runtime_error("no new video available while in produce");
    if (!theDevice->getDepth(depthMatFromKinect)) throw std::runtime_error("no new depth available while in produce");

    //copy video to cache
    videoMatFromKinect.copyTo(videoMatCache);
    //copy depth to cache; rescale a bit
    for (int y = 0; y < depthMatFromKinect.rows; ++y )
    {
        for (int x = 0; x < depthMatFromKinect.cols; ++x )
        {
            uint16_t val = depthMatFromKinect.at<uint16_t>(y,x);
            depthMatCache.at<float>(y,x) = val/2048.0f;
        }
    }

    //convert into output pin data
    CvMatData a(videoMatCache,true);
    CvMatData b(depthMatCache,true);

    //set both to output pin
    m_outputPinVideo->put(a);
    m_outputPinDepth->put(b);


    return true;
}


bool FreenectProducer::init()
{
    depthMatFromKinect = cv::Mat(Size(640,480),CV_16UC1);
    depthMatCache = cv::Mat(Size(640,480),CV_32FC1);
    videoMatFromKinect = cv::Mat(Size(640,480),CV_8UC3,Scalar(0));
    videoMatCache = cv::Mat(Size(640,480),CV_8UC3,Scalar(0));


    b_initialized = false;
    try
    {
        theDevice = &theFreenect.createDevice<HmiFreenectDevice>(m_kinectId);
        theDevice->setLed(LED_BLINK_RED_YELLOW);
        b_initialized = true;
    }
    catch( std::exception& e )
    {
        QString msg = "Error initializing Freenect " ;
        qWarning() << msg;
        deinit();
    }
    return true;
}

bool FreenectProducer::deinit() throw()
{
    try
    {
        theDevice->setLed(LED_BLINK_GREEN);
        theFreenect.deleteDevice(m_kinectId);
    }
    catch( std::exception& e )
    {
        QString msg = "Error deinitializing Freenect";
        qWarning() << msg;
    }
    b_initialized = false;
    return true;
}

bool FreenectProducer::start()
{
    theDevice->startVideo();
    theDevice->startDepth();
    theDevice->setLed(LED_RED);
    return true;
}

bool FreenectProducer::stop()
{
    theDevice->stopVideo();
    theDevice->stopDepth();
    theDevice->setLed(LED_BLINK_RED_YELLOW);
    return true;
}


