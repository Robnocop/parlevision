/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * Changes by Robby van Delden
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
    m_filename(""), m_directory(""), m_frameCount(0), m_posMillis(0), m_ratio(0), m_fps(0), m_prevtime(0), m_fpsValue(25), m_skipFirstXFrames(0)
{
    //create the output pin
    m_outputPin = createCvMatDataOutputPin("image_output", this );
    m_outputPin->addAllChannels();
    m_outputPin->addAllDepths();

    m_outFrameCount = createOutputPin<int>("frame count", this);
    m_outPositionMillis = createOutputPin<long>("milli seconds", this);
    m_outRatio = createOutputPin<double>("ratio", this);
    m_outFps = createOutputPin<int>("fps-info", this);
	m_continueProduce = true;
}

VideoProducer::~VideoProducer()
{
}


bool VideoProducer::validateExtension(const QString& filename)
{
	QStringList parts = filename.split(".");
    if (parts.length()>1) 
	{
		QString fileExtension = parts.at(1);
		//qDebug() << fileExtension ;
		//these are not verified with their most likely codecs
		if (!(fileExtension=="avi" || fileExtension=="vob" || fileExtension=="tod" || 
			fileExtension=="mkv" || fileExtension=="mpeg" || fileExtension=="mpg" ||
			fileExtension=="mp4" || fileExtension=="mts"
			//compare case insensitive, could also be done with QString::compare( &other, Qt::CaseInsensitive)
			|| fileExtension=="AVI" || fileExtension=="VOB" || fileExtension=="TOD" || 
			fileExtension=="MKV" || fileExtension=="MPEG" || fileExtension=="MPG" ||
			fileExtension=="MP4" || fileExtension=="MTS"
			
			) 
			)
			//non-suitable extenstions could have been tested instead?
			//|| fileExtension=="flv" || fileExtension=="mov" ||
			//fileExtension=="ogm" ) 
		{
			qDebug() << "improper extension was " << fileExtension;
			return false;
		}
		else
		{

			qDebug() << "proper extension was " << fileExtension;

		}
	}

	//this doesnt seem to be right just don't check it.
    return true;
}

bool VideoProducer::init()
{
	
	qDebug() <<"init videoproducer";
	m_continueProduce = true;
	m_skippedFrames = false;

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

	
	
    //no longer the total but just the current; 
	m_totalFrames = (int)m_capture->get(CV_CAP_PROP_FRAME_COUNT);
	//don't think it should be here as it not yet grabbed only opened
	m_frameCount = (int)m_capture->get(CV_CAP_PROP_POS_FRAMES);
    m_posMillis = (long)m_capture->get(CV_CAP_PROP_POS_MSEC);
    m_ratio = m_capture->get(CV_CAP_PROP_POS_AVI_RATIO);
    
	//make this a manual setting
	m_fps = (int)m_capture->get(CV_CAP_PROP_FPS);
    
	return true;
}

bool VideoProducer::start()
{
	qDebug() <<"start";
	m_continueProduce = true;
	return true;
}

bool VideoProducer::deinit() throw()
{
	QMutexLocker lock(&m_videoMutex);
    if (m_capture->isOpened())
	{
		m_capture->release();
	}
	//m_skippedFrames = false;
	return true;
}

bool VideoProducer::stop()
{
	//if (m_continueProduce)
	//{
		QMutexLocker lock(&m_videoMutex);
		if (m_capture->isOpened())
		{
			m_capture->release();
		}
	//}
		//m_skippedFrames = false;
	return true;
}

bool VideoProducer::readyToProduce() const
{
	//qDebug()<< "goingtoproduce" <<"with grab" ;
	//m_capture->grab();
	//qDebug()<< "grabbed" ;
	//m_capture->grab();

	//following the cameraproducer setup:
	//QMutexLocker lock(&m_videoMutex);
	// return( !m_frames.isEmpty() );
	//TODO check if false here is allowed as it seems to crash in process

	if (!m_capture->isOpened())
		return false;

	return true;
}

bool VideoProducer::produce()
{
	
	QMutexLocker lock(&m_videoMutex);
	//try to combine grab and retrieve using query maybe that works
	//m_capture->query(m_frame); 
	//not opencv 2.1 : http://opencv.willowgarage.com/documentation/cpp/highgui_reading_and_writing_images_and_video.html?highlight=videocapture#VideoCapture::VideoCapture

	if (!m_skippedFrames)
	{  
		for( int i=0; i < getSkipFirstXFrames(); ++i )
		{
			//only grab don't process this is way quicker:
     		if( !m_capture->grab() )
			{
				//it is an expected error
		
				qDebug() <<"Failed to grab frame";
				//setState(PLE_ERROR);
				//added to stop after last frame?
				//this->__stop();
				//stop();
				m_continueProduce = false;
				//this->__stop();
				setError(PlvPipelineRuntimeError, tr("Failed to grab frame"));
			
				return false;
				//TO check
				//return false;
				
			}
		}
		//when we don't skip also grab the first frame like we normally do:
		if (m_continueProduce && getSkipFirstXFrames()==0)
		{
			if( !m_capture->grab() )
			{
				//it is an expected error
				m_continueProduce = false;
				setError(PlvPipelineRuntimeError, tr("Failed to grab frame"));
				qDebug() <<"Failed to grab frame";
				//setState(PLE_ERROR);
				//added to stop after last frame?
				//this->__stop();
				return false;
			}
		}

		m_skippedFrames = true;
	}
	else
	{
		if(m_continueProduce)
		{
			if(!m_capture->grab() )
			{
				m_continueProduce = false;
				//it is an expected error at the end actually
				setError(PlvPipelineRuntimeError, tr("Failed to grab end frame"));
				qDebug() <<"Failed to grab frame";
				setState(PLE_ERROR);
				//added to stop after last frame?
				//this->__stop();
				return false;
			}
		}
		else
		{
			//will show this but has no effects
			//qDebug() <<"Failed to grab frame without continueproduce";
			//this->__stop();
			setError(PlvPipelineRuntimeError, tr("Failed to grab end frame"));
			setState(PLE_ERROR);
			return false;
		}

	}

	//if we allready now it cant and we couldn;t break it 
	if(m_continueProduce)
	{
		if(!m_capture->retrieve(m_frame) ) //, 0
		{
			setError(PlvPipelineRuntimeError, tr("Failed to retrieve frame"));
			qDebug() << "Failed to retrieve frame";
			//stop();
			setState(PLE_ERROR);
			this->__stop();
			return false;
		}
	}

	//Why does it need to be copied? ahh it may not be altered!
    CvMatData d = CvMatData::create(m_frame.properties());
    //m_frame.copyTo(d);
	//TODO when we have this view open it will crash, probably 
	d = m_frame;
    m_outputPin->put(d);

    //this is the total of frames not the current frame: 
	m_totalFrames = (int)m_capture->get(CV_CAP_PROP_FRAME_COUNT);
	//now the next frame to be grabbed
	m_frameCount = (int)m_capture->get(CV_CAP_PROP_POS_FRAMES);
    m_posMillis = (long)m_capture->get(CV_CAP_PROP_POS_MSEC);
    m_ratio = m_capture->get(CV_CAP_PROP_POS_AVI_RATIO);
    m_fps = (int)m_capture->get(CV_CAP_PROP_FPS);

	//int serial = getProcessingSerial();
	
    m_outFrameCount->put(m_frameCount);
    m_outPositionMillis->put(m_posMillis);
    m_outRatio->put(m_ratio);
    m_outFps->put(m_fps);
		
	//if(boolean limit fps)
	if(getFpsValue() >0 && fpsLimit)
	{
		//empiracly about 2-4 milliseconds needed for remaining calculations, might depend on system?
		while ((m_fpstimer.elapsed()-m_prevtime) < 1000/getFpsValue() )
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


//GUI

void VideoProducer::setFilename(const QString& filename)
{
    //Validate the filename
    if( filename.length() < 4 || !VideoProducer::validateExtension(filename) )
    {
        qDebug() << tr("Invalid filename %1.").arg(filename);
        return;
    }
	
	//checcking before saving would mean one has first to set directory and only then the filename so not done that way
    QMutexLocker lock( m_propertyMutex );
	QString dir = m_directory;
    QFile checkfile = dir.append(filename);
	if (checkfile.exists() )
	{
		m_filename = filename;
		qDebug() << "New filename selected: " << m_filename;
		lock.unlock();
		emit filenameChanged(m_filename);
	}
	else
	{
		m_filename = filename;
		qDebug() << "File does not exist: " << dir;
		//emit filenameChanged(m_filename);
		lock.unlock();
	}
	
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
    //QDir dir(directory);
	QDir dir(directoryCopy);
    if( dir.exists() )
    {
		//does it actually recognize without the appended / ?
        if(!directoryCopy.endsWith('/')) directoryCopy.append('/');
		QMutexLocker lock( m_propertyMutex );
		m_directory = directoryCopy;
        qDebug() << "New directory selected:" << m_directory;
		emit directoryChanged(directoryCopy);
    }
}

QString VideoProducer::getDirectory()
{
    QMutexLocker lock( m_propertyMutex );
    return m_directory;
}

void VideoProducer::setFpsValue(int f)
{
    QMutexLocker lock( m_propertyMutex );
    if( f > 0 )
    {
        m_fpsValue = f;
    }
    emit fpsValueChanged(m_fpsValue);
}

int VideoProducer::getFpsValue() const
{
    QMutexLocker lock( m_propertyMutex );
    return m_fpsValue;
}

//skip frames
void VideoProducer::setSkipFirstXFrames(int f)
{
    QMutexLocker lock( m_propertyMutex );
    if( f > -1 )
    {
        m_skipFirstXFrames = f;
    }
    emit skipFirstXFramesChanged(m_skipFirstXFrames);
}

int VideoProducer::getSkipFirstXFrames() const
{
    QMutexLocker lock( m_propertyMutex );
    return m_skipFirstXFrames;
}