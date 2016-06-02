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
#include "StringConcatenate.h"

#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>
#include <set>
#include <limits>
#include <QDate>

#include <QDir>

using namespace plv;
using namespace plvblobtracker;

StringConcatenate::StringConcatenate() :
	m_delimeter("\t")
{
    m_inputPinString1 =  plv::createInputPin<QString>("input text 1", this);
	m_inputPinString2 =  plv::createInputPin<QString>("input text 2", this);
	m_outputPin = createOutputPin<QString>( "output", this );
	
	//TODO add a single number to string
	//double, long, int, uint to string in one

}

StringConcatenate::~StringConcatenate()
{
}

bool StringConcatenate::init()
{
	//QString filenameConcat;
	//filenameConcat.append(getFilenamePreFix()).append("_").append(dateTodayString).append("_").append(timeString);
		
	return true;
}

bool StringConcatenate::start()
{
	
	return true;
	//perhaps add the possibility to have description of file in the name as well.
}

bool StringConcatenate::process()
{
	//expect input as string
	//TODO add check?
	QString srcString1 = m_inputPinString1->get();
	QString srcString2 = m_inputPinString2->get();
	
	//concatenate the two with a delimiter by choice, could be empty.
	srcString1.append(getDelimeter()).append(srcString2);

	m_outputPin->put(srcString1);
    return true;
}

void StringConcatenate::setDelimeter(const QString& delimeterText)
{	
	//perhaps check whether the file exists
	QMutexLocker lock( m_propertyMutex );
	m_delimeter = delimeterText;
}
