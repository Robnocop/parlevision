#include "skipframes.h"

#include <QDebug>
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

using namespace plv;

SkipFramesProcessor::SkipFramesProcessor() :
        m_framesToSkip(4),
        m_skipOn(true),
		m_firstrun(true),
		m_lastserial(0)
{
    m_inputPin = createCvMatDataInputPin("input", this);
    m_outputPin = createCvMatDataOutputPin("output", this);
	
	m_inputPin->addSupportedChannels(1);
	m_inputPin->addSupportedChannels(3);

    m_inputPin->addSupportedDepth(CV_8U);
    m_inputPin->addSupportedDepth(CV_16U);

}

SkipFramesProcessor::~SkipFramesProcessor()
{
}

bool SkipFramesProcessor::init()
{
    return true;
}

bool SkipFramesProcessor::deinit() throw ()
{
    return true;
}

bool SkipFramesProcessor::start()
{
	m_firstrun = true;
	m_lastserial = 0;
    return true;
}

bool SkipFramesProcessor::stop()
{
    return true;
}

bool SkipFramesProcessor::process()
{
	//unsigned int results in errors:
	unsigned int serial = this->getProcessingSerial();
    assert(m_inputPin != 0);
    
    CvMatData src = m_inputPin->get();
	
	//works fine:
	//qDebug() << "nonconditional serial debug" << serial << "last " << m_lastserial;
    // publish the new image
	if (!getSkipOn() || m_firstrun || serial >= (m_lastserial + getFramesToSkip()))
	{
		//works fine:
		//qDebug() << "serial debug" << serial << "last " << m_lastserial;
		m_lastserial = serial;
		m_firstrun = false;
		m_outputPin->put(src);
	}
	
    return true;
}
