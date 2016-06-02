/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * and for this module by Robby van Delden
  *
  * All rights reserved.
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

#ifndef PLVBLOBTRACK_STRINGCAST_H
#define PLVBLOBTRACK_STRINGCAST_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/Enum.h>
#include <plvcore/Pin.h>

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

namespace plvblobtracker
{
    class StringCast : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( StringCast  )
        Q_CLASSINFO("author", "Robby")
        Q_CLASSINFO("name", "StringCast")
        Q_CLASSINFO("description", "Using cast of the QVariant class, this returns the variant as a QString if the variant has type() String, Bool, ByteArray, Char, Date, DateTime, Double, Int, LongLong, StringList, Time, UInt, or ULongLong; otherwise returns an empty string." )

		
        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR

    public:
        StringCast();
        virtual ~StringCast ();
		virtual bool init();
		virtual bool start();

		
    public slots:
        
	signals:
		
    private:
        //plv::InputPin<QVariant>* m_inputPinNumber;
		//plv::InputPin<Double>* m_inputPinDouble;
		plv::OutputPin<QString>* m_outputString;
		
		//realtime gui variables:	
		QString m_delimeter;
		
		int m_cvMatDataTypeId;
	
    };
}
#endif // StringCast_H
