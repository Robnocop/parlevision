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

#include <QDebug>

#include "BackgroundSubtractor.h"
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

using namespace plv;
using namespace plvopencv;

enum SubtractionMethod {
    BGSM_GRAYSCALE,
    BGSM_COLOR_TO_GRAY,
    BGSM_COLOR_TO_COLOR
};

BackgroundSubtractor::BackgroundSubtractor() : m_threshold(128), m_replacement(255), m_reset(true)
{
    m_inInput = createCvMatDataInputPin( "input", this, IInputPin::CONNECTION_REQUIRED, IInputPin::CONNECTION_SYNCHRONOUS );
    m_inBackground = createCvMatDataInputPin( "background", this, IInputPin::CONNECTION_OPTIONAL, IInputPin::CONNECTION_ASYNCHRONOUS );
    m_inReset = createInputPin<bool>( "reset" , this, IInputPin::CONNECTION_OPTIONAL, IInputPin::CONNECTION_ASYNCHRONOUS );

    m_outForeground = createCvMatDataOutputPin( "foreground", this );
    m_outBackground = createCvMatDataOutputPin( "background", this );

    m_inInput->addSupportedChannels(1);
    m_inInput->addSupportedChannels(3);
    m_inInput->addSupportedDepth(CV_8U);
	m_inInput->addSupportedDepth(CV_16U);
	m_inInput->addSupportedDepth(CV_32F);

    m_inBackground->addSupportedChannels(1);
    m_inBackground->addSupportedChannels(3);
    m_inBackground->addSupportedDepth(CV_8U);
	m_inBackground->addSupportedDepth(CV_16U);
	m_inBackground->addSupportedDepth(CV_32F);

    m_outForeground->addAllChannels();
    m_outForeground->addAllDepths();

    m_outBackground->addSupportedChannels(1);
    m_outBackground->addSupportedDepth(CV_8U);
	m_outBackground->addSupportedDepth(CV_16U);
	m_outBackground->addSupportedDepth(CV_32F);

    m_method.add("gray", BGSM_GRAYSCALE);
    m_method.add("color to gray", BGSM_COLOR_TO_GRAY);
    m_method.add("color to color", BGSM_COLOR_TO_COLOR);
}

BackgroundSubtractor::~BackgroundSubtractor()
{
}

bool BackgroundSubtractor::process()
{
	//qDebug() << "enter process"; 
    CvMatData in = m_inInput->get();
	cv::Mat& bgmat = m_backgroundGray;
	//does not seem to return the actual depth
	//qDebug() << "depth "<< m_background.depth() << "actualdetpth"<< m_background.depthToString(m_background.depth()) << "depth to string 1" << m_background.depthToString(1) << "depth to string 2" << m_background.depthToString(2) <<"depth to string 3" << m_background.depthToString(3);
    // m_reset is true on initialisation
    // so the background is always initialized
    // to the first frame received
    if( m_reset )
    {
        setBackground(in);
        setReset(false);
    }

    if( m_inBackground->isConnected() && m_inBackground->hasData() )
    {
        CvMatData inbg = m_inBackground->get();
        setBackground(inbg);
    }

    if( m_inReset->isConnected() && m_inReset->hasData() )
    {
        bool value = m_inReset->get();
        if( value )
        {
            setBackground(in);
        }
    }

    //check format of images
    if( !m_background.isEmpty() )
    {
        if( in.width() != m_background.width() || in.height() != m_background.height() )
        {
            QString msg = tr("Images do not have same number of channels.");
            setError(PlvPipelineRuntimeError, msg);
            return false;
        }

		
        CvMatData srcGray = CvMatData::create(in.width(), in.height(), 1);
        CvMatData outGray = CvMatData::create(in.width(), in.height(), 1);
		cv::Mat& inmat = in;
		cv::Mat& srcgmat = srcGray;
		cv::Mat& distg = outGray;
        cv::Mat& mbg = m_background;

		if( m_method.getSelectedValue() == BGSM_GRAYSCALE )
        {
			////shouldn't be here
   //         CvMatData srcGray = CvMatData::create(in.width(), in.height(), 1);
   //         CvMatData outGray = CvMatData::create(in.width(), in.height(), 1);
			//cv::Mat& inmat = in;
			//cv::Mat& srcgmat = srcGray;
			//cv::Mat& distg = outGray;

            if( in.channels() == 3 )
            {
               // cv::cvtColor( in, srcGray, CV_RGB2GRAY );
				cv::cvtColor( inmat, srcgmat, CV_RGB2GRAY );
			}
            else
            {
                srcGray = in;
            }
            //cv::absdiff( srcGray, m_backgroundGray, outGray );
            cv::absdiff( srcgmat, bgmat, distg );
			//cv::threshold( outGray, outGray, m_threshold, m_replacement, CV_THRESH_BINARY );
            cv::threshold( distg , distg, m_threshold, m_replacement, CV_THRESH_BINARY );
            

			m_outForeground->put(outGray);
        }
        else if( m_method.getSelectedValue() == BGSM_COLOR_TO_GRAY )
        {
            CvMatData tmp = CvMatData::create(in.properties());
            cv::Mat& tmpmat = tmp;
			//cv::absdiff( in, m_background, tmp );
            cv::absdiff( inmat, bgmat, tmpmat  );
			//cv::threshold( tmp, tmp, m_threshold, m_replacement, CV_THRESH_BINARY );
			cv::threshold( tmpmat, tmpmat, m_threshold, m_replacement, CV_THRESH_BINARY );


            CvMatData outGray = CvMatData::create(in.width(), in.height(), 1);
            //cv::cvtColor( tmp, outGray, CV_RGB2GRAY );
            cv::cvtColor( tmpmat, distg, CV_RGB2GRAY );
			//cv::threshold( outGray, outGray, m_threshold, m_replacement, CV_THRESH_BINARY );
			cv::threshold( distg, distg, m_threshold, m_replacement, CV_THRESH_BINARY );
            m_outForeground->put(outGray);
        }
        else
        {
            CvMatData out = CvMatData::create(in.properties());
			cv::Mat& dst2 = out;
            //cv::absdiff( in, m_background, out );
            cv::absdiff( inmat, mbg, dst2 );
            
			//cv::threshold( out, out, m_threshold, m_replacement, CV_THRESH_BINARY );
            cv::threshold( dst2 , dst2 , m_threshold, m_replacement, CV_THRESH_BINARY );
			m_outForeground->put(out);
        }
        m_outBackground->put(m_background);
    }
    else
    {
        m_outForeground->put(in);
    }
    return true;
}

void BackgroundSubtractor::setBackground(CvMatData& mat)
{
    m_background = mat;
	cv::Mat& mbg = m_background;
	cv::Mat& bgmat = m_backgroundGray;

    if( m_background.channels() == 3 )
    {
		cv::cvtColor(mbg , bgmat, CV_RGB2GRAY );
        //cv::cvtColor(m_background, m_backgroundGray, CV_RGB2GRAY );
    }
    else
    {
        m_backgroundGray = m_background;
    }
}

void BackgroundSubtractor::setThreshold(int t)
{
    QMutexLocker lock(m_propertyMutex);
	
		
	//if (m_inInput
	if (m_background.depth() == 32)
	{

		//maxvalue int32 
		if( t >= 0 && t <= 2,147,483,647 )
		{
	        m_threshold = t;
		}
	}
	else 
	{
		if( t >= 0 && t <= 255 )
		{
	        m_threshold = t;
		}
	}
    emit thresholdChanged(m_threshold);
}

void BackgroundSubtractor::setReplacement(int r)
{
    QMutexLocker lock(m_propertyMutex);

    if( r >= 0 && r <= 255 )
    {
        m_replacement = r;
    }
    emit replacementChanged(m_replacement);
}

void BackgroundSubtractor::setReset(bool r)
{
    QMutexLocker lock(m_propertyMutex);
    m_reset = r;
    emit resetChanged(r);
}

void BackgroundSubtractor::setMethod(plv::Enum method)
{
    QMutexLocker lock(m_propertyMutex);
    m_method = method;
    emit methodChanged(method);
}

int BackgroundSubtractor::getThreshold() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_threshold;
}

int BackgroundSubtractor::getReplacement() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_replacement;
}

bool BackgroundSubtractor::getReset() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_reset;
}

plv::Enum BackgroundSubtractor::getMethod() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_method;
}
