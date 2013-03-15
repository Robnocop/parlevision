/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * All rights reserved.
  *
  * This file is part of the plvcore module of ParleVision.
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

#include "Average.h"
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

using namespace plv;
using namespace plvopencv;

Average::Average() :
    m_numFrames(10),
    m_total(0),
	m_altRunAvg(false)
{
    m_inputPin = createCvMatDataInputPin( "input", this );
    m_outputPin = createCvMatDataOutputPin( "output", this );

    m_inputFrames = createInputPin<int>( "num frames", this, IInputPin::CONNECTION_OPTIONAL, IInputPin::CONNECTION_ASYNCHRONOUS );

    m_inputPin->addAllChannels();
    m_inputPin->addSupportedDepth(CV_8U);
	m_inputPin->addSupportedDepth(CV_16U);
    m_inputPin->addSupportedDepth(CV_32F);

    m_outputPin->addAllChannels();
    m_outputPin->addSupportedDepth(CV_8U);
   // m_outputPin->addSupportedDepth(CV_32F); //why it is and will be 8_U
}

Average::~Average()
{
}

bool Average::start()
{
    m_total = 0;
	//m_avg = CvMatData::create( 640, 480, CV_32F, 1 );
	m_avg = CvMatData::create( 640, 480, CV_8U, 1 );
	
    return true;
}

bool Average::process()
{
	//DOH rewritten this block to a running average as it was not working anymore as a true average, some code optimalisation could be possible therefor

    if( m_inputFrames->isConnected() && m_inputFrames->hasData() )
    {
        int frames = m_inputFrames->get();
        if( m_numFrames != frames )
        {
            setNumFrames(frames);
        }
    }

    CvMatData in = m_inputPin->get();
    //used to be a const but needed to change to get it working for a 16 bit value.
	cv::Mat& src = in;

	// m_avg = CvMatData::create( in.width(), in.height(), CV_32F, in.channels() );
    // m_out = CvMatData::create( in.width(), in.height(), CV_8U, in.channels() );

    if( m_avg.width() != in.width() || m_avg.height() != in.height() || m_avg.channels() != in.channels() )
    {
        m_avg = CvMatData::create( in.width(), in.height(), CV_8U, in.channels() );
		qDebug() << "reset average with correct size and depth";
		m_total = 0;
    }
	
	//cv::Mat mat2;
    //mat2.create( cv::Size(in.width(), in.height()), in.type() );
    
	//the parlevision program might improve if we can use 32F throughout the pipeline, however several opencv functions do not allow this type.
	//convert non 8U pics to 8U
	//getAltRunAvg should be set only at init otherwise the type dfferences will screw everything up
	int m_conversionFactor =1.0;
	if (getAltRunAvg())
	{
		if (src.depth() == CV_16U)
		{
			CvMatData depthTypeChange;
			depthTypeChange = CvMatData::create(src.cols, src.rows, CV_8U, in.channels());
			cv::Mat& dTC = depthTypeChange;
			cv::convertScaleAbs(src, dTC, 0.00390625, 0);
			src = dTC;
		}
		if (src.depth() == CV_32F)
		{
			CvMatData depthTypeChange;
			depthTypeChange = CvMatData::create(src.cols, src.rows, CV_8U, in.channels());
			cv::Mat& dTC = depthTypeChange;
			cv::convertScaleAbs(src, dTC, 255, 0);
			src = dTC;
		}
	}
	else
	{
		if (src.depth() == CV_8U)
		{
			CvMatData depthTypeChange;
			depthTypeChange = CvMatData::create(src.cols, src.rows, CV_32F, in.channels());
			cv::Mat& dTC = depthTypeChange;
			m_conversionFactor = std::numeric_limits<unsigned char>::max();
			src.convertTo(dTC,dTC.type(), 1.0/m_conversionFactor);
			src = dTC;
		}
		if (src.depth() == CV_16U)
		{
			CvMatData depthTypeChange;
			depthTypeChange = CvMatData::create(src.cols, src.rows, CV_32F, in.channels());
			cv::Mat& dTC = depthTypeChange;
			m_conversionFactor = std::numeric_limits<unsigned short>::max();
			src.convertTo(dTC,dTC.type(), 1.0/m_conversionFactor);
			src = dTC;
		}
		else //src allready properly set to 32F
		{

		}
	}

	//this is a local m_temp not the one used for the real N frame average
	CvMatData m_temp = CvMatData::create( in.width(), in.height(), CV_8U, in.channels() );
	cv::Mat& mat2 = m_temp;

	//m_out = CvMatData::create( in.width(), in.height(), CV_8U, in.channels() );
	cv::Mat& avg = m_avg;
	cv::Mat& tmp = m_tmp; 

	//prevent mixing the black NULL average picture with values. 
	if (m_total==0)
	{		
		if (getAltRunAvg())
		{
			//save src to avg
			cv::convertScaleAbs(src, avg, 1.0, 0);
			m_total++;
		}
		else
		{
			//reset global temp for N frames average to zero
			m_tmp = CvMatData::create( in.width(), in.height(), CV_32F, in.channels() );
			m_total++;
		}
	}
	else if (m_total<getNumFrames())
	{
		if (getAltRunAvg())
		{
			m_total++;
		
			//double dgetnumframes = (double) getNumFrames();
			double alpha = (1.0/ getNumFrames());
			double alphaavg =  (1.0-alpha);
			//qDebug()<< "getnum is bigger m_total is" << m_total << "alpha" << getNumFrames();
			cv::convertScaleAbs(avg, mat2, alphaavg, 0);
			//cv::convertScaleAbs(src, src, alpha, 0);
			//http://opencv.willowgarage.com/documentation/python/operations_on_arrays.html
			cv::scaleAdd(src, alpha, mat2, avg);
		}
		else
		{
			cv::accumulate(src, tmp);
			m_total++;
			//probably need to add a weight factor
		}
	}
	else
	{
		if (getAltRunAvg())
		{
			//should have used 1.0 doh!
			//double dgetnumframes = (double) getNumFrames();
			double alpha = (1.0/getNumFrames());
			double alphaavg =  (1-alpha);
			cv::convertScaleAbs(avg, mat2, alphaavg, 0);
			//cv::convertScaleAbs(src, src, alpha, 0);
			//http://opencv.willowgarage.com/documentation/python/operations_on_arrays.html
			cv::scaleAdd(src, alpha, mat2, avg);
		}
		else
		{
			//is it allowed to convert to yourselves?
			tmp.convertTo(tmp, tmp.type(), 1.0 / m_total );
			//as the rest of pipeline do not like a 32F we set it back to 8U anyway
			CvMatData typeChange;
			typeChange = CvMatData::create(src.cols, src.rows, CV_8U, in.channels());
			cv::Mat& tChange = typeChange;
			cv::convertScaleAbs(tmp, tChange, 255, 0);
			tmp = tChange;

			m_avg = tmp;
			m_total = 0;
		}
	}

	// now needed
	if (getAltRunAvg())
	{
		m_avg = avg;
	}

	//convert the 32F to 8U
	//cv::convertScaleAbs(avg, mat2, 1.0, 0);
	//avg.convertTo(mat2, mat2.type(), 255);
	m_outputPin->put(m_avg);

	//this seems to only update beyond every m_total frames, instead we now take a running average even for less frames.
	//TODO rebuild this option, only add up so many frames once.
	//cv::accumulate(src, avg);

  //  if( m_total >= m_numFrames )
  //  {
  //      //avg.convertTo(m_out, m_out.type(), 1.0 / m_total );
		//avg.convertTo(mat2, mat2.type(), 1.0 / m_total );
		//			
		//m_outputPin->put(m_out);

  //     // src.convertTo(m_avg, m_avg.type());
  //      src.convertTo(avg, avg.type());
		//m_total = 0;
  //  }
  //  ++m_total;
    return true;
}

int Average::getNumFrames() const
{
    QMutexLocker lock( m_propertyMutex );
    return m_numFrames;
}

void Average::setNumFrames(int f)
{
    QMutexLocker lock( m_propertyMutex );
    if( f > 0 )
    {
        m_numFrames = f;
    }
    emit numFramesChanged(m_numFrames);
}
