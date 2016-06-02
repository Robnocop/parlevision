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
    CvMatData in = m_inInput->get();
	int depthoforg = in.depth();
	//qDebug() << "depth"<< in.depth() << "tostring" << in.depthToString(in.depth());
	
	//added
	//cv::Mat& bgmat = m_backgroundGray;
	
	//does not seem to return the actual depth
	//qDebug() << "depth "<< m_background.depth() << "actualdetpth"<< m_background.depthToString(m_background.depth()) << "depth to string 1" << m_background.depthToString(1) << "depth to string 2" << m_background.depthToString(2) <<"depth to string 3" << m_background.depthToString(3);
    
	// m_reset is true on initialisation
    // so the background is always initialized
    // to the first frame received
    if( m_reset && m_inBackground->hasData())
    {
        //added
		
		CvMatData inbg = m_inBackground->get();
		//renamed instead of in use inbg right?
		qDebug() << "depthbackground" << inbg.depthToString(inbg.depth()) << "not tostring" << inbg.depth() ;
		setBackground(inbg, depthoforg);
		setReset(false);
    }
	else if( m_inBackground->isConnected() && m_inBackground->hasData() )
    {
		//this will always be true (in most cases ;) )qDebug() << "i am connected and have data";
        //TODO add a selection whether to reset the background every frame, waste of processing and memory in most cases
		CvMatData inbg = m_inBackground->get();
        setBackground(inbg, depthoforg);
    }

    if( m_inReset->isConnected() && m_inReset->hasData() )
    {
		qDebug() << "i am reset and reset is supposed to have data";
        bool value = m_inReset->get();
        if( value )
        {
			CvMatData inbg = m_inBackground->get();
			setBackground(inbg, depthoforg);
            //setBackground(in);
        }
    }

	//due some strange changes ?I? made opencv function need a cv::mat instead of cvmatdata.
	cv::Mat& inmat = in;

    if( !m_background.isEmpty() )
    {
		//check format of images
		if( in.width() != m_background.width() || in.height() != m_background.height() )
        {
            qDebug() << "images do not have the same size" ;
			//maybe TODO add a selection to indicate whether resizing the background image is prefered
			cv::Mat& resize = m_background;
			cv::Mat resizedist = inmat;
			//interpolation is kept on default setting, resize background based on the input frame 
			cv::resize(resize, resizedist, inmat.size(), 0, 0);
			CvMatData resized = CvMatData::create(resizedist.rows, resizedist.cols, 1);
			resized = resizedist;
			setBackground(resized, depthoforg);
        }

		//check if resizing worked if the function of resizing becomes optional so does this, now it is needed and should throw a stop pipeline.
		if( in.width() != m_background.width() || in.height() != m_background.height() )
		{
            QString msg = tr("Images do still not have the same size.");
			setError(PlvPipelineRuntimeError, msg);
			
			return false;
		}

		// TODO reattach the difference either absdiff over grayscale or over colour pics see org code!
		//STILL doesn't work!
		if( m_method.getSelectedValue() == BGSM_GRAYSCALE )
        {
			CvMatData srcGray = CvMatData::create(in.width(), in.height(), 1);
			CvMatData outGray = CvMatData::create(in.width(), in.height(), 1);
			
			cv::Mat& srcGraymat = srcGray;
			cv::Mat& distGray = outGray;
			cv::Mat& mbg = m_background;
			
		
			////?shouldn't be here
            if( in.channels() == 3 )
            {
               // cv::cvtColor( in, srcGray, CV_RGB2GRAY );
				//src,dst,coler code conversion ,dstcn
				cv::cvtColor( inmat, srcGraymat, CV_RGB2GRAY );
			}
            else
            {
				srcGraymat = in;
			}

				//a CV_16U ==2 
			// converttto( dstmat. desttype, optional scale, optional delta
			if (depthoforg ==2)
			{
				//cv::Mat depth32;
				//float scaleFactor = 1.0;
				//srcGraymat.convertTo(depth32, CV_32F, scaleFactor);
				//will this work? no it doesnt apparently
				//srcGraymat = depth32;
				//TODO change back to a float scaling as it throws away valuable info at the moment. however the float scaling did not work yet. this does 
				CvMatData depthTypeChange;
				depthTypeChange = CvMatData::create(srcGraymat.cols, srcGraymat.rows, CV_8U, 1);
				cv::Mat& dTC = depthTypeChange;
				cv::convertScaleAbs(srcGraymat, dTC, 0.00390625, 0);
				srcGraymat = dTC;
				//qDebug() << "depth is " << srcGraymat.depth() << "no? "<< depthoforg;
			}
			else if (depthoforg != 0)
			{
				cv::Mat depth8u;
				srcGraymat.convertTo(depth8u, CV_8U);
				//will this work?
				srcGraymat = depth8u;
			}

            //cv::absdiff( srcGray, m_backgroundGray, outGray );
			//changed m_backgroundgray to cv::mat
            //this seems more logical cv::absdiff( srcGraymat, m_backgroundGray, distGray );
			cv::absdiff( srcGraymat, mbg, distGray );
			
			//this function only takes eight bit unsigned or 32_F arrays
			//cv::threshold( outGray, outGray, m_threshold, m_replacement, CV_THRESH_BINARY );
            cv::threshold( distGray , distGray, m_threshold, m_replacement, CV_THRESH_BINARY );
            
			//m_outForeground->put(outGray);
			m_outForeground->put(distGray);
        }
        else if( m_method.getSelectedValue() == BGSM_COLOR_TO_GRAY )
        {
            CvMatData tmp = CvMatData::create(in.properties());
            cv::Mat& tmpmat = tmp;

			cv::Mat& mbg = m_background;
			//never used
			//cv::Mat& bgmat = m_backgroundGray;
			
			cv::absdiff( inmat, mbg, tmpmat  );
			cv::threshold( tmpmat, tmpmat, m_threshold, m_replacement, CV_THRESH_BINARY );
			
            CvMatData outGray = CvMatData::create(in.width(), in.height(), 1);
			cv::Mat& distGray = outGray;
            
            cv::cvtColor( tmpmat, distGray, CV_RGB2GRAY );
			
			cv::threshold( distGray, distGray, m_threshold, m_replacement, CV_THRESH_BINARY );
            m_outForeground->put(distGray);
        }
        else
        {
			//changed to prevent colour output!
			//CvMatData out = CvMatData::create(in.properties());
	        CvMatData out = CvMatData::create(in.width(), in.rows(), CV_8U, 1);
			cv::Mat& dst2 = out;
			cv::Mat& mbg = m_background;
            cv::absdiff( inmat, mbg, dst2 );
            cv::threshold( dst2 , dst2 , m_threshold, m_replacement, CV_THRESH_BINARY );
			m_outForeground->put(dst2);
        }
		//
        m_outBackground->put(m_background);
    }
    else
    {
		//changed
		//this would perhaps result in color:
		//m_outForeground->put(in);
		CvMatData outGray = CvMatData::create(in.width(), in.height(), 1);
		cv::Mat& distGray = outGray;
		
		m_outBackground->put(m_background);
    }
    return true;
}

void BackgroundSubtractor::setBackground(CvMatData& mat, int depthorg)
{
    m_background = mat;
	cv::Mat& mbg = m_background;
	//more logical cv::Mat& bgmat = m_backgroundGray;

	if (m_background.depth() == CV_16U)
	{
		//CV_8U ==0, CV_16U ==2 
		//impossible right ==0  so changed to ==1 ?cv_32_f
		if(depthorg == CV_32F)
		{
			cv::Mat depth32;
			float scaleFactor = 1.0;
			mbg.convertTo(depth32, CV_8U, scaleFactor);
		} else 
		{
			CvMatData depthTypeChange;
			depthTypeChange = CvMatData::create(mbg.cols, mbg.rows, CV_8U, 1);
			cv::Mat& dTC = depthTypeChange;
			//1/2^8
			cv::convertScaleAbs(mbg, dTC, 0.00390625,0.0);
			mbg = dTC;
			qDebug() << "depth of background is " << mbg.depth() << "no? "<< depthorg;

			/*cv::Mat depth32;
			float scaleFactor = 1.0;
			mbg.convertTo(depth32, CV_32F, scaleFactor);*/
		}
	}

    if( m_background.channels() == 3 )
    {
		cv::Mat bgmat;
		cv::cvtColor(mbg , bgmat, CV_RGB2GRAY );
      	m_background = bgmat;
		//more logical m_backgroundGray = bgmat; 
    }
    else
    {
        //more logical m_backgroundGray = mat;
		m_background = mbg;
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
