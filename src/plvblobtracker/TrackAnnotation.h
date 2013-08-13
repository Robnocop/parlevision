#ifndef TRACKANNOTATION_H
#define TRACKANNOTATION_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/CvMatDataPin.h>
#include <plvcore/CvMatData.h>
#include <opencv/cv.h>
//#include "Blob.h"
#include "BlobTrack.h"

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
		Q_CLASSINFO("description", "To create a ground truth for tracking purposes, saves data to a textfile.")

		//Q_PROPERTY( bool saveToFile READ getSaveToFile WRITE setSaveToFile NOTIFY saveToFileChanged )
		Q_PROPERTY( bool saveToFile READ getSaveToFile WRITE setSaveToFile NOTIFY saveToFileChanged )
		Q_PROPERTY( bool legacyFormat READ getLegacyFormat WRITE setLegacyFormat NOTIFY legacyFormatChanged )

	
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
		int readFile(QString m_filename2);

		/** required standard method declaration for plv::PipelineProcessor */
		PLV_PIPELINE_PROCESSOR
	signals:
		//void saveToFileChanged (bool b);
		void saveToFileChanged (bool b);
		void legacyFormatChanged(bool b);

	public slots:
		//void setSaveToFile(bool b);
		void setSaveToFile(bool b) {m_saveToFile = b; emit (saveToFileChanged(b));}
		void setLegacyFormat(bool b) {m_legacyFormat = b; emit (legacyFormatChanged(b));}

	private slots:
		//void timeout();
		void mouseaction();
	
	private:
		plv::InputPin< QList<plvblobtracker::BlobTrack> >* m_inputBlobs;
		plv::InputPin< bool >* m_inputAnnotationNeeded;
		plv::InputPin<QString>* m_fileNameInputPin;

		
		plv::OutputPin<QString>* m_outputPin;
		plv::CvMatDataInputPin* m_inputImage;
			

		PlvMouseclick m_plvannotationwidget;

		QWidget* m_popupWidget;
		QLabel m_imlab1;

		QString m_filename;
		QString m_filename2;
		QString m_filename3;

		bool m_saveToFile; 
		bool m_popUpExists;
		bool m_stopwhile;
		bool m_interesting;


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
