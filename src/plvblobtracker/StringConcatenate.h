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

#ifndef PLVBLOBTRACK_STRINGCONCATENATE_H
#define PLVBLOBTRACK_STRINGCONCATENATE_H

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
    class StringConcatenate : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( StringConcatenate  )
        Q_CLASSINFO("author", "Robby")
        Q_CLASSINFO("name", "StringConcatenate")
        Q_CLASSINFO("description", "Concatenates two Strings to tab,space or comma delimited type text file" )

		Q_PROPERTY( QString delimeter READ getDelimeter WRITE setDelimeter NOTIFY delimeterChanged )

        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR

    public:
        StringConcatenate();
        virtual ~StringConcatenate ();
		virtual bool init();
		virtual bool start();

		QString getDelimeter() {QMutexLocker lock( m_propertyMutex ); return m_delimeter; };
	
    public slots:
        void setDelimeter(const QString& newValue);

	signals:
		void delimeterChanged(const QString& newValue);

    private:
        plv::InputPin<QString>* m_inputPinString1;
		plv::InputPin<QString>* m_inputPinString2;
		plv::OutputPin<QString>* m_outputPin;
		
		//realtime gui variables:	
		QString m_delimeter;
	
    };
}
#endif // StringConcatenate_H
