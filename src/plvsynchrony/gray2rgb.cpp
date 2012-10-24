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

#include "gray2rgb.h"
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

using namespace plv;

Gray2RGB::Gray2RGB()
{
    m_inputPin = createCvMatDataInputPin( "input", this );
    m_outputPin = createCvMatDataOutputPin( "output", this );

    m_inputPin->addAllChannels();
    m_inputPin->addAllDepths();
}

Gray2RGB::~Gray2RGB()
{
}

bool Gray2RGB::process()
{
	assert(m_inputPin != 0);
    assert(m_outputPin != 0);

    CvMatData src = m_inputPin->get();

    // allocate a target buffer
    CvMatData target;
    target.create( src.width(), src.height(), src.type() );

    // do a flip of the image
    const cv::Mat in = src;
    cv::Mat out = target;
    cv::flip( in, out, 1);

    // publish the new image
    m_outputPin->put( out );


//   CvMatData in = m_inputPin->get();
    //const cv::Mat& inputImage = in;
    //m_outputPin->put( inputImage );
    return true;

}
