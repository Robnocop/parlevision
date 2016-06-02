#include "TrackToStringConverter.h"

using namespace plv;
using namespace plvblobtracker;

#include <opencv/cv.h>
#include <QFile>

TrackToStringConverter::TrackToStringConverter() :
	m_saveToFile(true),
	m_legacyFormat(true),
	m_filename("blobtracklog.txt"),
	halfhour(0)
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
		file.write("tabbed format:"); 
		file.write("\n");
		file.write("BEGIN_MARKER#MARKER_ID: \t #MARKER_CENTER_X: \t #MARKER_CENTER_Y: \t #DIRECTION: \t #SPEED: \t #AVERAGEZ: \t #AGE:");
		file.write("\n");
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
	//if (b)
	//{
	//	QFile file(m_filename);
	//	bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
	//	Q_ASSERT(ret);
	//	QTextStream s(&file);
	//	file.write("set to save time:");
	//	QString dateAndTime = QTime().currentTime().toString(); //QString("blobs of frame %1").arg(this->getProcessingSerial());
	//	for (int i = 0; i < dateAndTime.size(); ++i)
	//		s << dateAndTime.at(i);// << '\n';
	//	file.write("\n");	
	//	ret = file.flush();
	//	Q_ASSERT(ret);
	//	file.close();
	//	//m_timenotset = false;
	//}
	emit (saveToFileChanged(b));
}

bool TrackToStringConverter::process()
{
    QList<plvblobtracker::BlobTrack> tracks = m_inputBlobs->get();
	//? old solution
	//bool savetofile = true;
	
	//set to empty
	QString out  = QString("");
    if (getLegacyFormat())
		QString out = QString("FRAME:%1#").arg( this->getProcessingSerial() );
	

	if (getSaveToFile())
	{
			QFile file(m_filename);
			bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
			Q_ASSERT(ret);
			QTextStream s(&file);
			QString dateAndTime = QTime().currentTime().toString();
			QString frameNumber = QString("FRAME: \t %1 \t at time \t %2").arg(this->getProcessingSerial()).arg(dateAndTime);;
			for (int i = 0; i < frameNumber.size(); ++i)
				s << frameNumber.at(i);// << '\n';
			file.write("\n");	
			ret = file.flush();
			Q_ASSERT(ret);
			file.close();
	}
    // convert coordinates to what the virtual playground expects
    foreach( BlobTrack t , tracks )
    {
        cv::Point p = t.getLastMeasurement().getCenterOfGravity();
        //p.x = image.width() - p.x;
        //p.y = image.height() - p.y;
		//playgroundQString blobString = QString("BEGIN_MARKER#MARKER_ID:%1#MARKER_CENTER_X:%2#MARKER_CENTER_Y:%3#").arg(t.getID()).arg(p.x).arg(p.y);
		//legacy format, change to tab formatted if in order
		
		if (getLegacyFormat())
		{
			QString blobString = QString("BEGIN_MARKER#MARKER_ID:%1#MARKER_CENTER_X:%2#MARKER_CENTER_Y:%3#DIRECTION:%4#SPEED:%5#AVERAGEZ:%6#AGE:%7").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
			out.append(blobString);
			out.append("\n");
		}
		else
		{
			//THE # is introduced as a new split \t seemed to result in some issues for the blox processor!
			//TODO add size of blob
			QString blobTabString2 = QString("%1 \t #%2 \t #%3 \t #%4 \t #%5 \t #%6 \t #%7 \t #%8").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge()).arg(t.getBlobSize()); 
			out.append(blobTabString2);
			out.append("\n");
		}

		if (getSaveToFile())
		{
			//maybe better to save the values instead of calling them twice. although none of these values seem to require much proccesing power, they only return a value. 
			QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
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
	//used to be at the bottom of the file
	if (tracks.count()==0)
		out.append("no data");

	if (getLegacyFormat())
	{
		out.append("\n");
	}
	
	//if (getSaveToFile())
	//{
	//	if (halfhour>54000)
	//	//if (halfhour>60)
	//	{
	//		halfhour = 0;
	//		QFile file(m_filename);
	//		bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
	//		Q_ASSERT(ret);
	//		QTextStream s(&file);
	//		QString dateAndTime = QTime().currentTime().toString(); //QString("blobs of frame %1").arg(this->getProcessingSerial());
	//		file.write("current time:");
	//		for (int i = 0; i < dateAndTime.size(); ++i)
	//			s << dateAndTime.at(i);// << '\n';
	//		file.write("\n");	
	//		ret = file.flush();
	//		Q_ASSERT(ret);
	//		file.close();
	//		//m_timenotset = false;
	//	}
	//	else
	//	{
	//		halfhour++;
	//	}
	//}
    // end TCP frame with newline, why?
    
	

    m_outputPin->put(out);
    return true;
}
