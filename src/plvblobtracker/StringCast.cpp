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
#include "StringCast.h"

#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>
#include <set>
#include <limits>
#include <QDate>

#include <QDir>

using namespace plv;
using namespace plvblobtracker;

StringCast::StringCast()
{
    //m_inputPinNumber =  plv::createInputPin<QVariant>("input number", this);
	//m_inputPinString2 =  plv::createInputPin<QString>("input text 2", this);
	m_outputString = createOutputPin<QString>( "output string", this );
	
	//TODO add a single number to string
	//double, long, int, uint to string in one

	//plv::createDynamicInputPin( "generic pin", this, plv::IInputPin::CONNECTION_OPTIONAL );
	plv::createDynamicInputPin( "generic pin", this, plv::IInputPin::CONNECTION_REQUIRED );
	m_cvMatDataTypeId = QMetaType::type("plv::CvMatData");
}

StringCast::~StringCast()
{
}

bool StringCast::init()
{
	//QString filenameConcat;
	//filenameConcat.append(getFilenamePreFix()).append("_").append(dateTodayString).append("_").append(timeString);
		
	return true;
}

bool StringCast::start()
{
	
	return true;
	//perhaps add the possibility to have description of file in the name as well.
}

bool StringCast::process()
{
	//return fals eno recognised as alternative to not creating the variable so we allocate an empty one
	QString srcString1 = "";

	PipelineElement::InputPinMap::iterator itr = m_inputPins.begin();
    for( ; itr != m_inputPins.end(); ++itr )
    {
        plv::IInputPin* pin = itr->second.getPtr();
        if( pin->isConnected() )
        {
			if( pin->getTypeId() == m_cvMatDataTypeId)
			{
				setError(PlvPipelineRuntimeError, tr("Can't, well just don't, cast image to string"));
				qDebug() <<"Failed to cast image";
				setState(PLE_ERROR);
				return false;
			}
			else
			{
              QVariant v;
              pin->getVariant(v);
			  srcString1 = v.toString();
			}
        }
    }

	//expect input as a castable number
	//QVariant srcNumber = m_inputPinNumber->get();

	//TODO remove and let pipeline stop if it can't be cast
	//not even needed 
	//QString srcString1 = "";
	//if (srcNumber.canConvert(QString))
	//{
		
	//}
	//QString srcString2 = m_inputPinString2->get();
	
	m_outputString->put(srcString1);
    return true;
}

