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

#include "VideoProducer.h"

#include <QMutexLocker>
#include <QDebug>


#include <opencv/highgui.h>
#include <string>
#include <plvcore/CvMatDataPin.h>
#include <QDir>

using namespace plv;
using namespace plvopencv;

VideoProducer::VideoProducer() :
    m_filename(""), m_directory(""), m_frameCount(0), m_posMillis(0), m_ratio(0), m_fps(0), m_prevtime(0)
{
    //create the output pin
    m_outputPin = createCvMatDataOutputPin("image_output", this );
    m_outputPin->addAllChannels();
    m_outputPin->addAllDepths();

    m_outFrameCount = createOutputPin<int>("frame count", this);
    m_outPositionMillis = createOutputPin<long>("milli seconds", this);
    m_outRatio = createOutputPin<double>("ratio", this);
    m_outFps = createOutputPin<int>("fps", this);
}

VideoProducer::~VideoProducer()
{
}

void VideoProducer::setFilename(const QString& filename)
{
    //Validate the filename
    if( filename.length() < 4 || !VideoProducer::validateExtension(filename) )
    {
        qDebug() << tr("Invalid filename %1.").arg(filename);
        return;
    }

    QMutexLocker lock( m_propertyMutex );
    m_filename = filename;
    qDebug() << "New filename selected: " << m_filename;
    return;
}

QString VideoProducer::getFilename()
{
    QMutexLocker lock( m_propertyMutex );
    return m_filename;
}

void VideoProducer::setDirectory(const QString& directory)
{
    //replace all '\' characters with '/' characters
    QString directoryCopy = directory;
    directoryCopy.replace('\\','/');

    //validate the directory
    QDir dir(directory);
    if( dir.exists() )
    {
        if(!directoryCopy.endsWith('/')) directoryCopy.append('/');
		QMutexLocker lock( m_propertyMutex );
		m_directory = directoryCopy;
        qDebug() << "New directory selected:" << m_directory;
    }
}

QString VideoProducer::getDirectory()
{
    QMutexLocker lock( m_propertyMutex );
    return m_directory;
}

bool VideoProducer::validateExtension(const QString& filename)
{
	//this doesnt seem to be right!
    return true;
}

bool VideoProducer::init()
{
    //create the m_capture here
	m_capture =  new cv::VideoCapture(); 
	m_fpstimer.start();	
	m_prevtime = 0;
	QString pathorg = getDirectory();
	pathorg.append(m_filename);
	QFile testfile = pathorg;
	
	const std::string path = pathorg.toStdString(); //"D:/videos/work/showcase-rapper2.avi";
	const char * path2 = path.c_str();

	////build the path use get instead
 //   QString path =  getDirectory();
	////should have been in set if(!directoryCopy.endsWith('/')) directoryCopy.append('/');
 //   path.append(m_filename);
	////const std::string path2 = path.toStdString();
	//const char * path2 = path.toStdString().c_str();
	//QFile testfile = path;
	
	
	if (testfile.exists())
	{
		qDebug() <<"checking file... file exists";
	} else
	{
		qDebug() << "checking file... file does not exist";
	}

	//temp reminder ignore library set to off/no in properties

	//m_capture = cv::VideoCapture::open(path);
    //if(!m_capture->open(path.toStdString()))
	m_capture->open(path2);
	//if(!m_capture->open(path2))
	if(!m_capture->isOpened())
    {
        qDebug() <<"Failed to open video during init";
		setError(PlvPipelineInitError, tr("Failed to open video, check type of avi of %1 reencode if neccesary we advise winff or ffmpeg").arg(pathorg));
        return false;
    }

	
	//don't think it should be here as it not yet grabbed only opened
    m_frameCount = (int)m_capture->get(CV_CAP_PROP_FRAME_COUNT);
    m_posMillis = (long)m_capture->get(CV_CAP_PROP_POS_MSEC);
    m_ratio = m_capture->get(CV_CAP_PROP_POS_AVI_RATIO);
    m_fps = (int)m_capture->get(CV_CAP_PROP_FPS);
    
	return true;
}

bool VideoProducer::deinit() throw()
{
    m_capture->release();
	return true;
}

bool VideoProducer::readyToProduce() const
{
	//qDebug()<< "goingtoproduce" <<"with grab" ;
	//m_capture->grab();
	//qDebug()<< "grabbed" ;
	//m_capture->grab();
	return true;
}

bool VideoProducer::produce()
{
	//try to combine grab and retrieve using query maybe that works
	//m_capture->query(m_frame); 
	//not opencv 2.1 : http://opencv.willowgarage.com/documentation/cpp/highgui_reading_and_writing_images_and_video.html?highlight=videocapture#VideoCapture::VideoCapture

	if( !m_capture->grab() )
    {
		//it is an expected error
		
        setError(PlvPipelineRuntimeError, tr("Failed to grab frame"));
        qDebug() <<"Failed to grab frame";
		setState(PLE_ERROR);
		//added to stop after last frame?
		this->__stop();
		return false;
    }

    if( !m_capture->retrieve(m_frame) ) //, 0
    {
        setError(PlvPipelineRuntimeError, tr("Failed to retrieve frame"));
        qDebug() << "Failed to retrieve frame";
		//stop();
		setState(PLE_ERROR);
		this->__stop();
		return false;
    }
	
	//Why does it need to be copied? ahh it may not be altered!
    CvMatData d = CvMatData::create(m_frame.properties());
    //m_frame.copyTo(d);
	d = m_frame;
    m_outputPin->put(d);

    m_frameCount = (long)m_capture->get(CV_CAP_PROP_FRAME_COUNT);
    m_posMillis = (long)m_capture->get(CV_CAP_PROP_POS_MSEC);
    m_ratio = m_capture->get(CV_CAP_PROP_POS_AVI_RATIO);
    m_fps = (int)m_capture->get(CV_CAP_PROP_FPS);

	int serial = getProcessingSerial();
	
    m_outFrameCount->put(m_frameCount);
    m_outPositionMillis->put(m_posMillis);
    m_outRatio->put(m_ratio);
    m_outFps->put(m_fps);
		
	//if(boolean limit fps)
	if(m_fps>0 && fpsLimit)
	{
		//empiracly about 2-4 milliseconds needed for remaining calculations, might depend on system?
		while ((m_fpstimer.elapsed()-m_prevtime) < 1000/m_fps)
		{
			//qDebug() << "elaps-prev" << m_fpstimer.elapsed()-m_prevtime << "boolean is " << (m_fpstimer.elapsed()-m_prevtime < 1/m_fps);
		}
	}
	m_prevtime = m_fpstimer.elapsed() ;
	
	//test if pipeline can be stopped not with this->__stop();
	//if (m_fpstimer.elapsed()>3000)
	//{
	//	this->__stop();
	//}

    return true;
}
