/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * and for this module Robby van Delden
  * 
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

#ifndef PLVBLOBTRACK_STRINGTOFILE_H
#define PLVBLOBTRACK_STRINGTOFILE_H

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
    class Blob;

    class StringToFile : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( StringToFile  )
        Q_CLASSINFO("author", "Robby")
        Q_CLASSINFO("name", "StringToFile ")
        Q_CLASSINFO("description", "Saves Strings to tab delimited type text file" )

        Q_PROPERTY( QString directory READ getDirectory WRITE setDirectory NOTIFY directoryChanged )
		Q_PROPERTY( QString filename READ getFilename WRITE setFilename NOTIFY filenameChanged )
		Q_PROPERTY( QString filenamePreFix READ getFilenamePreFix WRITE setFilenamePreFix NOTIFY filenameChangedPreFix )
		
		Q_PROPERTY( int logNumber READ getLogNumber WRITE setLogNumber NOTIFY logNumberChanged )
		Q_PROPERTY( bool includeTime READ getIncludeTime WRITE setIncludeTime NOTIFY includeTimeChanged )
		Q_PROPERTY( bool includeInt READ getIncludeInt WRITE setIncludeInt NOTIFY includeIntChanged )
		Q_PROPERTY( bool autoFileName READ getAutoFileName  WRITE setAutoFileName  NOTIFY autoFileNameChanged )


        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR

    public:
        StringToFile();
        virtual ~StringToFile ();
		virtual bool init();
		virtual bool start();

        QString getDirectory() {QMutexLocker lock( m_propertyMutex ); return m_directory; };
		QString getFilename() {QMutexLocker lock( m_propertyMutex ); return m_filename; };
		QString getFilenamePreFix() {QMutexLocker lock( m_propertyMutex ); return m_filenamePreFix; };
		bool getIncludeTime() {QMutexLocker lock( m_propertyMutex ); return m_includeTime; };
		bool getIncludeInt() {QMutexLocker lock( m_propertyMutex ); return m_includeInt; };
		int getLogNumber() {QMutexLocker lock( m_propertyMutex ); return m_logNumber; };
		bool getAutoFileName(){QMutexLocker lock( m_propertyMutex ); return m_autoFileName; };

    public slots:
        void setDirectory(const QString& newValue);
		void setFilename(const QString& newValue);
		void setFilenamePreFix(const QString& newValue);
		void setIncludeTime(bool b) {QMutexLocker lock(m_propertyMutex); m_includeTime = b; emit(includeTimeChanged(b));}
		void setIncludeInt(bool b) {QMutexLocker lock(m_propertyMutex); m_includeInt = b; emit(includeIntChanged(b));}
		void setLogNumber(int i) {QMutexLocker lock(m_propertyMutex); m_logNumber = i; emit(logNumberChanged(i));}
		void setAutoFileName(bool b) {QMutexLocker lock(m_propertyMutex); m_autoFileName = b; emit(autoFileNameChanged(b));}


    signals:
        void directoryChanged(const QString& newValue);
		void filenameChanged(const QString& newValue);
		void filenameChangedPreFix(const QString& newValue);
		void includeTimeChanged(bool newValue);
		void includeIntChanged(bool newValue);
		void logNumberChanged(int newValue);
		void autoFileNameChanged(bool newValue);

    private:
        plv::InputPin<QString>* m_inputPinString;
		
		//realtime gui variables:	
		QString m_directory;
		QString m_filename;
		QString m_filenamePreFix;

		bool m_includeTime;
		bool m_includeInt;
		bool m_autoFileName;
		int m_logNumber;
		QTime time;
    };
}
#endif // STRINGTOFILE_H
