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

#include "Resize.h"

#include <plvcore/CvMatData.h>
#include <plvcore/Types.h>

#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

using namespace plv;
using namespace plvopencv;


Resize::Resize() :
	destinationWidth(STITCH_DESTINATION_WIDTH_DEFAULT_RESIZE),
    destinationHeight(STITCH_DESTINATION_HEIGHT_DEFAULT_RESIZE),
	interpolationtype(cv::INTER_LINEAR)
{
    m_imageInput = createCvMatDataInputPin( "image", this );
    m_outputPin = createCvMatDataOutputPin( "out", this );

    m_imageInput->addAllChannels();
    m_imageInput->addAllDepths();

	m_outputPin->addAllChannels();
	m_outputPin->addAllDepths();

	m_interpolation.add("bilinear interpolation", cv::INTER_LINEAR); //default first value
	m_interpolation.add("nearest-neighbor interpolation", cv::INTER_NEAREST);
	m_interpolation.add("resampling using pixel area relation.", cv::INTER_AREA);
	m_interpolation.add("bicubic over 4x4 pixel neighborhood", cv::INTER_CUBIC);
    m_interpolation.add("Lanczos over 8x8 pixel neighborhood", cv::INTER_LANCZOS4);
}

Resize::~Resize()
{
}

bool Resize::process()
{
    CvMatData img = m_imageInput->get();
    CvMatData outImg = CvMatData::create( getDestinationWidth(), getDestinationHeight(), img.depth(), img.channels() );
    
	// open input image for reading
    const cv::Mat& in = img;
	cv::Mat& dst = outImg;
	//resize(const Mat& src, Mat& dst, Size dsize, double fx=0, double fy=0, int interpolation=INTER_LINEAR)
	//The destination image size. If it is zero, then it is computed as:
	//The scale factor along the horizontal axis. When 0, it is computed as
	
	//double fx = (double) dst.width()/in.cols;
	//double fy = (double) dst.height()/in.rows;
	//cv::Size dsize = Size(round(fx*src.cols),round(fy*src.rows));
	
	cv::Size dsize = cv::Size(dst.cols,dst.rows);
	cv::resize(in, dst, dsize, 0, 0, interpolationtype);
    m_outputPin->put(outImg);
    return true;
}

int Resize::getDestinationHeight()
{
    return destinationHeight;
}

int Resize::getDestinationWidth()
{
    return destinationWidth;
}


void Resize::setDestinationHeight( int height )
{
    QMutexLocker lock( m_propertyMutex );
    if( height < 0 ) height = 0;
    if( height > STITCH_DESTINATION_HEIGHT_MAX_RESIZE ) height = STITCH_DESTINATION_HEIGHT_MAX_RESIZE;
    destinationHeight = height;
    emit destinationHeightChanged(height);

}

void Resize::setDestinationWidth( int width )
{
    QMutexLocker lock( m_propertyMutex );

    if( width < 0 ) width = 0;
    if( width > STITCH_DESTINATION_WIDTH_MAX_RESIZE ) width = STITCH_DESTINATION_WIDTH_MAX_RESIZE;
    destinationWidth = width;
    emit destinationWidthChanged(width);
}

void Resize::setInterpolation(plv::Enum e)
{
    m_interpolation = e;

    //clear the additional properties
    interpolationtype = e.getSelectedIndex();
	emit interpolationChanged(m_interpolation);
}


plv::Enum Resize::getInterpolation() const
{
	QMutexLocker lock(m_propertyMutex);
    return m_interpolation;
}