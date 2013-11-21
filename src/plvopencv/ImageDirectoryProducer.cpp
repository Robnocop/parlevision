
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
	m_start(1),
	m_fps(30),
	m_loop(true),
	m_annotation(false),
	m_nr(1),
	m_end(65000),
	m_imgtype(".png"), //png makes more sense for annotation
	m_filenameFrameNr("framenr.txt"),
	m_trailingZeros(0),
	m_flagTimer(true),
	m_flagpaused(false),
	alejandroprefab("syncedVF")
{
	 //first one added is default
	PLV_ENUM_ADD( m_sort, WITHNUMBERS);
    PLV_ENUM_ADD( m_sort, BYNAME );
    PLV_ENUM_ADD( m_sort, BYTIME );
    PLV_ENUM_ADD( m_sort, BYSIZE );
	PLV_ENUM_ADD( m_sort, BYTYPE);
	PLV_ENUM_ADD( m_sort, BYUNSORTED);
	PLV_ENUM_ADD( m_sort, BYNOSORT);
	PLV_ENUM_ADD( m_sort, BYDIRSFIRST);
	PLV_ENUM_ADD( m_sort, BYDIRSLAST);
	PLV_ENUM_ADD( m_sort, BYREVERSED);
	PLV_ENUM_ADD( m_sort, BYIGNORECASE);
	PLV_ENUM_ADD( m_sort, BYLOCALEAWARE);
	
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

    //setSortType( m_sort );

    m_filePathOutputPin  = createOutputPin<QString>("file path", this );

	m_fileNameOutputPin  = createOutputPin<QString>("file name", this );
    	
	//ANNOTATIOM
	//m_serialOutputPin = createOutputPin<unsigned int>("read in serial", this );
	m_fileNameNrOutputPin = createOutputPin<int> ("the nr for annotation", this );
	m_correctimagedirectoryboolOutputPin = createOutputPin<bool> ("a signal of textfile anno", this );

	 m_imgOutputPin = createCvMatDataOutputPin("depth image output", this );
	//RGB value for annotation temp hack!
	m_imgOutputPinRGB  = createCvMatDataOutputPin("video output", this );

	//to allow a cyclic influence, asking for problems should be used carefully! Needed for trackannotation.
	//m_changeFrame = createInputPin<int>( "num frames", this, IInputPin::CONNECTION_OPTIONAL, IInputPin::CONNECTION_ASYNCHRONOUS );
	//cyclce detection is implemented :(, instead use an ugly solution, external text file with serial and code that is read every frame.

    m_imgOutputPin->addAllChannels();
    m_imgOutputPin->addAllDepths();

	m_imgOutputPinRGB->addAllChannels();
    m_imgOutputPinRGB->addAllDepths();
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
	
	if(!path.endsWith('/'))
    {
        path.append('/');
    }
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

void ImageDirectoryProducer::setDirectoryVideos(const QString& directory)
{	
    //replace all '\' characters with '/' characters
    QString path = directory;
    path.replace('\\','/');
	
	if(!path.endsWith('/'))
    {
        path.append('/');
    }
    //validate the directory
	//QDir dir(directory);
    QDir dir(path);
    if( dir.exists() )
    {
        QMutexLocker lock( m_propertyMutex );
        m_directoryVideos = path;
        qDebug() << "New RGB directory selected:" << m_directoryVideos;
    }
	else
	{
		qDebug() << "This directory does not exist:" << path;
	}

	//init(); //guess we will need that
}

//not needed for annotation
QFileInfoList ImageDirectoryProducer::loadImageDir(QDir dir)
{
	QFileInfoList entryInfoListOut;

	//if (getSortType().getSelectedValue() != 11) //taken out always use nr method
   
	QFileInfo fileInfo ;
	//added ability to  set to preceeding zeros, based on a gui so 0 will remain the same, more will add preceeding zeros
	QStringList filters;
	filters << ".jpg" << ".jpeg" << ".bmp" << ".sr" << ".png" << ".pdm";
	QString filenames = QString("%1%2").arg(getStartNumber(), getTrailingZeros(), 10, QChar('0')).arg(filters.at(0)).toUpper();
	fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
	if (!fileInfo.exists()) 
	{
		filenames = QString("%1%2").arg(getStartNumber(), getTrailingZeros(), 10, QChar('0')).arg(filters.at(1)).toUpper();
		fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
		if (!fileInfo.exists()) 
		{
			filenames = QString("%1%2").arg(getStartNumber(), getTrailingZeros(), 10, QChar('0')).arg(filters.at(2)).toUpper();
			fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
			if (!fileInfo.exists()) 
			{
				filenames = QString("%1%2").arg(getStartNumber(), getTrailingZeros(), 10, QChar('0')).arg(filters.at(3)).toUpper();
				fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
				if (!fileInfo.exists()) 
				{
					filenames = QString("%1%2").arg(getStartNumber(), getTrailingZeros(), 10, QChar('0')).arg(filters.at(4)).toUpper();
					fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
					if (!fileInfo.exists()) 
					{
						filenames = QString("%1%2").arg(getStartNumber(), getTrailingZeros(), 10, QChar('0')).arg(filters.at(5)).toUpper();
						fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
						if (!fileInfo.exists()) 
						{
							qDebug() << "no files img file (.jpg .jpeg .bmp .png .sr .pdm) found with startnumber in this directory";
						}
						else
						{
							m_imgtype = ".pdm";
						}
					}
					else
					{
						m_imgtype = ".png";
					}
				}
				else
				{
					m_imgtype = ".sr";
				}
			}
			else
			{
				m_imgtype = ".bmp";
			}
		}
		else
		{
			m_imgtype = ".jpeg";
		}
			
	}

	return entryInfoListOut = dir.entryInfoList();
}


bool ImageDirectoryProducer::init()
{
	m_previousFrameNr = 0;
	m_signal = true;
	m_wantedFrameNr = -1;
	m_lastDirection = 1; //check was 1

	//reset file for annotation tool, instead zerofile on init.
	//zeroFile();
	resetFile();

	if (m_flagTimer)
		m_timeSinceLastFPSCalculation.start();
	else
		m_timeSinceLastFPSCalculation.restart();
	m_flagTimer = false;
	
	//wasn't able to put the qdir::sortflags directly in the plv enum so used a ugly workaround
	QDir dir(m_directory);
	//m_entryInfoList = loadImageDir(dir);

	QDir dir2(m_directoryVideos);
	//m_entryInfoListRGB = loadImageDir(dir2);
	m_imgtype = ".png";
	
	if (!m_flagpaused)
	{
		qDebug() << "flagpaused in init";
		m_nr = getStartNumber();
		m_previousFrameNr = m_nr;
	}
	return true;
}

bool ImageDirectoryProducer::deinit() throw ()
{
	m_flagpaused = true;
    //not needed for annotation
	//m_entryInfoList.clear();
	//m_entryInfoListRGB.clear();
	
	//does that really need to be here i don't think so!
	//m_nr = getStartNumber();
    return true;
}

bool ImageDirectoryProducer::readyToProduce() const
{
    //return m_idx < m_entryInfoList.size(); will not work for 
	return true;
}

//reset the framenr change file to read 0 change of framnr, as long no interference was needed by a block later on in the pipeline
bool ImageDirectoryProducer::zeroFile()
{
	//UGLY SOLUTION!!!!
	if (getAnnotation())
	{
		QString changeframe = QString("%1 \t %2").arg("0").arg("0");
		QFile file2(m_filenameFrameNr);
		//overwrites as it is QIODevice with truncate
		bool ret2 = file2.open(QIODevice::WriteOnly | QIODevice::Truncate);
		Q_ASSERT(ret2);
		QTextStream s(&file2);
		s << changeframe;
		ret2 = file2.flush();
		Q_ASSERT(ret2);
		file2.close();
	}
	return true;
}


//reset the framenr change file to read 0 change of framnr, as long no interference was needed by a block later on in the pipeline
bool ImageDirectoryProducer::resetFile()
{
	//QString toimageproducer = QString("%1 \t %2 \t %3").arg(back).arg(back+filenamenr).arg(true);
	//jibberish values that can be recognised, only the back value needs to be maintained
	
	//arg 1 used to be m_lastdirection
	//TODO what happens if we don't reset m_lastdirection
	//QString toimagproducerorannotation = QString("%1 \t %2 \t %3").arg(-2).arg(-1).arg(false);
	QString toimagproducerorannotation = QString("%1 \t %2 \t %3").arg(m_lastDirection).arg(-2).arg(false);
	
	//qDebug() << "resetfileline in imagedir " << toimagproducerorannotation;
	QFile file2(m_filenameFrameNr);
	bool ret2 = file2.open(QIODevice::WriteOnly | QIODevice::Truncate);
	Q_ASSERT(ret2);
	////Q_ASSERT(blobString.size()>0);
	QTextStream s(&file2);
	s << toimagproducerorannotation;
	//for (int i = 0; i < blobTabString.size(); ++i)
	//	s << blobTabString.at(i);// << '\n';
	////file.write(blobString);
	////file.write("APPEND new Line");
	ret2 = file2.flush();
	Q_ASSERT(ret2);
	file2.close();

	///////////////////////////////////////////
	return true;
}

//QString toimageproducer = QString("%1 \t %2 \t %3").arg(back).arg(back+filenamenr).arg(true);
int ImageDirectoryProducer::readFile(QString filename, int backvalue) 
{
	int back = -2;
	int newframe = -1;
	QFile inFile(filename);
	if(inFile.exists())
	{
		if ( inFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) 
		{
			//QString processingserial,framechange;
			
			QTextStream stream( &inFile );
			QString line;

			for (int counter = 1; counter < 2; counter++) {
				line = stream.readLine(); 
			}

			//we can read that value to know whether to go back.
			//at init it should be set to 0,0 ??
			back = line.section('\t', 0,0).toInt();
			newframe = line.section('\t', 1,1).toInt();
			//actually only bool allowed
			//this bool is only true if it is read from a trackannotation save to the readfile
			//as long as the direction is remained a signal is not needed, if it changes images will be purged until the signal frame is found
			m_signal = line.section('\t', 2,2).toInt();
			//qDebug() << "debug readfile imagedirectoryproducer" << "direction the framechangeInt" << back << " newframe" << newframe<< ", signal" << m_signal ;
			
			//do stuff with the temp strings
			
			//if (processingserialDouble < (this->getProcessingSerial()) && processingserialDouble > (this->getProcessingSerial()-3) )
		}
	}
	inFile.close();

	//overwrite the file after each read with recognisable jibberish
	if (backvalue == 1)
		return back;
	
	return newframe;
}

bool ImageDirectoryProducer::produce()
{

	//cycles are prevented in parlevision (which makes some sense)
	//instead an ugly solution using a txt file at the end of the pipeline to change the next loop
	if (getAnnotation())
	{
		//TODO WHY if key for currentframe is pressed for long time in annotation will it result in nextframe (on release) every now and then? Is the read and write to slow?
		//m_previousFrameNr = readFile(m_filenameFrameNr);
		//compensate for the +1 later on
		m_wantedFrameNr = readFile(m_filenameFrameNr, 0);
		m_lastDirection = readFile(m_filenameFrameNr, 1);
		//m_signalFile = readFile(m_filenameFrameNr, 2); //is allready set on every readFile.
		
		//TODO did this commenting change anything?
		//int framechangeInt= readFile(m_filenameFrameNr);
		

		//int temp2 = m_nr+m_previousFrameNr;
		int temp2 =  m_previousFrameNr;
		if (m_wantedFrameNr != -2)
		{
			temp2 = m_wantedFrameNr; 
			//qDebug() << "set to wantedframenr"<< m_wantedFrameNr;
		}
		//m_lastdirection should not be reset, so this will no longer be reached until proven otherwise
		else if(m_lastDirection != -3)
		{
			temp2 = m_nr+m_lastDirection;
			//qDebug() << "set to anotherdir"<< m_lastDirection;
		}
		
		qDebug() << "m_nr:" << m_nr << "last direction" << m_lastDirection << "nr temp,will be loaded" << temp2;

	
		if (temp2>=getStartNumber() && temp2<=getEndNumber())
		{
			m_nr = temp2;
			//qDebug() << "m_nr " <<m_nr << ",will be loaded";
		}
		else if (temp2<=getStartNumber())
		{
			m_nr = getStartNumber();
			qDebug() << "m_nr not in range set to startnumber";
		}
		else if (temp2>=getEndNumber())
		{
			m_nr = getEndNumber();
			qDebug() << "m_nr too big, not in range so set to endnumber";
		}
		else
		{
			qDebug() << "strange!!! m_nr not in range but somehow not bigger or smaller than the range!";
		}
		
		resetFile();
	}

	//QFileInfo fileInfo;
	bool outofscope =false;
	m_timeSinceLastFPSCalculation.restart();
	
	//DEBUGGING
	//QString filenames;
	//filenames = QString("%1.jpg").arg(m_nr);
	//QFileInfo fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
	
    QFileInfo fileInfo ;
	QFileInfo fileInfoRGB;
	cv::Mat image;
	
	CvMatData outx = CvMatData::create(720,480,CV_8U,3);
	cv::Mat& rgbimage = outx;
    //rgbimage = cv::Scalar(0,0,0);
	
	//cv::Mat rgbimage;

	if (m_nr>=(unsigned int) getEndNumber())
	{
		if (m_nr!=getEndNumber()+1)
		{
			//qDebug() << "I have run out of scope";
		}
		outofscope=true;
	}

	if (outofscope)
	{
		if (getLoopIt())
		{
			//qDebug() << "You selected loop so i will start over" ;
			m_nr = getStartNumber();
			outofscope=false;
		}
		else
		{
			if (m_nr==getEndNumber())
				qDebug() << "You did not select loop so i will stop loading images now" ;
			
			//WHY?
			m_nr =getEndNumber()+1;
		}
	}
		
	//add dir directly
	QString filenames = QString("%1%2%3").arg(m_directory).arg(m_nr, getTrailingZeros(), 10, QChar('0')).arg(m_imgtype).toUpper();
	
	if (getAnnotation())
	{
		QString filenamesRGB = QString("%1%2%3%4").arg(m_directoryVideos).arg(alejandroprefab).arg(m_nr, getTrailingZeros(), 10, QChar('0')).arg(".jpg").toUpper();
		if (!outofscope)
		{
			QFile filergb(filenamesRGB);
			if( filergb.exists() )
			{
				rgbimage = cv::imread(filenamesRGB.toStdString(), CV_LOAD_IMAGE_UNCHANGED);
				if(rgbimage.data == 0)
				{
					setError( PlvPipelineRuntimeError, tr("Failed to load image with own numbermethod %1.").arg(fileInfo.absolutePath()));
					//qDebug() << tr("image with filename : %1, could not be loaded").arg(filenames);
					qDebug("failed to load RGB image with number method");
					//create an empty file instead
					return false;
				}
			}
			else
			{
				qDebug() << filenamesRGB << "does not exist";
				//create an empty file instead;

			}

		}
	}

	//QString filenames = QString("%1%2%3").arg(m_directory).arg(m_nr).arg(m_imgtype); 
	fileInfo = QFileInfo::QFileInfo(m_directory,filenames );
		
	if (!outofscope)
	{
		QFile file(filenames);
		if( file.exists() )
			image = cv::imread(filenames.toStdString(), CV_LOAD_IMAGE_UNCHANGED);
	}
	
	if(image.data == 0 && !outofscope)
	{
		setError( PlvPipelineRuntimeError, tr("Failed to load image with own numbermethod %1.").arg(fileInfo.absolutePath()));
		//qDebug() << tr("image with filename : %1, could not be loaded").arg(filenames);
		qDebug("failed to load image, either empty or outofscope");
		return false;
	}

	if(image.data == 0 && outofscope)
	{
		setError( PlvPipelineRuntimeError, tr("Failed to load image with own numbermethod %1. This is probabaly because it is out of scope").arg(fileInfo.absolutePath()));
		return false;
	}
		
	//save last framenr
	m_previousFrameNr = m_nr;
					
	m_imgOutputPin->put( CvMatData(image));
	//RGB
	m_imgOutputPinRGB->put( CvMatData(rgbimage));
	//TODO add video feeds as frame based input;
	//should somehow be linked to the actual number of the frame so we can load the correct file directly from the textfile of annotation shit. 
	
	//fileInfo.fileName() or filenames doesn't really matter
	m_fileNameOutputPin->put( fileInfo.fileName() );
    
	m_fileNameNrOutputPin->put(m_nr);
	m_correctimagedirectoryboolOutputPin-> put(m_signal);
	m_filePathOutputPin->put( fileInfo.absolutePath() );

	
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