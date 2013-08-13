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

//set to caps:
#define BYNAME 0
#define BYTIME 1 
#define BYSIZE 2  
#define BYTYPE 3 
#define BYUNSORTED 4 
#define BYNOSORT 5 
#define BYDIRSFIRST 6
#define BYDIRSLAST 7
#define BYREVERSED 8
#define BYIGNORECASE 9
#define BYLOCALEAWARE 10
#define WITHNUMBERS 11

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
        Q_CLASSINFO("author", "Richard Loos heavily rewritten by Robby")
        Q_CLASSINFO("name", "Image directory producer")
        Q_CLASSINFO("description", "A producer that loads all the images in a directory. We recommend only to use the WithNumbers method for large directories of image files.");

        Q_PROPERTY( QString directory READ getDirectory WRITE setDirectory NOTIFY directoryChanged )
		Q_PROPERTY( QString directoryRGB READ getDirectoryRGB WRITE setDirectoryRGB NOTIFY directoryRGBChanged )
		Q_PROPERTY( plv::Enum sortType READ getSortType WRITE setSortType NOTIFY sortTypeChanged )
		Q_PROPERTY( int startNumber READ getStartNumber WRITE setStartNumber NOTIFY startNumberChanged )
		Q_PROPERTY( int endNumber READ getEndNumber WRITE setEndNumber NOTIFY endNumberChanged )
		Q_PROPERTY( int wantedFPS READ getWantedFPS WRITE setWantedFPS NOTIFY wantedFPSChanged )
		Q_PROPERTY( int trailingZeros READ getTrailingZeros WRITE setTrailingZeros NOTIFY trailingZerosChanged )
		Q_PROPERTY( bool loopIt READ getLoopIt WRITE setLoopIt NOTIFY loopItChanged )
		Q_PROPERTY( bool annotation READ getAnnotation WRITE setAnnotation NOTIFY annotationChanged )

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
		bool getAnnotation() {return m_annotation;}
		int getTrailingZeros() {return m_trailingZeros;}
		
		//is the mutexlocker needed here?
		QString getDirectory() {QMutexLocker lock( m_propertyMutex ); return m_directory; };

		QString getDirectoryRGB() {QMutexLocker lock( m_propertyMutex ); return m_directoryRGB; };

		//void setAveragePixelValue(bool i) {m_averagePixelValue = i; emit (averagePixelValueChanged(i));}

		virtual bool init();
        virtual bool deinit() throw ();

        
        //is this used??
		//void updateDirectory(const QString& s){ setDirectory(s); directoryChanged(s); }

    signals:
        void directoryChanged(const QString& newValue);
		//RGB
		void directoryRGBChanged(const QString& newValueRGB);

		void sortTypeChanged(plv::Enum newValue);
		void startNumberChanged(int i);
		void endNumberChanged(int i);
		void wantedFPSChanged(int i);
		void loopItChanged (bool b);
		void trailingZerosChanged(bool b);
		
		//ugly and temp solution :
		void annotationChanged(bool b);
		
		
    public slots:
		//???void setDirectory(const QString& newDir); dont know the results 
		void setDirectory(const QString& newValue);
		//RGB
		void setDirectoryRGB(const QString& newValueRGB);

		void setSortType(plv::Enum e);
		//allows to reset the number, we now don't set it twice in init as it allows for a pause and return
		void setStartNumber(int i) {m_start = i; m_nr = m_start; m_idx = m_start; emit (startNumberChanged(i)); } //init();
		void setEndNumber(int i) {m_end = i; emit (endNumberChanged(i)); } //init();
		void setWantedFPS(int i) {m_fps = i; emit (wantedFPSChanged(i));}
		void setLoopIt(bool b) {m_loop = b; emit (loopItChanged(b));}
		void setAnnotation(bool b) {m_annotation = b; emit (annotationChanged(b));}
		void setTrailingZeros(int i) {m_trailingZeros = i; emit (trailingZerosChanged(i));}

    protected:
        plv::CvMatData m_loadedImage;
        plv::OutputPin<QString>* m_fileNameOutputPin;
		plv::CvMatDataOutputPin* m_imgOutputPin;
		//TEMP HACK RGB
		plv::CvMatDataOutputPin* m_imgOutputPinRGB;
        		
        plv::OutputPin<QString>* m_filePathOutputPin;
		plv::Enum m_sort;
		
    private:
		QTime m_timeSinceLastFPSCalculation;
        QString m_directory;

		QFileInfoList m_entryInfoList;
		//temp hack RGB
		QFileInfoList m_entryInfoListRGB;
		QString m_directoryRGB;
        
		bool resetFile();
		int readFile(QString filename);
		QFileInfoList loadImageDir(QDir dir);

        int m_idx;
		unsigned int m_nr;
		int m_start;
		int m_end;
		int m_fps;
		int m_trailingZeros;
		bool m_loop;
		bool m_flagTimer;
		bool m_flagpaused;
		QString m_imgtype;
		
		//ugly solution:
		bool m_annotation;
		QString m_filename2;
		//plv::InputPin<int>* m_changeFrame;
    };
}

#endif // IMAGEDIRECTORYPRODUCER_H
