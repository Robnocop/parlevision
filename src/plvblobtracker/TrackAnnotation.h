#ifndef TRACKANNOTATION_H
#define TRACKANNOTATION_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/CvMatDataPin.h>
#include <plvcore/CvMatData.h>
#include <opencv/cv.h>
//#include "Blob.h"
#include "BlobTrack.h"
#include "BlobTracker.h"

#include "PlvMouseclick.h"

#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QtGui>
#include <QScopedPointer>

#include <QVector>

class QPixmap;
class QHBoxLayout;

namespace plvblobtracker
{

	class TrackAnnotation : public plv::PipelineProcessor
	{
		Q_OBJECT
		Q_DISABLE_COPY( TrackAnnotation )

		Q_CLASSINFO("author", "Robby van Delden")
		Q_CLASSINFO("name", "Track Annotation Tool")
		Q_CLASSINFO("description", "To create a ground truth for tracking purposes, saves data to a textfile in tab delimited format 11 items per line/identified track. 1: framefilename 2: t.getId(), 3: p.x 4: p.y 5: t.getAveragePixel() 6: t.getDirection() 7:t.getVelocity2() 8: t.getAge() 9: m_interesting [trackerstate msg] 10: annotationstate [users msg] 11: processingserial")

		//Q_PROPERTY( bool saveToFile READ getSaveToFile WRITE setSaveToFile NOTIFY saveToFileChanged )
		Q_PROPERTY( bool saveToFile READ getSaveToFile WRITE setSaveToFile NOTIFY saveToFileChanged )
		Q_PROPERTY( bool legacyFormat READ getLegacyFormat WRITE setLegacyFormat NOTIFY legacyFormatChanged )
		/** the filename of the logfile to save*/
		Q_PROPERTY( QString filenameLog READ getFilenameLog WRITE setFilenameLog NOTIFY filenameLogChanged )

	
	public:
		TrackAnnotation();
		virtual ~TrackAnnotation();
		virtual bool init();
		virtual bool deinit();
		virtual bool createPopup();
		//virtual bool toPaint(plv::CvMatData& image);
		void mousePressEventCopy(QMouseEvent *event);
		QImage CvMatDataToQImage(plv::CvMatData& input);
	
		//bool getSaveToFile() {return m_saveToFile;}
		bool getSaveToFile() {return m_saveToFile;}
		bool getLegacyFormat() {return m_legacyFormat;}
		bool getStopState();
		bool stop();
		QString getFilenameLog();

		int readFile(QString m_filename2, bool previous);

		/** required standard method declaration for plv::PipelineProcessor */
		PLV_PIPELINE_PROCESSOR
	signals:
		//void saveToFileChanged (bool b);
		void saveToFileChanged (bool b);
		void legacyFormatChanged(bool b);
		void filenameLogChanged(const QString& newValue);

	public slots:
		//void setSaveToFile(bool b);
		void setSaveToFile(bool b) {m_saveToFile = b; emit (saveToFileChanged(b));}
		void setLegacyFormat(bool b) {m_legacyFormat = b; emit (legacyFormatChanged(b));}
		void setFilenameLog(const QString& filename) {m_filenameLog = filename; emit (filenameLogChanged(filename));}

	private slots:
		//void timeout();
		void mouseaction();
	
	private:
		plv::InputPin< QList<plvblobtracker::BlobTrack> >* m_inputBlobs;
		plv::InputPin< bool >* m_inputAnnotationNeeded;
		
		plv::InputPin<QString>* m_fileNameInputPin;
		plv::InputPin<int>* m_fileNameNrInputPin;
		plv::InputPin<bool>* m_correctimagedirectoryboolInputPin;
		plv::InputPin<QList<plvblobtracker::PlvBlobTrackState>>* m_inputBlobTrackState;
		
		plv::OutputPin<QString>* m_outputPin;
		plv::CvMatDataInputPin* m_inputImage;
			
		PlvMouseclick m_plvannotationwidget;

		QWidget* m_popupWidget;
		QLabel m_imlab1;
		bool m_skipprocessloop;
		bool m_skipprocessloopback; //todo seperate the two blocking situations
		bool m_skipprocesslooptrack; //todo seperate the two blocking situations
		bool m_correctimagedirectorybool;
		bool debugging;
		unsigned int m_waitserial;
		int m_back_prev;

		QString saveTrackToAnnotation(QString filename, BlobTrack t, cv::Point p, char annotationstate, int newid);
		void saveBlobChangeDataToFile(QString filename);

		//QString m_filename;
		QString m_filenameFrameNr;
		QString m_filenameBCD; /** the filename of the blobchangedata to save and later on load in the blobtracker to reset the ids*/
		QString m_filenameLog;  /** the filename of the logfile to save*/

		bool m_saveToFile; 
		bool m_popUpExists;
		bool m_stopwhile;
		bool m_interesting;
		bool m_closedwidget; 

		//bool m_annotationneeded;
		//bool m_timenotset;
	   // bool            m_busy;
	   // QMutex          m_busy_mutex;
	   // ImageConverter* m_converter;
	
		bool m_legacyFormat; 
		//bool m_timenotset;

	};

}

#endif // VPBLOBTOSTRINGCONVERTER_H
