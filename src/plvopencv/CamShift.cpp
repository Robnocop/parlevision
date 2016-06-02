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
  *
  * uses part of http://docs.opencv.org/trunk/doc/py_tutorials/py_video/py_meanshift/py_meanshift.html
  */



#include <QDebug>

#include "CamShift.h"
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <qdebug.h>

using namespace plv;
using namespace plvopencv;


CamShift::CamShift() :
	m_reset(false),
	m_minXPoint(160),
	m_minYPoint(120),
	m_width(100),
	m_height(100)
{
    m_inInput = createCvMatDataInputPin( "input image", this, IInputPin::CONNECTION_REQUIRED, IInputPin::CONNECTION_SYNCHRONOUS );
	
	m_visualisationPin = createCvMatDataOutputPin( "visualisation of tracking", this );
	m_histPin = createCvMatDataOutputPin( "visualisation of historgram", this );
	m_cogRotatedRect = createOutputPin<QString>( "cog tracked window", this );

	m_inInput->addSupportedChannels(3);
//    m_inInput->addSupportedChannels(3);
    m_inInput->addSupportedDepth(CV_8U);
//	m_inInput->addSupportedDepth(CV_16U);
//	m_inInput->addSupportedDepth(CV_32F);

	//m_inDelayed->addSupportedChannels(3);
	//m_inDelayed->addSupportedDepth(CV_8U);

}

CamShift::~CamShift()
{
}

bool CamShift::init()
{
	m_trackObject = 0;
	
	//TODO selection of region is set with the values instead of clicking on it,
	//however would make sense to catch a selection in a seperate running popup,
	//perhaps make the existence of the popup dependent on another boolean
	//the reset will call the values clicked in the popup if they exist and are correct
	//
	m_selection.x = 160;
	m_selection.y = 120;
	m_selection.width = 100;
	m_selection.height = 100;
	
	//to set the trackwindow max
	m_maxwidth = 640;
	m_maxheight = 480;

	//add a method to select to be tracked thingy
	m_needToInit = false;
	
	//todo instead of checking it, now set hardcoded
	m_selectObject = false;
    if( m_selection.width > 0 && m_selection.height > 0 )
            m_trackObject = -1;
	
	//This was in the demo of opencv242, to me this seems  wrong, it was originaly used in the mouse handling of the m_selection
	//m_selection &= cv::Rect(0, 0, src.cols, src.rows);

	return true;
}

void CamShift::setReset(bool r)
{
	m_reset = r; 
	m_needToInit=true; 
	qDebug() << "reset";
	emit(resetChanged(r));
}

void CamShift::setHeight(int i)
{
	if(i>0 && i<(m_maxheight-getMinYPoint())) 
	{
		m_height = i; 
		emit (heightChanged(i));
	}
	else
	{
		qDebug() << "not allowed height";
	}
}

void CamShift::setWidth(int i)
{
	if(i>0 && i<(m_maxwidth-getMinXPoint())) 
	{
		m_width = i; 
		emit (widthChanged(i));
	}
	else
	{
		qDebug() << "not allowed width" << i;
	}
}




bool CamShift::process()
{
	//get the input images
    CvMatData in = m_inInput->get();
	m_maxwidth = in.width();
	m_maxheight = in.height();

	//CvMatData indelayed = m_inDelayed->get();
	
	cv::Mat& src = in;
	//cv::Mat& indelayedimage = indelayed;
	
	//create an empty writable output 
	CvMatData out;
	out = CvMatData::create(in.width(), in.height(), CV_8U, 3);
	cv::Mat& dst = out;

	//create an empty histogram cvmatdata
	CvMatData plvhist;
	//plvhist =  CvMatData::create(320, 200, CV_8U, 3);
	plvhist =  CvMatData::create(200, 320, CV_8U, 3);
	cv::Mat& histimg = plvhist;
	
	//settings
	bool backprojMode = false;
	bool showHist = true;

	//settings histogram
	int hsize = 16;
	float hranges[] = {0,180};
    const float* phranges = hranges;
	//done now set with GUI:
	//int vmin = 10, vmax = 256, smin = 30;


	//initialisation variables
	cv::Point origin;
	//to send the centerpoint of the tracked window, zero if not tracked correctly 
	cv::Point cogTrackedWindow;
	cogTrackedWindow.x= 0;
	cogTrackedWindow.y= 0;
	
	if (!src.empty())
	{

		//create ceveral cvmats
		cv::Mat hsv, hue, mask, backproj; 
		//histimg = cv::Mat::zeros(200, 320, CV_8UC3), //rows,cols
		
		//this converts to the hsv spectrum instead of the incoming bgr.
		cvtColor(src, hsv, CV_BGR2HSV);

		//in demo a region is selected manually
		//if( selection.width > 0 && selection.height > 0 )
				//trackObject = -1;

		//will an int of-1 also return true?
		if( m_trackObject )
		{
			//it returns this value
			//qDebug() <<"yes trackobject";
		}
		else
		{
			//qDebug() << "no trackobject";
		}
	
		int _vmin = getMinV(), _vmax = getMaxV();
		
		//Checks if array elements lie between the elements of two other arrays and sets to true if it is within the boundary.
		//saves to this mask of cv8_U type but actually binary values, 255(within bounds) or 0, 
		//size is the whole picture
		//dst(I ) = lowerb(I)0 <= src(I)0 <= upperb(I);
		inRange(hsv, cv::Scalar(0, getMinS(), MIN(_vmin,_vmax)),
				cv::Scalar(180, 256, MAX(_vmin, _vmax)), mask);
		
		//Copies specified channels from input arrays to the specified channels of output arrays.
		//seems to only take the first value the hue
		int ch[] = {0, 0};
		hue.create(hsv.size(), hsv.depth());
		mixChannels(&hsv, 1, &hue, 1, ch, 1);

		//ADDED to prevent the error of camshift with empty window.
		if (!m_trackWindow.area()>0)
		{
			qDebug() << "I have an empty trackwindow and will reinit";
			m_needToInit =true;
		}
		
		//if a new region is selected or has to be set
		if( m_trackObject < 0 || m_needToInit)
		{
			qDebug() << "I am in the progress of reiniting";

			m_selection.x= getMinXPoint();
			m_selection.y= getMinYPoint();
			m_selection.width = getWidth();
			m_selection.height = getHeight();
						
			//create a mat roi, with selection from hue which is channel swapped and converted from the src image
			cv::Mat roi(hue, m_selection), maskroi(mask, m_selection);

			qDebug()<< "I will recalc the histogram";
			calcHist(&roi, 1, 0, maskroi, m_hist, 1, &hsize, &phranges);
			normalize(m_hist, m_hist, 0, 255, CV_MINMAX);

			m_trackWindow = m_selection;
			m_trackObject = 1;

			histimg = cv::Scalar::all(0);
			int binW = histimg.cols / hsize;
			cv::Mat buf(1, hsize, CV_8UC3);
			for( int i = 0; i < hsize; i++ )
			{
				buf.at<cv::Vec3b>(i) = cv::Vec3b(cv::saturate_cast<uchar>(i*180./hsize), 255, 255);
			}
			cvtColor(buf, buf, CV_HSV2BGR);

			for( int i = 0; i < hsize; i++ )
			{
				int val = cv::saturate_cast<int>(m_hist.at<float>(i)*histimg.rows/255);
				rectangle( histimg, cv::Point(i*binW,histimg.rows),
							cv::Point((i+1)*binW,histimg.rows - val),
							cv::Scalar(buf.at<cv::Vec3b>(i)), -1, 8 );
			}

			m_needToInit = false;
		}

		if(!m_hist.empty())
		{
			
			//seems to be always true
			//TODO it seems as if the backproj is always calculated however an on reset only approach seems better to me
			calcBackProject(&hue, 1, 0, m_hist, backproj, &phranges);
		}
		else
		{
			qDebug() << "hist is empty!";
		}

		//cpp syntax:
		//// This is a combination bitwise AND and assignment.
		////myVar &= var;
		//// The above is equivalent to
		////myVar = myVar & var;

		//mask is simply a mask (0or255) if hsv image is within normal bounds at this time, if it is not reset in this frame
		//it will also 
		backproj &= mask;

		//TODO camshift.cpp:80: error: (-5) Input window has non-positive sizes 
		cv::RotatedRect trackBox = cv::CamShift(backproj, m_trackWindow,
							cv::TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));

		//TODO why is this here anyway?
		if( m_trackWindow.area() <= 1 )
		{
			qDebug() << "mtrackwindow was <1";
			int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
			m_trackWindow = cv::Rect(m_trackWindow.x - r, m_trackWindow.y - r,
								m_trackWindow.x + r, m_trackWindow.y + r) &
							cv::Rect(0, 0, cols, rows);
		}
		else
		{
			cogTrackedWindow.x= trackBox.center.x;
			cogTrackedWindow.y= trackBox.center.y;
		}
				

		//the backprojemode is also off, It seems to me it would be unlikely to benefit from the converted backprojected image ... 
		if( backprojMode )
			cvtColor( backproj, src, CV_GRAY2BGR );
        
		//draw the actual tracked (rotated) Box in red
		src.copyTo(dst);
		ellipse( dst, trackBox, cv::Scalar(0,0,255), 3, CV_AA );
		
		//draw a rectangle of the window of initialisation.
		rectangle(	dst, 
					cv::Point(m_selection.x, m_selection.y),
					cv::Point(m_selection.x+m_selection.width,m_selection.y+m_selection.height),
					cv::Scalar(255,0,0),
					4, 8 );
			
		//draw a rectangle of the target window for a future initialisation on reset.
		rectangle(	dst, 
					cv::Point(getMinXPoint(), getMinYPoint()),
					cv::Point(getMinXPoint()+getWidth(),getMinYPoint()+getHeight()),
					cv::Scalar(0,255,0),
					4, 8 );
			

		//} //end of m_trackobject
	
		
		if( m_selectObject && m_selection.width > 0 && m_selection.height > 0 )
		{
			cv::Mat roi(src, m_selection);
			bitwise_not(roi, roi);
			//ADDED seems reasonable to reset after this, if needed at all
			m_selectObject = false;
		}
	}
	else
	{
		qDebug() << "src is empty";
	}
	

	m_visualisationPin->put(dst);
	m_histPin -> put(plvhist);

	//output the center of gravity of the trackedwindow. will return zero if no area is tracked
	QString str = QString::number(cogTrackedWindow.x).append("\t").append(QString::number(cogTrackedWindow.y));
	//if (sumx>0 && sumy>0)
	//if(!toomanypointslost)
	m_cogRotatedRect->put(str);

    return true;
}
