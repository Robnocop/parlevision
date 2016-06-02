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

#include "PixelSumS.h"
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>

using namespace plv;

PixelSumS::PixelSumS()
{
    m_inputPin = createCvMatDataInputPin( "input", this );
    m_outputPin = createOutputPin<double>( "output", this );
	m_outputPin2 = createOutputPin<QString>( "sum in string format", this );

    m_inputPin->addAllChannels();
    m_inputPin->addAllDepths();
}

PixelSumS::~PixelSumS()
{
}

bool PixelSumS::process()
{
    CvMatData in = m_inputPin->get();
	//in.type();
    const cv::Mat& inputImage = in;
    cv::Scalar sumout = cv::sum(inputImage);
    
	//only need the first tuple of the four in the form of a double as this contains the actual value
    //TODO need to address for the incoming image type, we can assume it is BW but we are not sure
	double sumoutd = sumout[0]/255;
	m_outputPin->put( sumoutd);
	QString doubletostring= QString("%1").arg(sumoutd);
	m_outputPin2->put(doubletostring);
    return true;

    //original sumpixels from Richard Loos
//    const cv::Mat& src = in;
//    // tuple of 1,2,3 or 4 values depending on the number of channels
//    cv::Scalar scalar = cv::sum( src );
//    m_outputPin->put( scalar );
//    return true;

}
