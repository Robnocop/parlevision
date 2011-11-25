#ifndef FREENECTPRODUCER_H
#define FREENECTPRODUCER_H


#include <QMutex>
#include <QWaitCondition>

#include <plvcore/PipelineProducer.h>
#include <plvcore/CvMatData.h>
#include <plvcore/Enum.h>

#include "hmifreenect.h"

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>


namespace plv
{
    class CvMatDataOutputPin;
}

namespace plvfreenect
{
    class FreenectProducer : public plv::PipelineProducer
    {
        Q_OBJECT
        Q_DISABLE_COPY( FreenectProducer )

        Q_CLASSINFO("author", "Dennis Reidsma")
        Q_CLASSINFO("name", "Libfreenect video producer")
        Q_CLASSINFO("description", "This producer produces images from a Kinect using the libfreenect libraries. Yields color and depth images.")
        Q_PROPERTY( double tiltDegree READ getTiltDegree WRITE setTiltDegree NOTIFY tiltDegreeChanged )
        Q_PROPERTY( int kinectId READ getKinectId WRITE setKinectId NOTIFY kinectIdChanged )

        Q_PROPERTY( plv::Enum videoFormat READ getVideoFormat WRITE setVideoFormat NOTIFY videoFormatChanged )

        /** required standard method declaration for plv::PipelineProducer */
        PLV_PIPELINE_PRODUCER

    public:
        FreenectProducer();
        virtual ~FreenectProducer();

        virtual bool init();
        virtual bool deinit() throw();
        virtual bool start();
        virtual bool stop();

        inline double getTiltDegree() const { return m_tiltDegree; }
        inline int getKinectId() const { return m_kinectId; }
        plv::Enum getVideoFormat() { return m_videoFormat; }

    protected:
        double m_tiltDegree;
        int m_kinectId;
        plv::Enum m_videoFormat;

        HmiFreenect theFreenect;
        HmiFreenectDevice* theDevice;

        cv::Mat depthMatFromKinect;
        cv::Mat depthMatCache;
        cv::Mat videoMatFromKinect;
        cv::Mat videoMatCache;

        plv::CvMatDataOutputPin* m_outputPinVideo;
        plv::CvMatDataOutputPin* m_outputPinDepth;

        bool b_initialized;

    public slots:
        void setTiltDegree(double d);
        void setKinectId(int i);
        void setVideoFormat(plv::Enum v){ m_videoFormat = v; }
;

    signals:
//added video format
        void videoFormatChanged(plv::Enum newValue);
        void tiltDegreeChanged(double d);
        void kinectIdChanged(int i);
        void conversionTypeChanged(plv::Enum newValue);

    };

}


#endif // FREENECTPRODUCER_H
