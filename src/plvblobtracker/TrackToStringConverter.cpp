#include "TrackToStringConverter.h"

using namespace plv;
using namespace plvblobtracker;

#include <opencv/cv.h>
#include <QFile>

TrackToStringConverter::TrackToStringConverter() :
	m_saveToFile(true),
	m_filename("blobtracklog.txt"),
	m_legacyFormat(true)
{
    m_inputBlobs = createInputPin< QList<BlobTrack> >( "input", this );
    m_outputPin = createOutputPin<QString>( "output", this );
}

TrackToStringConverter::~TrackToStringConverter()
{
}

bool TrackToStringConverter::init()
{
	//m_timenotset = true;
	if (getSaveToFile())
	{
		QFile file(m_filename);
		bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
		Q_ASSERT(ret);
		QTextStream s(&file);
		QString dateAndTime = QTime().currentTime().toString(); //QString("blobs of frame %1").arg(this->getProcessingSerial());
		file.write("\n");
		//file.write("tabbed format:"); 
		//file.write("\n");
		//file.write("BEGIN_MARKER#MARKER_ID: \t #MARKER_CENTER_X: \t #MARKER_CENTER_Y: \t #DIRECTION: \t #SPEED: \t #AVERAGEZ: \t #AGE:");
		//file.write("\n");
		file.write("init time:");
		for (int i = 0; i < dateAndTime.size(); ++i)
			s << dateAndTime.at(i);// << '\n';
		file.write("\n");	
		ret = file.flush();
		Q_ASSERT(ret);
		file.close();
		//m_timenotset = false;
	}
	return true;
}

void TrackToStringConverter::setSaveToFile(bool b)
{
	m_saveToFile = b;
	emit (saveToFileChanged(b));
}

bool TrackToStringConverter::process()
{
    QList<plvblobtracker::BlobTrack> tracks = m_inputBlobs->get();
	//bool savetofile = true;
    QString out;
	if (getLegacyFormat())
	{	 
		out = QString("FRAME:%1#").arg( this->getProcessingSerial() );
	}
	else
	{
		QString dateAndTime = QTime().currentTime().toString();
		out = QString("FRAME: \t %1 \t at time \t %2").arg(this->getProcessingSerial()).arg(dateAndTime);
	}
	//as the framenumber might not be constant it is in order to have the system time which in most cases is more reliable. 
	if (getSaveToFile())
	{
			QFile file(m_filename);
			bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
			Q_ASSERT(ret);
			QTextStream s(&file);
			QString dateAndTime = QTime().currentTime().toString();
			QString frameNumber = QString("FRAME: \t %1 \t at time \t %2").arg(this->getProcessingSerial()).arg(dateAndTime);
			for (int i = 0; i < frameNumber.size(); ++i)
				s << frameNumber.at(i);// << '\n';
			file.write("\n");	
			ret = file.flush();
			Q_ASSERT(ret);
			file.close();
	}

    // convert coordinates to what the virtual playground expects (legacy) otherwise save the info per updated track tab delimited. 
    foreach( BlobTrack t , tracks )
    {
        cv::Point p = t.getLastMeasurement().getCenterOfGravity();
        //p.x = image.width() - p.x;
        //p.y = image.height() - p.y;
		//playgroundQString blobString = QString("BEGIN_MARKER#MARKER_ID:%1#MARKER_CENTER_X:%2#MARKER_CENTER_Y:%3#").arg(t.getID()).arg(p.x).arg(p.y);
		//legacy format, change to tab formatted if in order
		QString blobString;
//		if (getLegacyFormat())
//		blobString = QString("BEGIN_MARKER#MARKER_ID:%1#MARKER_CENTER_X:%2#MARKER_CENTER_Y:%3#DIRECTION:%4#SPEED:%5#AVERAGEZ:%6#AGE:%7").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
//		else
		//NOT NEEDED FOR ANNO blobString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
		QString blobTabString = QString("%1 \t %2 \t %3 \t %4").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAveragePixel());
		out.append(blobString);
		out.append("\n");
		if (getSaveToFile())
		{
			//maybe better to save the values instead of calling them twice. although none of these values seem to require much proccesing power, they only return a value. 
			//NOT NEEDED FOR ANNO 
			QString blobTabString = QString("%1 \t %2 \t %3 \t %4").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAveragePixel());
			//QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
			QFile file(m_filename);
			bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
			Q_ASSERT(ret);
			//Q_ASSERT(blobString.size()>0);
			QTextStream s(&file);
			for (int i = 0; i < blobTabString.size(); ++i)
				s << blobTabString.at(i);// << '\n';
			//file.write(blobString);
			//file.write("APPEND new Line");
			file.write("\n");	
			ret = file.flush();
			Q_ASSERT(ret);
			file.close();
		}
    }

    // end TCP frame with newline, why?
    out.append("\n");

    m_outputPin->put(out);
    return true;
}
