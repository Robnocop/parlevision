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
  * uses part of http://answers.opencv.org/question/4815/cvcalcopticalflowlk-issue/
  * and http://docs.opencv.org/modules/video/doc/motion_analysis_and_object_tracking.html
  */



#include <QDebug>

#include "OpticalFlowLK.h"
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <qdebug.h>

using namespace plv;
using namespace plvopencv;


OpticalFlowLK::OpticalFlowLK() :
	m_minPoints(30),
	m_maxPoints(500),
	m_chgPoints(250),
	m_framesMovement(25),
	m_reset(false)
{
    m_inInput = createCvMatDataInputPin( "input image", this, IInputPin::CONNECTION_REQUIRED, IInputPin::CONNECTION_SYNCHRONOUS );
	m_inDelayed = createCvMatDataInputPin( "delayed image", this, IInputPin::CONNECTION_REQUIRED, IInputPin::CONNECTION_SYNCHRONOUS );
	m_inInputColor = createCvMatDataInputPin( "color image", this, IInputPin::CONNECTION_REQUIRED, IInputPin::CONNECTION_SYNCHRONOUS );
	m_visualisationPin = createCvMatDataOutputPin( "visualisation of vectors", this );
	m_sumXY = createOutputPin<QString>( "movement x and y", this );


	m_inInput->addSupportedChannels(1);
//    m_inInput->addSupportedChannels(3);
    m_inInput->addSupportedDepth(CV_8U);
//	m_inInput->addSupportedDepth(CV_16U);
//	m_inInput->addSupportedDepth(CV_32F);

	m_inDelayed->addSupportedChannels(1);
	m_inDelayed->addSupportedDepth(CV_8U);

	m_inInputColor->addSupportedChannels(3);
	m_inInputColor->addSupportedDepth(CV_8U);

}

OpticalFlowLK::~OpticalFlowLK()
{
}

bool OpticalFlowLK::init()
{
	//qDebug() << "poep";
	m_needToInit = true;
	m_addRemovePt = false;
	m_nrofpointstracked = 0;
	m_summationxlastframes.clear();
	m_summationylastframes.clear();
	return true;
}


bool OpticalFlowLK::process()
{
	//get the input images
    CvMatData in = m_inInput->get();
	CvMatData indelayed = m_inDelayed->get();
	CvMatData incolor = m_inInputColor->get();
	
	cv::Mat& inputimage = in;
	cv::Mat& indelayedimage = indelayed;
	cv::Mat& incolorimage = incolor;

	//create an empty writable output 
	CvMatData out;
	out = CvMatData::create(in.width(), in.height(), CV_8U, 3);
	cv::Mat& image = out;
	//to draw the interesting points on
	incolorimage.copyTo(image);
	
	//in demo the image is the input from the opencv cam that is then converted to grayscale
	
	//create the size of the searchwindow
	CvSize searchWindowSize;
	//todo make a GUI dependent value for this size
	searchWindowSize.width = 3;
	searchWindowSize.height = 3;
	//size as used in the lkdemo 
    CvSize subPixWinSize, winSize;
	subPixWinSize.width= subPixWinSize.height = 10; 
	winSize.width = winSize.height = 31;
	
	//(re)set other variables
	cv::TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03);
	//const int MAX_COUNT = 500;

	//set of points
	CvMatData flowX,flowY;
	flowX = CvMatData::create(in.width(),in.height(), CV_32F, 1);
	flowY = CvMatData::create(in.width(),in.height(), CV_32F, 1);
	
	//reset the total movement value every x seconds in order to keep the tracking reliably and not just recording the displacement of e.g. a wheelchair
	//todo make dependent on number of tracked points? on the other hand it might be lost more easy if the movement is quick
	//summation x is the difference
	float summationx =0.0f;
	float summationy =0.0f;
	float sumx,sumy=0;

	//do the optical flow see lines under code for settings and further explenation
	//if first time or otherwise need to create new featurepoints do this
	if( m_needToInit )
    {
		//reset the overal sum of movement
		//todo also reset if a certain time has elapsed, so we are actually measuring movement and not displacement. 
		

        // automatic initialization of interesting points
		//todo filter on a subarea for instance an area around a blob that is being tracked.
        goodFeaturesToTrack(inputimage, m_points[1], getMaxPoints(), 0.01, 10, cv::Mat(), 3, 0, 0.04);
        cornerSubPix(inputimage, m_points[1], subPixWinSize, cv::Size(-1,-1), termcrit);
        m_addRemovePt = false;
		//draw points that are found, will not be visible probably
		for( int i = 0; i < m_points[1].size(); i++ )
        {	
			circle( image, m_points[1][i], 5, cv::Scalar(0,255,255), -1, 8);
		}
		m_nrofpointstracked = m_points[1].size();

		//if needing no points but not first time this will set it to >0, no added value of this frame will be set 
		std::for_each(m_summationxlastframes.begin(),m_summationxlastframes.end(),[&](float n){
                        sumx += n;
		});
		std::for_each(m_summationylastframes.begin(),m_summationylastframes.end(),[&](float n){
                        sumy += n;
		});

	}
    else if( !m_points[0].empty() )
    {
		
        cv::vector<uchar> status;
        cv::vector<float> err;
        //in our setup an empty frame is impossible as parlevision will probably lag until it does have one.
		//if(indelayedimage.empty())
        //   gray.copyTo(prevGray);

        calcOpticalFlowPyrLK(indelayedimage, inputimage, m_points[0], m_points[1], status, err, winSize,
                                3, termcrit, 0, 0.001);

		//save the previously tracked number of points to act when too many points are dropped in one frame.
		m_nrofpointstracked = m_points[0].size();
		
		//status size and points0 size are indeed equal as could be expected
		//qDebug() << "status size" << status.size() << "size of points 0" << m_points[0].size();
		for( int j= 0; j< m_points[0].size(); j++)
		{
			//just to be sure check that size is smaller 
			if(j<< status.size())
			{
				//draw untracked points red;
				if (status[j] == 0)
					circle( image, m_points[0][j], 5, cv::Scalar(0,0,255), -1, 8);
				//sum the displacement of those that are moved.
				else 
				{
					//save the change of this as an indication of overall movement,
					//perhaps localie later and also save per point, not yet done
					summationx += m_points[1][j].x-m_points[0][j].x;
					summationy += m_points[1][j].y-m_points[0][j].y;
					//save a vector adding the direction of the last frame
					//m_points[2][j+1] = ;//m_points[1][j]-m_points[0][j];
				}
			}
		}

		
		//perhaps save the indication of movement both localised per point and overall
		/*for( int h=0; h<xmovement.size();h++)
		{
			  xmovement[h];
			 xmovement[h];
		}*/

		//used for for-loops
		size_t i, k;

        for( i = k = 0; i < m_points[1].size(); i++ )
        {
            //we do not yet use a mouse input to set a point
			/*if( m_addRemovePt )
            {
                if( norm(point - points[1][i]) <= 5 )
                {
                    addRemovePt = false;
                    continue;
                }
            }
*/
            if( !status[i] )
                continue;

            m_points[1][k++] = m_points[1][i];
			//in demo the image is the input from the opencv cam that is then converted to grayscale and these circles are drawn on it,
			//shows the interesting points and their position in green
			
			//thickness set to 2 instead of -1 and radius to 5
            circle( image, m_points[1][i], 5, cv::Scalar(0,255,0), 2, 8);
			//show where the points used to be in blue, thickness -1 is filled
			circle( image, m_points[0][i], 5, cv::Scalar(255,0,0), -1, 8);

        }
        m_points[1].resize(k);

		
		//remove the movement of the oldest frames until this hostory has the wished amount of frames
		while (m_summationxlastframes.size()>getFramesMovement())
		{
			m_summationxlastframes.pop_back();
			m_summationylastframes.pop_back();
			//stupid solution
			//m_framesCounter=0;
			//m_summationx =0.0f;
			//m_summationy =0.0f;
		}
		//save total movement of last frames, assume x and y are tightly linked and thus same size etc.
		m_summationxlastframes.push_front(summationx);
		m_summationylastframes.push_front(summationy);

		
		//sum the vectors
		std::for_each(m_summationxlastframes.begin(),m_summationxlastframes.end(),[&](float n){
                        sumx += n;
		});
		std::for_each(m_summationylastframes.begin(),m_summationylastframes.end(),[&](float n){
                        sumy += n;
		});

		//qDebug() << "sumx " << sumx << "sumy" << sumy;
    }
	else
	{
		qDebug() << "init but no points";
	}

	//we do not yet include this mousclick event to select points.
    /*if( m_addRemovePt && points[1].size() < (size_t)MAX_COUNT )
    {
        vector<Point2f> tmp;
        tmp.push_back(point);
        cornerSubPix( gray, tmp, winSize, cvSize(-1,-1), termcrit);
        points[1].push_back(tmp[0]);
        addRemovePt = false;
    }
	*/

	//if we end up here we do not need to init
	//unless the points are empty in which case it will always remain empty? so check on whether they are empty
    
	std::swap(m_points[1], m_points[0]);
	//in next loop need to find new points if there are none, if there are too little, or if the reset button is pushed.
	//check if more than allowed number of points have been lost since last frame 
	//change since init should be related directly to the minpoints.
	bool toomanypointslost = ((m_nrofpointstracked - m_points[0].size()) > getChgPoints());
	if (!m_points[0].empty() && m_points[0].size() > getMinPoints() && !toomanypointslost)
	{
		if (m_needToInit)
		{
			qDebug() << "succesfully added points" << m_points[0].size();		
		}
		m_needToInit = false;
	}
	else
	{
		m_needToInit = true;
		if (!m_points[0].empty())
			qDebug() << "need to init" << toomanypointslost << "chgpoints" << getChgPoints() << "pointsize" <<  m_points[0].size() << "nrofpoints" << m_nrofpointstracked;	
		else
			qDebug() << "need to init points are empty";	
	}
		// we use our own delayed image so this swap will not be needed
	//swap(prevGray, gray);

	//show the result
	//yellow are newly tracked feature points so after initialisation
	//untracked points are red
	//where the points used is filled blue
	//where the points are now is in green circles.
	m_visualisationPin->put(image);

	//TODO handle reset of values
	//now only put if there has been no reset as sumxy are not set than.
	QString str = QString::number(sumx).append("\t").append(QString::number(sumy));
	//if (sumx>0 && sumy>0)
	//if(!toomanypointslost)
	m_sumXY->put(str);
	//else
	//		m_sumXY->put("0 \t 0");

    return true;
}

//OPTICAL FLOW OPERATION:
//void calcOpticalFlowPyrLK(InputArray prevImg, InputArray nextImg, InputArray prevPts, InputOutputArray nextPts, OutputArray status, OutputArray err, Size winSize=Size(21,21), int maxLevel=3, TermCriteria criteria=TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01), int flags=0, double minEigThreshold=1e-4 )
//Parameters:	
//prevImg – first 8-bit input image or pyramid constructed by buildOpticalFlowPyramid().
//nextImg – second input image or pyramid of the same size and the same type as prevImg.
//prevPts – vector of 2D points for which the flow needs to be found; point coordinates must be single-precision floating-point numbers.
//nextPts – output vector of 2D points (with single-precision floating-point coordinates) containing the calculated new positions of input features in the second image; when OPTFLOW_USE_INITIAL_FLOW flag is passed, the vector must have the same size as in the input.
//status – output status vector (of unsigned chars); each element of the vector is set to 1 if the flow for the corresponding features has been found, otherwise, it is set to 0.
//err – output vector of errors; each element of the vector is set to an error for the corresponding feature, type of the error measure can be set in flags parameter; if the flow wasn’t found then the error is not defined (use the status parameter to find such cases).
//winSize – size of the search window at each pyramid level.
//maxLevel – 0-based maximal pyramid level number; if set to 0, pyramids are not used (single level), if set to 1, two levels are used, and so on; if pyramids are passed to input then algorithm will use as many levels as pyramids have but no more than maxLevel.
//criteria – parameter, specifying the termination criteria of the iterative search algorithm (after the specified maximum number of iterations criteria.maxCount or when the search window moves by less than criteria.epsilon.
//flags –
//operation flags:
//
//OPTFLOW_USE_INITIAL_FLOW uses initial estimations, stored in nextPts; if the flag is not set, then prevPts is copied to nextPts and is considered the initial estimate.
//OPTFLOW_LK_GET_MIN_EIGENVALS use minimum eigen values as an error measure (see minEigThreshold description); if the flag is not set, then L1 distance between patches around the original and a moved point, divided by number of pixels in a window, is used as a error measure.
//minEigThreshold – the algorithm calculates the minimum eigen value of a 2x2 normal matrix of optical flow equations (this matrix is called a spatial gradient matrix in [Bouguet00]), divided by number of pixels in a window; if this value is less than minEigThreshold, then a corresponding feature is filtered out and its flow is not processed, so it allows to remove bad points and get a performance boost

