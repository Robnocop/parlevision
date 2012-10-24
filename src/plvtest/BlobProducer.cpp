/**
  * Copyright (C)2011 by Richard Loos
  * All rights reserved.
  *
  * This file is part of the plvtest plugin of ParleVision.
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

#include "BlobProducer.h"

#include <QDebug>
#include <plvcore/Pin.h>

using namespace plv;

BlobProducer::BlobProducer() :
    m_width(640),
    m_height(480),
	m_blobSize(50),
    m_maxStep(1), //10
    m_numBlobs(3),//10
	m_pixelShift(0)
{
    m_outputPin = createCvMatDataOutputPin( "output", this );

    m_outputPin->addSupportedChannels(1);
    m_outputPin->addSupportedDepth(CV_8U);
}

BlobProducer::~BlobProducer()
{
}

bool BlobProducer::init()
{
    m_positions = QVector<cv::Point>(m_numBlobs, cv::Point(0,0));
    m_targets   = QVector<cv::Point>(m_numBlobs, cv::Point(m_width/2, m_height/2));
    m_factor = (m_maxStep / (double)RAND_MAX);
	m_timeSinceLastFPSCalculation.start();
	m_colorp = 255;
    return true;
}

bool BlobProducer::readyToProduce() const
{
    return true;
}

bool BlobProducer::produce()
{
	//qDebug() << "i produce";
    CvMatData out = CvMatData::create(640,480,CV_8UC1);
    cv::Mat& img = out;
    img = cv::Scalar::all(0);

	
	++m_numFramesSinceLastFPSCalculation;
	int elapsed = m_timeSinceLastFPSCalculation.elapsed();
	
    for( int i=0; i < m_numBlobs; ++i )
    {
		//get a colorcode based on frames elapsed
		m_colorp = getColor(i, elapsed);
        cv::Point& pos = m_positions[i];
        cv::Point& target = m_targets[i];

        // 1 when left, -1 when right
        double xBias = (target.x - pos.x);
        xBias = xBias < 0 ? -1.0 : 1.0;

        // 1 when top, -1 when bottom
        double yBias = (target.y - pos.y);
        yBias = yBias < 0 ? -1.0 : 1.0;

        pos.x += qrand() * m_factor * xBias;
        pos.y += qrand() * m_factor * yBias;

        pos.x = qMin(pos.x, img.cols);
        pos.y = qMin(pos.y, img.rows);

        if( abs(pos.x - target.x) < 5 )
        {
            target.x = (qrand()/(double)RAND_MAX) * img.cols;
        }

        if( abs(pos.y - target.y) < 5 )
        {
            target.y = (qrand()/(double)RAND_MAX) * img.rows;
        }

		//Robby's edit TODO we want to test a morse like-based ID-blob tracker so needed to alter the 255 255 255 value
        //cv::circle(img, pos, 10, cv::Scalar(m_colorp,m_colorp,m_colorp), CV_FILLED, CV_AA );
		//edit 2 we need a blob and three lights within this blob
		cv::circle(img, pos, getBlobSize(), cv::Scalar(100,100,100), CV_FILLED, CV_AA );
		//pos.x =pos.x +10;
		//pos.y =pos.y +10;

		//PIXEL SHIFT FOR DIRECTION OF LEDBLOB! (removed pixxelshift 0 for a state)
		if (getPixelShift()!= 0)
		{
		//side 20pixels height 40 correct
		//upper left corner
			if (pos.x>10 && pos.y > 10)
			{
				cv::circle(img, cv::Point(pos.x-10,pos.y-10), 5, cv::Scalar(m_colorp,m_colorp,m_colorp), CV_FILLED, CV_AA );
			}
			//upper right corner
			if (pos.x< (img.cols-10) && pos.y>10 )
			{
				cv::circle(img, cv::Point(pos.x+10,pos.y-10+getPixelShift()), 5, cv::Scalar(m_colorp,m_colorp,m_colorp), CV_FILLED, CV_AA );
			}
			//bottom center
			if (pos.y< (img.rows-30))
			{
				cv::circle(img, cv::Point(pos.x,pos.y+30), 5, cv::Scalar(m_colorp,m_colorp,m_colorp), CV_FILLED, CV_AA );
			}

			//random point
			if (i==0)
			{
				if (pos.x>10 && pos.y > 10)
				{
					cv::circle(img, cv::Point(pos.x-30+getPixelShift()*2,pos.y-10), 5, cv::Scalar(m_colorp,m_colorp,m_colorp), CV_FILLED, CV_AA );
				}
			}

		}
		//pos.x =pos.x -20;
		//cv::circle(img, pos, 10, cv::Scalar(m_colorp,m_colorp,m_colorp), CV_FILLED, CV_AA );
		//pos.x =pos.x +20;
		//pos.y =pos.y -30;
		//cv::circle(img, pos, 10, cv::Scalar(m_colorp,m_colorp,m_colorp), CV_FILLED, CV_AA );
    }

	
	m_outputPin->put(out);
	
	//get the FPS and reset the elapsed so we have a three (bit) second code.
	if( elapsed > 3500 ) //10000
	{
		// add one so elapsed is never 0 and
		// we do not get div by 0
		m_fps = (m_numFramesSinceLastFPSCalculation * 1000) / elapsed;
		qDebug() << "FPS in BlobProd: " << (int)m_fps << "elapsed time "<< (int) elapsed;
		//m_fps = m_fps == -1.0f ? fps : m_fps * 0.9f + 0.1f * fps;
		m_timeSinceLastFPSCalculation.restart();
		m_numFramesSinceLastFPSCalculation = 0;
		
		//emit framesPerSecond(m_fps);
	}
    return true;
}

int BlobProducer::getColor(int blobid, int elapsedtime) 
{
	int colorValue = 50;
	switch (blobid)
	{
		case 0:
			if ( elapsedtime <500)
				colorValue = 150;
			else if (elapsedtime <1000)
				colorValue = 150;
			else if (elapsedtime <1500)
				colorValue = 150;
			else if (elapsedtime <2000)
				colorValue = 150;
			else if (elapsedtime <2500)
				colorValue = 150;
			else if (elapsedtime <3000)
				colorValue = 150;
			else if (elapsedtime <3500)
				colorValue = 255;
			break;
		case 1:
			if ( elapsedtime <500)
				colorValue = 150;
			else if (elapsedtime <1000)
				colorValue = 150;
			else if (elapsedtime <1500)
				colorValue = 150;
			else if (elapsedtime <2000)
				colorValue = 150;
			else if (elapsedtime <2500)
				colorValue = 150;
			else if (elapsedtime <3000)
				colorValue = 255;
			else if (elapsedtime <3500)
				colorValue = 255; //255
			break;
		case 2:
			if ( elapsedtime <500)
				colorValue = 150;
			else if (elapsedtime <1000)
				colorValue = 150;
			else if (elapsedtime <1500)
				colorValue = 150;
			else if (elapsedtime <2000)
				colorValue = 150;
			else if (elapsedtime <2500)
				colorValue = 255;
			else if (elapsedtime <3000)
				colorValue = 150;
			else if (elapsedtime <3500)
				colorValue = 255;
			break;
	}
	return colorValue;
	
}

int BlobProducer::getMaxStep() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_maxStep;
}

void BlobProducer::setMaxStep(int step)
{
    QMutexLocker lock(m_propertyMutex);
    if( step >= 0 )
    {
        m_maxStep = step;
        m_factor = (m_maxStep / (double)RAND_MAX);
    }
    emit maxStepChanged(m_maxStep);
}

int BlobProducer::getBlobSize() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_blobSize;
}


void BlobProducer::setBlobSize(int pixel)
{
    QMutexLocker lock(m_propertyMutex);
	if(pixel<200&& pixel > 0)
		m_blobSize = pixel;
	emit blobSizeChanged(m_blobSize);
}

int BlobProducer::getPixelShift() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_pixelShift;
}

void BlobProducer::setPixelShift(int pixel)
{
    QMutexLocker lock(m_propertyMutex);
	if(pixel<40&& pixel > -15)
		m_pixelShift = pixel;
	emit pixelShiftChanged(m_pixelShift);
}


int BlobProducer::getNumBlobs() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_numBlobs;
}

void BlobProducer::setNumBlobs(int num)
{
    QMutexLocker lock(m_propertyMutex);
    if( num >= 0 )
    {
        if( num > m_numBlobs )
        {
            m_positions.resize(num);
            m_targets.resize(num);
            for( int i=m_numBlobs; i < num; ++i )
            {
                m_positions[i] = cv::Point(0,0);
                m_targets[i]   = cv::Point(m_width/2, m_height/2);
            }
        }
        m_numBlobs = num;
    }
    emit numBlobsChanged(m_numBlobs);
}
