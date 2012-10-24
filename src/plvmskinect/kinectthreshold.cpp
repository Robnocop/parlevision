/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * All rights reserved.
  *
  * This file is part of the plvopencv module of ParleVision.
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

#include "kinectthreshold.h"
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

using namespace plv;
using namespace plvmskinect;

KinectThreshold::KinectThreshold() :
        m_threshold( 0 ),
        m_maxValue( 5000 )
{
    m_inputPin = createCvMatDataInputPin( "input", this,  IInputPin::CONNECTION_REQUIRED );
    m_outputPin = createCvMatDataOutputPin( "output", this );

    //PLV_ENUM_ADD( m_method, CV_THRESH_BINARY );
    //PLV_ENUM_ADD( m_method, CV_THRESH_BINARY_INV );
    //PLV_ENUM_ADD( m_method, CV_THRESH_TRUNC );
    //PLV_ENUM_ADD( m_method, CV_THRESH_TOZERO );
    //PLV_ENUM_ADD( m_method, CV_THRESH_TOZERO_INV );

    m_inputPin->addSupportedChannels(1);
    //m_inputPin->addSupportedDepth(CV_8U); //IR camera 
	m_inputPin->addSupportedDepth(CV_16U);
	//m_inputPin->addSupportedDepth(CV_32F);

    m_outputPin->addSupportedChannels(1);
	//m_inputPin->addSupportedDepth(CV_8U); //IR camera 
    m_outputPin->addSupportedDepth(CV_16U);
	//m_inputPin->addSupportedDepth(CV_32F);
}

KinectThreshold::~KinectThreshold(){}

bool KinectThreshold::process()
{
    CvMatData in = m_inputPin->get();
	//qDebug()<< "w " << in.width() << "h"<< in.height() << "type " << in.type(); // w  640 h 480 type  5  qdebug 
    CvMatData out = CvMatData::create( in.width(), in.height(), in.type() );

    const cv::Mat& src = in;
    cv::Mat& dst = out;

    // perform threshold operation on the image
	//CV_THRESH_TOZERO
    //cv::threshold( src, dst, m_threshold, m_maxValue, m_method.getSelectedValue() );
	//cv::threshold( src, dst, m_threshold, m_maxValue, CV_THRESH_TOZERO);
	//FOR A LIBFREENECT PIC IT ONLY RETURNED HALF OF THE PIC
    unsigned int threshold = m_threshold << 4; //bitshift <<
    unsigned int maxValue = m_maxValue << 4;

	bool toggle = false; 

    for( int y = 0 ; y < src.rows ; ++y )
    {
        for( int x = 0 ; x < src.cols ; ++x )
        {
            unsigned short v = src.at<unsigned short>(y,x);
            dst.at<unsigned short>(y,x) = v >= threshold ? (v > maxValue ? 0 : v) : 0;
			//if (toggle )
		//	//{
		//	//		qDebug() << "x is " << x  << "v is" << v;  //indeed 640 or 639 indeed similar values left and rigth of cut off x.
		//	//}

        }/*
		*if (y == 20)
			toggle = true;
		else 
			toggle = false;
		**////qDebug() << "x is " << y;  //indedd 480 or 479
    }

    // publish the new image
    m_outputPin->put( out );

    return true;
}

/** propery methods */
plv::Enum KinectThreshold::getMethod() const
{
    QMutexLocker lock( m_propertyMutex );
    return m_method;
}

int KinectThreshold::getThreshold() const
{
    QMutexLocker lock( m_propertyMutex );
    return m_threshold;
}

int KinectThreshold::getMaxValue() const
{
    QMutexLocker lock( m_propertyMutex );
    return m_maxValue;
}

void KinectThreshold::setMethod( plv::Enum m )
{
    QMutexLocker lock( m_propertyMutex );
    m_method = m;
    emit(methodChanged(m_method));
}

void KinectThreshold::setThreshold(int d)
{
    QMutexLocker lock( m_propertyMutex );
    m_threshold = d;
    emit(thresholdChanged(m_threshold));
}

void KinectThreshold::setMaxValue(int d)
{
    QMutexLocker lock( m_propertyMutex );
    m_maxValue = d;
    emit(maxValueChanged(m_maxValue));
}
