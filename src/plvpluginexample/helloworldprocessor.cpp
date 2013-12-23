#include "helloworldprocessor.h"

#include <QDebug>

#include "HelloWorldProcessor.h"
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

using namespace plv;

HelloWorldProcessor::HelloWorldProcessor() :
        m_someInt(1337),
        m_someDouble(1.23456),
        m_someBool(true),
        m_someString("hello")
{
    m_inputPin = createCvMatDataInputPin("input", this);
    m_outputPin = createCvMatDataOutputPin("output", this);
	m_outputPin2 = createOutputPin<QString>( "text", this);

	m_inputPin->addSupportedChannels(1);
	m_inputPin->addSupportedChannels(3);

    m_inputPin->addSupportedDepth(CV_8U);
    m_inputPin->addSupportedDepth(CV_16U);

}

HelloWorldProcessor::~HelloWorldProcessor()
{
}

bool HelloWorldProcessor::init()
{
    return true;
}

bool HelloWorldProcessor::deinit() throw ()
{
    return true;
}

bool HelloWorldProcessor::start()
{
    return true;
}

bool HelloWorldProcessor::stop()
{
    return true;
}

bool HelloWorldProcessor::process()
{
    assert(m_inputPin != 0);
    assert(m_outputPin != 0);

    CvMatData src = m_inputPin->get();

    // allocate a target buffer
    CvMatData target;
    target.create( src.width(), src.height(), src.type() );

	//get the current text
	QString texttoshow = getSomeString();

	//get the current processingserial();
	unsigned int serialInInt = this->getProcessingSerial();
	QString serialIntInString = QString("%1").arg(serialInInt);

	//output the text with serial if boolean
	if (!m_someBool)
		m_outputPin2->put(texttoshow);
	else
		m_outputPin2->put(texttoshow.append(serialIntInString));
	
	unsigned int serial = this->getProcessingSerial();
	
    
    // do a flip of the image
    const cv::Mat in = src;
    cv::Mat out = target;
    cv::flip( in, out, (int)m_someBool);

    // publish the new image
	m_outputPin->put( out );

	
    // update our "frame counter"
    this->setSomeInt(this->getSomeInt()+1);

    return true;
}
