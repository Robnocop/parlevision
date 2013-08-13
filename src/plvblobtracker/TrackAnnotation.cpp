#include "TrackAnnotation.h"

using namespace plv;
using namespace plvblobtracker;

#include <plvcore/CvMatData.h>
#include <QFile>
#include <QDebug>
#include <QtGui>

#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QHBoxLayout>
#include <QScopedPointer>

#include "PlvMouseclick.h"

TrackAnnotation::TrackAnnotation() :
	m_saveToFile(false),
	m_filename("annotationlog.txt"),
	m_filename2("framenr.txt"),
	m_filename3("blobtrackchange.txt"),
	m_popUpExists(false),
	m_legacyFormat(true),
	m_interesting(true)
{
	
	m_fileNameInputPin = createInputPin<QString> ("filename of frame", this);
    m_inputImage = createCvMatDataInputPin( "blob track image", this );
	m_inputAnnotationNeeded = createInputPin<bool>("annotation need", this);
	m_inputBlobs = createInputPin< QList<BlobTrack> >( "input tracks", this );

    m_inputImage->addAllChannels();
    m_inputImage->addAllDepths();

    m_outputPin = createOutputPin<QString>( "output", this );
	//doesnt work due to cycle check:
	//m_changeFramePin = createOutputPin<int>( "frame changer", this );
}

TrackAnnotation::~TrackAnnotation()
{
	if(m_popUpExists)
	{
		m_popupWidget->close ();
	}
	m_stopwhile = true;
	qDebug() << "~ in trackannotation";
}


bool TrackAnnotation::deinit() throw ()
{
	m_stopwhile = true;
	m_plvannotationwidget.deinit();
	qDebug() << "deinit in trackannotation";
    return true;
}

bool TrackAnnotation::stop()
{
    m_stopwhile = true;
	qDebug() << "stop the pipeline qdebug";
	return true;
}


bool TrackAnnotation::init()
{
	/*if(!m_popUpExists)
	{*/
	m_stopwhile = false;
	m_plvannotationwidget.init();
	//m_plvannotationwidget.createPopUp();
	createPopup();
	m_popUpExists = m_plvannotationwidget.m_popUpExists; 
	
	/*else if (!m_popupWidget->isVisible())
	{
		m_popUpExists = false;
		createPopup();
	}*/

	return true;
}


//probably need to create a seperate popup-widget class myself in order to deal with the mouse events
bool TrackAnnotation::createPopup()
{
	//use a class instead:
	//PlvMouseclick testunit; //= new PlvMouseclick();
	//a global instance is needed: m_plvannotationwidget;
	m_plvannotationwidget.createPopUp();
	
	m_popUpExists = m_plvannotationwidget.m_popUpExists; 
	
	//works
	/*qDebug() << "before " << m_plvannotationwidget.m_mouseclick;
	m_plvannotationwidget.testFunction();
	qDebug() << "after " << m_plvannotationwidget.m_mouseclick;
*/

	//connect( m_plvannotationwidget.mouseReleaseEvent(), SIGNAL(mouseaction()), this, SLOT(mouseaction());

	/*m_popUpExists = true;*/
	//signals? 
	return true;
}

//would be better to have a general class function as this is used by several classes of blobtracker
int TrackAnnotation::readFile(QString filename) 
{
	int framechangeInt = 0;
	QFile inFile(filename);
	if(inFile.exists())
	{
		if ( inFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) 
		{
			QString processingserial,framechange;
			double processingserialDouble;
			
			QTextStream stream( &inFile );
			QString line;

			for (int counter = 1; counter < 2; counter++) {
				line = stream.readLine(); 
				// line of text excluding '\n'
			}

			processingserialDouble = line.section('\t', 0,0).toDouble();
			framechangeInt = line.section('\t', 1,1).toInt();
			qDebug() << "processingserial" << this->getProcessingSerial() << " read" << processingserialDouble << "change val" << framechangeInt;
			//do stuff with the temp strings
			
			//if (processingserialDouble < (this->getProcessingSerial()) && processingserialDouble > (this->getProcessingSerial()-3) )
		}
	}
	inFile.close();
	return framechangeInt;
}

//?does this do anything? I dont think so:
void TrackAnnotation::mouseaction()
{
	qDebug() << "mouseclick";
}

//?does this do anything? I dont think so:
void TrackAnnotation::mousePressEventCopy(QMouseEvent *event)
{
	event->accept();
}

bool TrackAnnotation::getStopState()
{
	if (m_plvannotationwidget.framestate != NotSet)
	{
		m_stopwhile = true;
		qDebug() << "no longer not notset" << m_plvannotationwidget.framestate;
	}
	else if(m_plvannotationwidget.m_error)
	{
		m_stopwhile = true;
	}
	
	return m_stopwhile;
}

bool TrackAnnotation::process()
{
	//m_stopwhile = false; //false would block, false would meen no annotation needed
	//if annotation needed is true then stop(the-while loop) should be false
	m_stopwhile = !m_inputAnnotationNeeded->get();
	

	//if going back in frames always block until proceeding again
	if (m_stopwhile == true)
	{
		if(readFile(m_filename2)<1)
		{	
			qDebug() << "readfile <1 :" << readFile(m_filename2);
			m_stopwhile = false;
		}
	}

	m_plvannotationwidget.processInit();
	
	//get image of tracks and put it
	QList<plvblobtracker::BlobTrack> tracks = m_inputBlobs->get();
	CvMatData image = m_inputImage->get(); 
	//cv::Mat& dst = image;
	m_plvannotationwidget.toPaint(image);
	
	int back = 0;	
	//wait for a mouse click before proceeding
	//will result in a crash when parlevision is stopped!
	//can't be tested in debug because of the use of different threads according to debug error.

	//probably a readytoproduce && produce would work better
	//while (m_plvannotationwidget.m_mouseclick==false && (m_stopwhile!=true))

	//wait for the popup to give input (when needed)
	while (m_stopwhile==false) 
	{
	  /*if (m_plvannotationwidget.m_mouseclick==true)
			break;*/
	  //qDebug() << "I enter loop";
		
		//processloop in testunit?

	  if (getStopState())
	  {
		break;
	  }
	}

	m_interesting = m_plvannotationwidget.m_dontskipframeitisinteresting;
	//why?
	/*if (!getStopState())
		return true;*/

	//have the popup change frame
	//TO ALTER THE FRAME/ TO REWIND
	if (getStopState())
	{
		//default next frame, in imagedirectory +1 is added 
		back = 0;
		if (m_plvannotationwidget.framestate == GotoPreviousFrame)
			back = -2;
		if (m_plvannotationwidget.framestate == GotoCurrentFrame)
			back = -1;
		if (m_plvannotationwidget.m_error)
		{
			back = 999;
		}
		
		//m_changeFramePin->put(back); //does not work with a cycle
		//instead write text file to message other pipeline elements for the next process loop
		QString changeframe = QString("%1 \t %2").arg(this->getProcessingSerial()).arg(back);
		QFile file2(m_filename2);
		bool ret2 = file2.open(QIODevice::WriteOnly | QIODevice::Truncate);
		Q_ASSERT(ret2);
		////Q_ASSERT(blobString.size()>0);
		QTextStream s(&file2);
		s << changeframe;
		//for (int i = 0; i < blobTabString.size(); ++i)
		//	s << blobTabString.at(i);// << '\n';
		////file.write(blobString);
		////file.write("APPEND new Line");
		ret2 = file2.flush();
		Q_ASSERT(ret2);
		file2.close();
	}
	
	//to save corrected tracks data:
	QString out;
	if (getLegacyFormat())
	{	 
		out = QString("FRAME:%1#").arg( this->getProcessingSerial() );
	}
	
	QString framefilename = m_fileNameInputPin->get();

	foreach( BlobTrack t , tracks )
	{
		cv::Point p = t.getLastMeasurement().getCenterOfGravity();
	
		QString blobString;
		
		//legacy format, change to tab formatted if in order	
		if (getLegacyFormat())
			blobString = QString("BEGIN_MARKER#MARKER_ID:%1#MARKER_CENTER_X:%2#MARKER_CENTER_Y:%3#DIRECTION:%4#SPEED:%5#AVERAGEZ:%6#AGE:%7").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
		else
			blobString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \t %8").arg(framefilename).arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
		out.append(blobString);
		out.append("\n");

		if (getSaveToFile())
		{
			//maybe better to save the values instead of calling them twice. although none of these values seem to require much proccesing power, they only return a value. 
			//QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \t %8").arg(framefilename).arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
			//cleaned up:
			QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \t %8 \t %9").arg(framefilename).arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAveragePixel()).arg(m_interesting).arg(t.getDirection()).arg(t.getVelocity2()).arg(t.getAge());
			QFile file(m_filename);
			bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
			Q_ASSERT(ret);
			QTextStream s(&file);
			for (int i = 0; i < blobTabString.size(); ++i)
				s << blobTabString.at(i);// << '\n';
			file.write("\n");	
			ret = file.flush();
			Q_ASSERT(ret);
			file.close();
		}
	}

    //out.append("\n");

    m_outputPin->put(out);
	
	return true;
}

//OLD
//CREATE POPUP STUFF
//	//?is this possible?	
//	QMutexLocker lock(m_propertyMutex);
//	if(!m_popUpExists)
//	{		
//		m_popupWidget = new QWidget();
//	}
//	m_popupWidget->setWindowFlags(Qt::Popup);
//	m_popupWidget->resize(640, 480);
//	m_popupWidget->move(0, 10);
//
//	//ALIGN:
//	m_imlab1.setAlignment(Qt::AlignCenter);
//	// create horizontal layout
//    QHBoxLayout* layoutqhb =new QHBoxLayout;
//    // and add label to it
//    layoutqhb->addWidget(&m_imlab1);
//    // set layout to widget
//    m_popupWidget->setLayout(layoutqhb);
////	m_popupWidget->autohide(false); //is set but is not a member of the qwidget class
//    
//	m_popupWidget->installEventFilter(this);
//
//	//QMouseEvent *mouseclick = m_popupWidget->mouseReleaseEvent();
//	//connect( mouseclick, SIGNAL(mouseaction()), &this->parent, SLOT(mouseaction()));
//	
//	//QTimer *timer = new QTimer( this );
//	//connect( timer, SIGNAL(timeout()), this, SLOT(timeout));
//
//	m_popupWidget->show();
//	//mousePressEventCopy(&m_popupWidget->mouseReleaseEvent);
//	//apparently doesnt make sense: m_popupWidget->setWindowTitle("annotationwidget");
//	

////TOWARDS SOLUTION
//From opencvimagerenderer:
//m_layout      = new QHBoxLayout(ImageWidget *parent);
//   m_imageWidget = new ImageWidget;
//   m_layout->addWidget( m_imageWidget );
//   QImage image = QImage(320,240,QImage::Format_RGB32);
//   image.fill( qRgb(0,0,0) );
//   // TODO make minimum configurable somewhere
//   m_imageWidget->setMinimumSize( 160, 120 );


//legacy interactive playground notation
//foreach( BlobTrack t , tracks )
//  {
//      cv::Point p = t.getLastMeasurement().getCenterOfGravity();
//      //p.x = image.width() - p.x;
//      //p.y = image.height() - p.y;
////playgroundQString blobString = QString("BEGIN_MARKER#MARKER_ID:%1#MARKER_CENTER_X:%2#MARKER_CENTER_Y:%3#").arg(t.getID()).arg(p.x).arg(p.y);
////legacy format, change to tab formatted if in order
//QString blobString;
//if (getLegacyFormat())
//	blobString = QString("BEGIN_MARKER#MARKER_ID:%1#MARKER_CENTER_X:%2#MARKER_CENTER_Y:%3#DIRECTION:%4#SPEED:%5#AVERAGEZ:%6#AGE:%7").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
//else
//	blobString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
//out.append(blobString);
//out.append("\n");
//if (getSaveToFile())
//{
//	//maybe better to save the values instead of calling them twice. although none of these values seem to require much proccesing power, they only return a value. 
//	QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge());
//	QFile file(m_filename);
//	bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
//	Q_ASSERT(ret);
//	//Q_ASSERT(blobString.size()>0);
//	QTextStream s(&file);
//	for (int i = 0; i < blobTabString.size(); ++i)
//		s << blobTabString.at(i);// << '\n';
//	//file.write(blobString);
//	//file.write("APPEND new Line");
//	file.write("\n");	
//	ret = file.flush();
//	Q_ASSERT(ret);
//	file.close();
//}
//  