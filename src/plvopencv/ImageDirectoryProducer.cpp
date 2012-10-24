
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

#include <QMutexLocker>
#include <QDebug>

#include "ImageDirectoryProducer.h"

#include <opencv/highgui.h>
#include <plvcore/CvMatDataPin.h>
#include <QDir>

using namespace plv;
using namespace plvopencv;

ImageDirectoryProducer::ImageDirectoryProducer() : 
	m_idx(0),
	m_start(1),
	m_fps(30),
	m_loop(true),
	m_nr(1),
	m_end(65000),
	ByName(0),
	ByTime(1),
	BySize(2),
	ByType(3),
	ByUnsorted(4),
	ByNoSort(5),
	ByDirsFirst(6),
	ByDirsLast(7),
	ByReversed(8),
	ByIgnoreCase(9),
	ByLocaleAware(10),
	WithNumbers(11)
{
	 //first one added is default
	PLV_ENUM_ADD( m_sort, WithNumbers);
    PLV_ENUM_ADD( m_sort, ByName );
    PLV_ENUM_ADD( m_sort, ByTime );
    PLV_ENUM_ADD( m_sort, BySize );
	PLV_ENUM_ADD( m_sort, ByType);
	PLV_ENUM_ADD( m_sort, ByUnsorted);
	PLV_ENUM_ADD( m_sort, ByNoSort);
	PLV_ENUM_ADD( m_sort, ByDirsFirst);
	PLV_ENUM_ADD( m_sort, ByDirsLast);
	PLV_ENUM_ADD( m_sort, ByReversed);
	PLV_ENUM_ADD( m_sort, ByIgnoreCase);
	PLV_ENUM_ADD( m_sort, ByLocaleAware);
	
	//PLV_ENUM_ADD( m_sort, QDir::Name );
 //   PLV_ENUM_ADD( m_sort, QDir::Time );
 //   PLV_ENUM_ADD( m_sort, QDir::Size );
	//PLV_ENUM_ADD( m_sort, QDir::Type);
	//PLV_ENUM_ADD( m_sort, QDir::Unsorted);
	//PLV_ENUM_ADD( m_sort, QDir::NoSort);
	//PLV_ENUM_ADD( m_sort, QDir::DirsFirst);
	//PLV_ENUM_ADD( m_sort, QDir::DirsLast);
	//PLV_ENUM_ADD( m_sort, QDir::Reversed);
	//PLV_ENUM_ADD( m_sort, QDir::IgnoreCase);
	//PLV_ENUM_ADD( m_sort, QDir::LocaleAware);

    setSortType( m_sort );

    m_imgOutputPin = createCvMatDataOutputPin("image_output", this );
    m_fileNameOutputPin  = createOutputPin<QString>("file name", this );
    m_filePathOutputPin  = createOutputPin<QString>("file path", this );

    m_imgOutputPin->addAllChannels();
    m_imgOutputPin->addAllDepths();
}

ImageDirectoryProducer::~ImageDirectoryProducer()
{
}

//QString ImageDirectoryProducer::getDirectory()
//{
//    QMutexLocker lock( m_propertyMutex );
//    return m_directory;
//}

void ImageDirectoryProducer::setDirectory(const QString& directory)
{
	
    //replace all '\' characters with '/' characters
    QString path = directory;
    path.replace('\\','/');

    //validate the directory
	//QDir dir(directory);
    QDir dir(path);
    if( dir.exists() )
    {
        QMutexLocker lock( m_propertyMutex );
        m_directory = path;
        qDebug() << "New directory selected:" << m_directory;
    }
	else
	{
		qDebug() << "This directory does not exist:" << path;
	}

	//init(); //guess we will need that
}

bool ImageDirectoryProducer::init()
{
	m_timeSinceLastFPSCalculation.start();
	qDebug() << "init with case is:" << getSortType().getSelectedValue();
	//wasn't able to put the qdir::sortflags directly in the plv enum so used a ugly workaround
	
	if (getSortType().getSelectedValue() != 11)
    {
		QDir dir(m_directory);
		if( !dir.exists() )
		{
			qDebug() << "Directory is invalid";
		    setError( PlvPipelineInitError, "Directory is invalid");
	        return false;
	    }
	    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable);
	
		//just in case not set properly later on
		dir.setSorting(QDir::Name );
		
		//set the sorttype based on the fitting int number, could net get the QDir sortflag as an enum.
		switch (getSortType().getSelectedValue())
		{
			case 0:
			dir.setSorting(QDir::Name );
			break;

			case 1:
			dir.setSorting(QDir::Time );
			break;

			case 2:
			dir.setSorting( QDir::Size );
			qDebug() << "size";
			break;

			case 3:
			dir.setSorting( QDir::Type);
			break;

			case 4:
			dir.setSorting( QDir::Unsorted);
			break;

			case 5:
			dir.setSorting( QDir::NoSort);
			break;

			case 6:
			dir.setSorting( QDir::DirsFirst);
			break;

			case 7:
			dir.setSorting( QDir::DirsLast);
			break;

			case 8:
			dir.setSorting( QDir::Reversed);
			break;

			case 9:
			dir.setSorting( QDir::IgnoreCase);
			break;

			case 10:
			dir.setSorting( QDir::LocaleAware);
			break;

			case 11:
			qDebug()<< "this state is supposed to beunreachable";
			break;
		}

		QStringList filters;
		filters << "*.jpg" << "*.jpeg" << "*.bmp" << "*.sr" << "*.png" << "*.pdm";
		dir.setNameFilters(filters);

		m_entryInfoList = dir.entryInfoList();
	
	} 
	else 
	{
		qDebug()<< "no Qdir set to fasten up process using string instead";
	}

	m_idx = getStartNumber();
	m_nr = getStartNumber();
    return true;
}

bool ImageDirectoryProducer::deinit() throw ()
{
    m_entryInfoList.clear();
	//does that really need to be here i don't think so!
	//m_idx = getStartNumber();
	//m_nr = getStartNumber();
    return true;
}

bool ImageDirectoryProducer::readyToProduce() const
{
    //return m_idx < m_entryInfoList.size(); will not work for 
	return true;
}

bool ImageDirectoryProducer::produce()
{
//	m_timeSinceLastFPSCalculation.restart();
//	m_timeSinceLastFPSCalculation.elapsed() > track.getLastUpdate()) 
	    
	//QFileInfo fileInfo;
	bool outofscope =false;
	m_timeSinceLastFPSCalculation.restart();
	
	//DEBUGGING
	//QString filenames;
	//filenames = QString("%1.jpg").arg(m_nr);
	//QFileInfo fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
	

    QFileInfo fileInfo ;
	cv::Mat image;

	if (getSortType().getSelectedValue()!= 11)
	{
		fileInfo = m_entryInfoList.at(m_idx);
		//load the image
		const std::string& path = fileInfo.absoluteFilePath().toStdString();
		//cv::Mat image = cv::imread(path, CV_LOAD_IMAGE_UNCHANGED);
		image = cv::imread(path, CV_LOAD_IMAGE_UNCHANGED);
		if(image.data == 0)
		{
			setError( PlvPipelineRuntimeError, tr("Failed to load image %1.").arg(fileInfo.absolutePath()));
			return false;
		}
		m_idx++ ;
		//assume one will not use the number method for loop intensive things
		if (m_idx>(getEndNumber()-1))
		{
			//qDebug() << "runoutofscope is" ;
			if (getLoopIt()) 
			{
				//qDebug() << "reset" ;
				m_idx=getStartNumber();
			}
		
		}
	}
	else //if (getSortType().getSelectedValue()== 11)
	{
		//qDebug() << "selectedvalue is eleven";
		//QString filenames;
		if (m_nr>(getEndNumber()-1))
		{
			outofscope=true;
		}

		if (outofscope)
		{
			qDebug() << "I have run out of scope" ;
			if (getLoopIt())
			{
				qDebug() << "You selected loop so i will start over" ;
				m_nr = getStartNumber();
			}
			else
			{
				qDebug() << "You did not select loop so i will stop now" ;
				//deinit();
				//stop();
				setError( PlvPipelineRuntimeError, tr("Failed to stop properly but run out of scope %1 is bigger than %2.").arg(m_nr).arg(getEndNumber()-1));
				//return false;
				while (!getLoopIt());
			}
		}

		QString filenames = QString("%1.jpg").arg(m_nr);
		fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
	
		const std::string& pathalt = fileInfo.absoluteFilePath().toStdString();
		
		/*if(!outofscope)
		{*/			
		image = cv::imread(pathalt, CV_LOAD_IMAGE_UNCHANGED);
		if(image.data == 0)
		{
			setError( PlvPipelineRuntimeError, tr("Failed to load image with own numbermethod %1.").arg(fileInfo.absolutePath()));
			return false;
		}
		//}
		
		m_nr++;
		//m_idx-- ;
	} //if not with own number case

	

	//if(image.data == 0)
	//{
	//	qDebug() << "not a proper file:" << "dir" << m_directory << "absolute path" << fileInfo.absoluteFilePath();
	//	setError( PlvPipelineRuntimeError, tr("Failed to load image %1.").arg(fileInfo.absolutePath()));
	//	return false;
	//}

	m_imgOutputPin->put( CvMatData(image));

	m_fileNameOutputPin->put( fileInfo.fileName() );
    m_filePathOutputPin->put( fileInfo.absolutePath() );

    //m_imgOutputPin->put( CvMatData(image) );
    //m_fileNameOutputPin->put( fileInfo.fileName() );
    //m_filePathOutputPin->put( fileInfo.absolutePath() );

	int elapsed = m_timeSinceLastFPSCalculation.elapsed();
	bool setwaiting =false;
	//TODO insert a more reliable maxFPS setter that does not rely solely on the producer but also on the tracker
	while (elapsed < (1000/getWantedFPS()))
	{
		elapsed=m_timeSinceLastFPSCalculation.elapsed();
		setwaiting = true;
	}

	//if (setwaiting)
	//{
	//	//qDebug()<< "I waited as the frame rate was lower than I processed";
	//}

	return true;
}

void ImageDirectoryProducer::setSortType(plv::Enum e)
{

    // update selection
    m_sort = e;
	//??
	init();
    // update GUI
    emit( sortTypeChanged(e) );
}