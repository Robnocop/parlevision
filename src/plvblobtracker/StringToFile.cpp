/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos,
  * and for this module Robby van Delden
  * All rights reserved.
  *
  * This file is part of the blobtracker module of ParleVision.
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
#include "StringToFile.h"
#include "Blob.h"

#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>
#include <set>
#include <limits>
#include <QDate>

#include <QDir>

using namespace plv;
using namespace plvblobtracker;

StringToFile::StringToFile() :
	m_directory("C:/temp/"),
	m_filename("parlevision.txt"),
	m_includeTime(true),
	m_logNumber(1),
	m_autoFileName(true),
	m_filenamePreFix("")
{
    m_inputPinString =  plv::createInputPin<QString>("input text", this);
}

StringToFile::~StringToFile()
{
}

bool StringToFile::init()
{
	if (getAutoFileName())
	{
		QDate dateToday;
		QString dateTodayString = dateToday.currentDate().toString();
		QString timeString = time.toString("hh:mm:ss");
		QString filenameConcat;
		filenameConcat.append(getFilenamePreFix()).append("_").append(dateTodayString).append("_").append(timeString);
		setFilename(filenameConcat);
	}
	time.start();
	return true;
}

bool StringToFile::start()
{
	if (getAutoFileName())
	{
		QDate dateToday;
		QString dayTodayString = QString("%1").arg(dateToday.currentDate().day());
		QString monthTodayString = QString("%1").arg(dateToday.currentDate().month());
		QString yearTodayString = QString("%1").arg(dateToday.currentDate().year());
		QString timeString = time.toString("hh:mm:ss");
		QString hourString = QString("%1").arg(time.hour());
		QString minuteString = QString("%1").arg(time.minute());
		QString msecString = QString("%1").arg(time.msec());
		QString filenameConcat;
		filenameConcat.append(getFilenamePreFix()).append("_").append(dayTodayString).append(dayTodayString).append("_").append(monthTodayString).append("_").append(yearTodayString).append("_").append(hourString).append("_").append(minuteString).append("_").append(msecString).append(".txt");
		setFilename(filenameConcat);
	}
	return true;
	//perhaps add the possibility to have description of file in the name as well.
}

bool StringToFile::process()
{
	//expect input as string
	QString srcString = m_inputPinString->get();
	
	
		
	//based on annotation tool:
	//QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6").arg(m_framenr).arg(bcd.oldid).arg(bcd.newid).arg(bcd.cogs.x).arg(bcd.cogs.y).arg(bcd.changetype);
	QString longfilename = getDirectory().append(getFilename());
	QFile file(longfilename); //getFilename());

	bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
	Q_ASSERT(ret);
	QTextStream s(&file);
	
	//include participant number followed by a tab
	if (getIncludeInt())
	{
		QString intNumber = QString("%1").arg(getLogNumber());
		for (int i = 0; i < intNumber.size(); ++i)
			s << intNumber.at(i);
		s << "\t";
	}
	
	//inlcude time in the log followed by a tab	
	//no this creates a second file
	if (getIncludeTime())
	{
		
		//add time if required for output
		//set time to current time
		time = QTime::currentTime();
		//save time in format 14:13:09
		//default for time to string already is hh:mm:ss
		QString timeString = time.toString("hh:mm:ss:zzz");

		QFile file2(getDirectory().append("time_").append(getFilename())); //getFilename());
		bool ret2 = file2.open(QIODevice::WriteOnly | QIODevice::Append);
		Q_ASSERT(ret2);
		QTextStream t(&file2);
		for (int i = 0; i < timeString.size(); ++i)
			t << timeString.at(i);
		
		//TO CHECK should we close it?
		file2.write("\n");
		ret2 = file2.flush();
		Q_ASSERT(ret2);
		file2.close();
	}

	//write the string to the stream
	for (int i = 0; i < srcString.size(); ++i)
		s << srcString.at(i);
	//end the stream with a line ending
	//TODO check if the string allready includes a line ending
	//TO CHECK should we close it?
	//TOD we should only close it on stop/deinit actually, although in the current way we are more sure that we can use the logfiles if a crash happends.
	file.write("\n");	
	ret = file.flush();
	Q_ASSERT(ret);
	file.close();
    return true;
}

//create a variable with name and seperate a directory to put it in
void StringToFile::setDirectory(const QString& directory)
{	
    QMutexLocker lock( m_propertyMutex );
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
        m_directory = path;
        qDebug() << "New directory selected:" << m_directory;
    }
	else
	{
		qDebug() << "This directory does not exist:" << path;
	}
}


void StringToFile::setFilename(const QString& filename)
{	
	//perhaps check whether the file exists
	QMutexLocker lock( m_propertyMutex );
	m_filename = filename;
}

void StringToFile::setFilenamePreFix(const QString& filename)
{	
	//perhaps check whether the file exists
	QMutexLocker lock( m_propertyMutex );
	m_filenamePreFix = filename;
}
//TODO
//bonus link to blobtrack info/rectangle info/ based on the type of input as was done in the server
