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

#ifndef IMAGEDIRECTORYPRODUCER_H
#define IMAGEDIRECTORYPRODUCER_H

#include <QMutex>
#include <plvcore/PipelineProducer.h>
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/Enum.h>
#include <QFileInfo>

namespace plv
{
    class CvMatDataOutputPin;
}

namespace plvopencv
{
    class ImageDirectoryProducer : public plv::PipelineProducer
    {
        Q_OBJECT
        Q_CLASSINFO("author", "Richard Loos")
        Q_CLASSINFO("name", "Image directory producer")
        Q_CLASSINFO("description", "A producer that loads all the images in a directory.");

        Q_PROPERTY( QString directory READ getDirectory WRITE setDirectory NOTIFY directoryChanged )
		Q_PROPERTY( plv::Enum sortType READ getSortType WRITE setSortType NOTIFY sortTypeChanged )
		Q_PROPERTY( int startNumber READ getStartNumber WRITE setStartNumber NOTIFY startNumberChanged )
		Q_PROPERTY( int endNumber READ getEndNumber WRITE setEndNumber NOTIFY endNumberChanged )
		Q_PROPERTY( int wantedFPS READ getWantedFPS WRITE setWantedFPS NOTIFY wantedFPSChanged )
		Q_PROPERTY( bool loopIt READ getLoopIt WRITE setLoopIt NOTIFY loopItChanged )

        /** required standard method declaration for plv::PipelineElement */
        PLV_PIPELINE_PRODUCER

    public:
        ImageDirectoryProducer();
        virtual ~ImageDirectoryProducer();

		/** propery methods */
        plv::Enum getSortType() { return m_sort; }
		int getStartNumber() {return m_start;}
		int getEndNumber() {return m_end;}
		int getWantedFPS() {return m_fps;}
		bool getLoopIt() {return m_loop;}
		//is the mutexlocker needed here?
		QString getDirectory() {QMutexLocker lock( m_propertyMutex ); return m_directory; };

		//void setAveragePixelValue(bool i) {m_averagePixelValue = i; emit (averagePixelValueChanged(i));}
		int ByName;
		int ByTime;
		int BySize;
		int ByType;
		int ByUnsorted;
		int ByNoSort;
		int ByDirsFirst;
		int ByDirsLast;
		int ByReversed;
		int ByIgnoreCase;
		int ByLocaleAware;
		
		//another solution
		int WithNumbers;
        
		virtual bool init();
        virtual bool deinit() throw ();

        
        //is this used??
		//void updateDirectory(const QString& s){ setDirectory(s); directoryChanged(s); }

    signals:
        void directoryChanged(const QString& newValue);
		void sortTypeChanged(plv::Enum newValue);
		void startNumberChanged(int i);
		void endNumberChanged(int i);
		void wantedFPSChanged(int i);
		void loopItChanged (bool b);

    public slots:
		//???void setDirectory(const QString& newDir); dont know the results 
		void setDirectory(const QString& newValue);
		void setSortType(plv::Enum e);
		void setStartNumber(int i) {m_start = i; emit (startNumberChanged(i)); } //init();
		void setEndNumber(int i) {m_end = i; emit (endNumberChanged(i)); } //init();
		void setWantedFPS(int i) {m_fps = i; emit (wantedFPSChanged(i));}
		void setLoopIt(bool b) {m_loop = b; emit (loopItChanged(b));}

    protected:
        plv::CvMatData m_loadedImage;
        plv::CvMatDataOutputPin* m_imgOutputPin;
        plv::OutputPin<QString>* m_fileNameOutputPin;
        plv::OutputPin<QString>* m_filePathOutputPin;
		plv::Enum m_sort;

    private:
		QTime m_timeSinceLastFPSCalculation;
        QString m_directory;
        QFileInfoList m_entryInfoList;
        int m_idx;
		unsigned int m_nr;
		int m_start;
		int m_end;
		int m_fps;
		bool m_loop;
    };
}

#endif // IMAGEDIRECTORYPRODUCER_H
