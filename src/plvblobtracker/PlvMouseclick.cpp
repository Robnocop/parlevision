﻿#include "PlvMouseclick.h"
#include <stdlib.h>
#include <limits>

#include <plvcore/CvMatData.h>
//using namespace plv;
//using namespace plvblobtracker;

//added for debuging offcourse
#include <QDebug>

//gui stuff:
#include <QImage>
#include <QWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QShortcut>

#include <QPainter>
#include <QHBoxLayout>
#include <QScopedPointer>

#include "BlobChangeData.h"

using namespace plv;
using namespace plvblobtracker;


PlvMouseclick::PlvMouseclick()
{
	//init shizzle
	m_filenameBCD = "blobtrackchange.txt"; //will set the blobtrack changed data, switches etc. , to be loaded into the tracker to swithc the ids etc for once and all to prevent restoring same mistake over and over
	//m_filenameLog = "annotationlog.txt";
	m_filename = "?.png";
	m_mouseclick = false;
	m_popUpExists = false;
	m_rightbottom = false;
	m_error = false;
	framestate = NotSet;
	blobtracksetting = Neutral;
	donotcontinue = false;
	m_dontskipframeitisinteresting = true;
	m_firstid =true;
	m_secondid = true; // a little strange but this is set to false later on if it is a third id
}

PlvMouseclick::~PlvMouseclick()
{
	m_mouseclick = false;
	m_error =true;
	m_dontskipframeitisinteresting = true;
	blobtracksetting = Neutral;
	m_firstid = true;
	m_secondid = true;
	//deinit shizzle
}

void PlvMouseclick::deinit()
{
	//deinit shizzle
	qDebug() << "plvmouseclick has been deint";
	m_mouseclick = false;
	m_error=true;
	m_popUpExists = false;
	m_dontskipframeitisinteresting = true;
	framestate = GotoNextFrame;
	blobtracksetting = Neutral;
	m_blobchanges.clear();
	this->hide();
	this->close();
}

void PlvMouseclick::processInit(int framenr)
{
	framestate = NotSet;
	blobtracksetting = Neutral;
	m_dontskipframeitisinteresting = true;
	m_firstid = true;
	m_secondid = true;
	m_inputnr = 0;
	//TODO reset m_blobchanges?
	m_blobchanges.clear();
	m_framenr = framenr;
}

void PlvMouseclick::init()
{
	//this->close();
	m_framenr = 0;
	m_inputnr = 0;
	m_error = false;
	m_mouseclick = false;
	m_popUpExists = false;
	m_rightbottom = false;
	m_dontskipframeitisinteresting = true;
	framestate = NotSet;
	blobtracksetting = Neutral;
	donotcontinue = false;
	m_firstid = true;
	m_secondid = true;
	m_blobchanges.clear();
}

//functions:
void PlvMouseclick::testFunction()
{
	m_mouseclick = true;
	m_popUpExists = false;
}

void PlvMouseclick::setNext() 
{
	qDebug() << "next";
	m_rightbottom = true;
}

void PlvMouseclick::closeDown()
{
	m_rightbottom = true;
	m_popUpExists = false;
	qDebug() << "windows closed"; 
	m_error = true;
	//setError(PlvPipelineRuntimeError, tr("Cannot continue annotation video playback"));
	throw std::runtime_error("Cannot continue annotation video playback");
	this->close();
}

void PlvMouseclick::closeEvent(QCloseEvent *event)
{
	m_rightbottom = true;
	m_popUpExists = false;
	qDebug() << "windows closed"; 
	m_error = true;
	//TODO i dont think normal throw is allowed however, plvseterror will also not be allowed
	throw std::runtime_error("Cannot continue annotation video playback");
	//plv::PipelineElement::setError(plv::PlvErrorType, tr("Cannot continue annotation video playback"));
	
	this->close();
}

//correct number is painted
bool PlvMouseclick::toPaint(CvMatData& image)
{
	int x,y,w,h;

    w = image.width();// * m_zoomFactor;
    h = image.height();//* m_zoomFactor;
    x = 0;
    y = 0;
	
	//m_popupWidget->resize(image.width(), image.height());
	this->resize(image.width(), image.height());
	
    QRectF target(x,y,w,h);
	QPixmap pixmap(w,h);
    QPainter p(&pixmap);
	QImage qtimage = CvMatDataToQImage(image);
    p.drawImage(target, qtimage);

	m_imlab1.setPixmap(pixmap); 
	return true;
}

//quite an expensive function it seems!

QImage PlvMouseclick::CvMatDataToQImage(CvMatData& imageMat)
{
	cv::Mat& input = imageMat;
	
 /*   if (!input)
        return 0;*/
	//if (!imageMat.width()>0)
		// return 0; //needs to be a qimage null not accepted
	//	return null;

	QImage image(imageMat.width(), imageMat.height(), QImage::Format_RGB32);

    uchar* pBits = image.bits();
    int nBytesPerLine = image.bytesPerLine();

    for (int n = 0; n < imageMat.height(); n++)
    {
        for (int m = 0; m < imageMat.width(); m++)
        {
			//QRgb v = input.at<unsigned short>(n,m);
			cv::Vec3b bgrPixel = input.at<cv::Vec3b>(n, m);
			QRgb value = qRgb((uchar)bgrPixel.val[2], (uchar)bgrPixel.val[1], (uchar)bgrPixel.val[0]);

			//CvScalar s = cvGet2D(input, n, m);
           // QRgb value = qRgb((uchar)s.val[2], (uchar)s.val[1], (uchar)s.val[0]);
			
			//greyscale
			//unsigned int v = input.at<unsigned short>(n,m);
			//QRgb value = qRgb((uchar)v, (uchar)v, (uchar)v);
			
            uchar* scanLine = pBits + n * nBytesPerLine;
            ((uint*)scanLine)[m] = value;
        }
    }

    return image;
}


void PlvMouseclick::createPopUp()
{
	//?is this possible?	
	//QMutexLocker lock(m_propertyMutex);
	if(!m_popUpExists)
	{	
			//m_popupWidget = new QWidget();
	
		//m_popupWidget->setWindowFlags(Qt::Popup);
		//m_popupWidget->resize(640, 480);
		//m_popupWidget->move(0, 10);

		//
		//this->setWindowFlags(Qt::Popup);
		//this->move(0, 10);

		this->resize(640, 480);
	
		//ALIGN:
		m_imlab1.setAlignment(Qt::AlignCenter);
		// create horizontal layout
		QHBoxLayout* layoutqhb =new QHBoxLayout;
		// and add label to it
		layoutqhb->addWidget(&m_imlab1);
		// set layout to widget
		//m_popupWidget->setLayout(layoutqhb);
		this->setLayout(layoutqhb);
	
		this->setWindowTitle("annotationwidget");
		this->show();
		//apparently doesnt make sense: m_popupWidget->setWindowTitle("annotationwidget");
		m_popUpExists = true;
	}
}


//EVENTS

void PlvMouseclick::keyPressEvent(QKeyEvent * event)
{
	//qDebug() << "key pressed" << event->key();
}


//http://harmattan-dev.nokia.com/docs/library/html/qt4/qt.html#Key-enum
void PlvMouseclick::keyReleaseEvent(QKeyEvent * event)
{
	QString windowname = "";
	QString changetypechar = " ";
	int undidpidnr = -1;
	int undidtracknr = -1;
	//qDebug() << "key released" << event->key();
	switch(event->key())
	{
		//DEALING WITH FRAME PROCEEDING
		case Qt::Key_Left:
			qDebug() << "Pressed Left";
			framestate = GotoPreviousFrame;
		break;

		case Qt::Key_Right:
			qDebug() << "Pressed Right";
			framestate = GotoNextFrame;
		break;

		case Qt::Key_Down:
			qDebug() << "Pressed Down";
			framestate = GotoCurrentFrame;
		break;

		//DEALING WITH TRACK CHANGE:
		//is it mutually exclusive, no it is not so multiple things should be setable in one frame, before proceeding
		case Qt::Key_Escape:
			qDebug() << "Should undo";
			blobtracksetting = Undo;
			m_firstid = true;
			m_secondid = true;
			if (!m_blobchanges.empty()) 
			{
				changetypechar = m_blobchanges.last().changetype;
				undidtracknr = m_blobchanges.last().oldid;
				undidpidnr = m_blobchanges.last().newid;
			}
			windowname = QString("anno %1, undid last %2 blobchange state for track %3 with pid# %4 , last input %5").arg(m_filename).arg(changetypechar).arg(undidtracknr).arg(undidpidnr).arg(m_inputnr);
			this->setWindowTitle(windowname);
			m_inputnr = 0;
			//removes last item if it exists
			setTheState("U");
			qDebug() << "Pressed Escape";
			//framestate = GotoCurrentFrame;
		break;
		
		case Qt::Key_U:
			qDebug() << "Should undo";
			blobtracksetting = Undo;
			m_firstid = true;
			m_secondid = true;
			//TODO perhaps add the last removed id in the windowtitle
			if (!m_blobchanges.empty()) 
			{
				changetypechar = m_blobchanges.last().changetype;
			}
			windowname = QString("anno %1, undid last %2 blobchange state, last nr %3").arg(m_filename).arg(changetypechar).arg(m_inputnr);
			this->setWindowTitle(windowname);
			m_inputnr = 0;
			//removes last item if it exists
			setTheState("U");
			qDebug() << "Pressed U";
			//framestate = GotoCurrentFrame;
		break;

		//case Qt::Key_N:
		case Qt::Key_Minus:
			blobtracksetting = Neutral;
			m_firstid = true;
			m_secondid = true;
			if (!m_blobchanges.isEmpty())
			{
				qDebug() << "neutral: last-changes" << m_blobchanges.last().oldid << "new" << m_blobchanges.last().newid << "last cog:" << m_blobchanges.last().cogs.x << "," << m_blobchanges.last().cogs.y;
				//m_blobchanges.replace(m_blobchanges.last(),exchanging);
			}		
			qDebug() << "Pressed-";
			windowname = QString("anno %1, annotationwidget, neutral state").arg(m_filename);
			this->setWindowTitle(windowname);
		break;
		
		case Qt::Key_N:
			blobtracksetting = New;
			m_firstid = false;
			m_secondid = true;
			setTheState("N");
			qDebug() << "Pressed N";
			windowname = QString("anno %1, annotationwidget, new state").arg(m_filename);
			this->setWindowTitle(windowname);
		break;

		case Qt::Key_A:
			blobtracksetting = Add;
			m_firstid = true;
			//visual studio does not allow to create variables in a case statement
			setTheState("A");
			windowname = QString("anno %1, is in add state").arg(m_filename);
			this->setWindowTitle(windowname);			
		break;

		case Qt::Key_R:
			blobtracksetting = Remove;
			m_firstid = true;
			//visual studio does not allow to create variables in a case statement
			setTheState("R");
			windowname = QString("anno %1, is in remove state").arg(m_filename);
			this->setWindowTitle(windowname);	
		break;
		
		case Qt::Key_I:
			blobtracksetting = Insert;
			m_firstid = true;
			m_secondid = true;
			//visual studio does not allow to create variables in a case statement
			setTheState("I");
			windowname = QString("anno %1, is in insert state").arg(m_filename);
			this->setWindowTitle(windowname);	
		break;
		

		//ID stuff:
		case Qt::Key_0:
			//react on number according to current state
			numberKeyHandling(0,false);
		break;

		case Qt::Key_1:
			numberKeyHandling(1,false);	
		break;
		
		case Qt::Key_2:
			numberKeyHandling(2,false);
		break;

		case Qt::Key_3:
			numberKeyHandling(3,false);
		break;
		
		case Qt::Key_4:
			numberKeyHandling(4,false);
		break;

		case Qt::Key_5:
			numberKeyHandling(5,false);
		break;

		case Qt::Key_6:
			numberKeyHandling(6,false);
		break;

		case Qt::Key_7:
			numberKeyHandling(7,false);
		break;

		case Qt::Key_8:
			numberKeyHandling(8,false);
		break;

		case Qt::Key_9:
			numberKeyHandling(9,false);
		break;

		case Qt::Key_Comma:
			numberKeyHandling(0,true);
			if (!m_blobchanges.empty())
			{
				if (!m_firstid)
				{
					qDebug()<< "blobchanges are not empty, you did push comma, please dont and follow the right protocol";
					//perhaps if it happens often allow for comma to be seen as an enter 
					//saveBlobChangeDataToFileOnEnter(m_filenameBCD, m_blobchanges.last());
					windowname = QString("anno %1, track %2 has changetype %3 and currently pid %4 ").arg(m_filename).arg(m_blobchanges.last().oldid).arg(m_blobchanges.last().changetype).arg(m_blobchanges.last().newid);
					this->setWindowTitle(windowname);
				}
			}
			else
			{
				qDebug()<< "blobchanges are empty";
			}
		break;

		case Qt::Key_Enter:
			//numberKeyHandling(0,true);
			qDebug()<< "pushed enter";
			numberKeyHandling(0,true);
			if (!m_blobchanges.empty())
			{
				qDebug()<< "blobchanges are not empty";
				saveBlobChangeDataToFileOnEnter(m_filenameBCD, m_blobchanges.last());
				windowname = QString("anno %1, saved, t %2 has changetype %3 pid %4 ").arg(m_filename).arg(m_blobchanges.last().oldid).arg(m_blobchanges.last().changetype).arg(m_blobchanges.last().newid);
				this->setWindowTitle(windowname);
			}
			else
			{
				qDebug()<< "blobchanges are empty";
			}
		break;

		case Qt::Key_Return:
			//numberKeyHandling(0,true);
			qDebug()<< "pushed return";
			numberKeyHandling(0,true);
			if (!m_blobchanges.empty())
			{
				qDebug()<< "blobchanges are not empty";
				saveBlobChangeDataToFileOnEnter(m_filenameBCD, m_blobchanges.last());
				windowname = QString("anno %1, saved, t %2 has %3 pid %4 ").arg(m_filename).arg(m_blobchanges.last().oldid).arg(m_blobchanges.last().changetype).arg(m_blobchanges.last().newid);
				this->setWindowTitle(windowname);
			}
			else
			{
				qDebug()<< "blobchanges are empty";
			}
		break;
	}

}

//todo test whether this actually works
void PlvMouseclick::numberKeyHandling(int nr, bool comma)
{
	QString windowname = "";
	//nrs can be higher than 10 so need to handle 2 and 3 digits as well
	if 	(blobtracksetting == Neutral)
	{
		qDebug() << "you pressed id or a number " << nr << "but you are in neutral state, choose first";
	}
	else if (!comma)
	{
		m_inputnr = m_inputnr*10+nr;
		if (blobtracksetting == Add)
		{
			if (m_firstid)
			{
				windowname = QString("anno %1, is in add state, changing track %2").arg(m_filename).arg(m_inputnr);
				this->setWindowTitle(windowname);		
			}
			else
			{
				if (m_blobchanges.size()>0)
				{
					windowname = QString("anno %1, is in add state, changing pid %2 to track %3").arg(m_filename).arg(m_inputnr).arg(m_blobchanges.last().oldid);
					this->setWindowTitle(windowname);		
				}
			}
		}
		else if (blobtracksetting == Remove)
		{
			if (m_firstid)
			{
				windowname = QString("anno %1, is in remove state, changing track %2").arg(m_filename).arg(m_inputnr);
				this->setWindowTitle(windowname);		
			}
			else
			{
				if (m_blobchanges.size()>0)
				{
					windowname = QString("anno %1, is in remove state, changing removal pid %2 to track %3").arg(m_filename).arg(m_inputnr).arg(m_blobchanges.last().oldid);
					this->setWindowTitle(windowname);		
				}
			}
		}
		else if (blobtracksetting == New)
		{
			if (m_firstid)
			{
				qDebug() << "this state in adding new should be unreachable as only the newid is interesting";	
			}
			else
			{
				if (m_blobchanges.size()>0)
				{
					windowname = QString("anno %1, is in new state, adding a pid %2").arg(m_filename).arg(m_inputnr);
					this->setWindowTitle(windowname);		
				}
			}
		}
		else if (blobtracksetting == Insert)
		{
			if (m_firstid)
			{
				if (m_blobchanges.size()>0)
				{
					windowname = QString("anno %1, is in inserting state, about to change track# %2").arg(m_filename).arg(m_inputnr);
					this->setWindowTitle(windowname);		
				}	
			}
			else
			{
				if (m_blobchanges.size()>0)
				{
					windowname = QString("anno %1, is in inserting state, adding a pid %2").arg(m_filename).arg(m_inputnr);
					this->setWindowTitle(windowname);		
				}
			}
		}//this->setWindowTitle("annotationwidget, saved last blobchange");
	}
	else if(comma)
	{
		qDebug()<< "comma";
		if (blobtracksetting == Add)
		{
			if (m_firstid)
			{
				m_firstid = false;
				m_secondid = true;
				m_blobchanges.last().oldid = m_inputnr;
				if (m_blobchanges.last().oldid == -1)
					windowname = QString("anno %1, add state, UNDO please hit escape you did not put in a track nr").arg(m_filename);
				else
					windowname = QString("anno %1, is in add state, change of track %2 and now to input pid").arg(m_filename).arg(m_blobchanges.last().oldid);
				this->setWindowTitle(windowname);	
			}
			else
			{
				m_firstid = true;
				m_blobchanges.last().newid = m_inputnr;
				if (m_blobchanges.last().newid == -1 || m_blobchanges.last().newid == -1)
				{
					windowname = QString("anno %1, add state, UNDO if you used comma, not correct pid or track").arg(m_filename);
				}
				else
					windowname = QString("anno %1, add state, add pid %2 to track %3").arg(m_filename).arg(m_blobchanges.last().newid).arg(m_blobchanges.last().oldid);
				this->setWindowTitle(windowname);	
				qDebug() << "just added" << m_blobchanges.last().newid << "to" << m_blobchanges.last().oldid;
			}
		}
		else if (blobtracksetting == Remove)
		{
			if (m_firstid)
			{
				m_firstid = false;
				m_secondid = true;
				m_blobchanges.last().oldid = m_inputnr;
				windowname = QString("anno %1, is in remove state, change of track %2 and now to input pid").arg(m_filename).arg(m_blobchanges.last().oldid);
				this->setWindowTitle(windowname);	
			}
			else
			{
				m_firstid = true;
				m_blobchanges.last().newid = m_inputnr;
				if (m_blobchanges.last().newid == -1 || m_blobchanges.last().newid == -1)
				{
					windowname = QString("anno %1, removal state, UNDO if you used comma, not correct pid or track").arg(m_filename);
				}
				else
					windowname = QString("anno %1, removal state, removed pid %2 of track %3").arg(m_filename).arg(m_blobchanges.last().newid).arg(m_blobchanges.last().oldid);
				this->setWindowTitle(windowname);	
				qDebug() << "just removed" << m_blobchanges.last().newid << "from" << m_blobchanges.last().oldid;
			}
		}
		else if (blobtracksetting == New)
		{
			if (m_firstid)
			{
				m_firstid = false;
				m_secondid = true;
				qDebug() << "this state of firstid in new blob and a comma is supposed to be unreachable, perhaps you entered a comma before, enter suffices";
				m_blobchanges.last().newid = m_inputnr;
				qDebug() << "creating" << m_blobchanges.last().newid << "" ;
			}
			else
			{
				m_firstid = true;
				m_blobchanges.last().newid = m_inputnr;
				if (m_blobchanges.last().newid != -1 || m_blobchanges.last().newid == -1)
				{
					windowname = QString("anno %1, new unseen track state, UNDO if you used comma, not correct pid or entered a track").arg(m_filename);
				}
				else
					windowname = QString("anno %1, new track state finished, created pid %2").arg(m_filename).arg(m_blobchanges.last().newid).arg(m_blobchanges.last().oldid);
				this->setWindowTitle(windowname);	
				qDebug() << "creating" << m_blobchanges.last().newid << "" ;
			}
		}
		else if (blobtracksetting == Insert)
		{
			if (m_firstid)
			{
				m_firstid = false;
				//to be sure no strange bugs occur
				m_secondid = true;
				m_blobchanges.last().oldid = m_inputnr;
				windowname = QString("anno %1, is in inserting state, change of track %2 and now to input pid").arg(m_filename).arg(m_blobchanges.last().oldid);
				this->setWindowTitle(windowname);	
			}
			else
			{
				m_firstid = true;
				m_blobchanges.last().newid = m_inputnr;
				if (m_blobchanges.last().oldid == -1 || m_blobchanges.last().newid == -1)
				{
					windowname = QString("anno %1, inserting state, UNDO somehow not correct pid or track").arg(m_filename);
				}
				else
					windowname = QString("anno %1, inserting state, removed pid %2 of track %3").arg(m_filename).arg(m_blobchanges.last().newid).arg(m_blobchanges.last().oldid);
				this->setWindowTitle(windowname);	
				qDebug() << "just added pid " << m_blobchanges.last().newid << " to and removed existing pids from " << m_blobchanges.last().oldid;
			}
		}
		//also put the old settings like merge and exchange in the bin

		m_inputnr = 0;
	}
}

void PlvMouseclick::setTheState(QString c)
{
	//if undo try to remove last item
	if(c == "U")
	{
		if (!m_blobchanges.isEmpty())
		{
			m_blobchanges.removeLast();
		}
	}
	else if(c == "N" )
	{
		//seems redundant
		BlobChangeData exchanging;
		exchanging.oldid = -1;
		exchanging.newid = -1;
		exchanging.cogs.x= 0;
		exchanging.cogs.y= 0;
		exchanging.changetype = c;
		m_blobchanges.push_back(exchanging);
	}
	else
	{
		BlobChangeData exchanging;
		exchanging.oldid = -1;
		exchanging.newid = -1;
		exchanging.cogs.x= 0;
		exchanging.cogs.y= 0;
		exchanging.changetype = c;
		m_blobchanges.push_back(exchanging);
	}
}


//TODO set this to plvmouseclick process
void PlvMouseclick::saveBlobChangeDataToFileOnEnter(QString filename, BlobChangeData bcd)
{
	//todo m_interesting
	//QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7").arg(m_framenr).arg(bcd.oldid).arg(bcd.newid).arg(bcd.cogs.x).arg(bcd.cogs.y).arg(false).arg(bcd.changetype);
	QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6").arg(m_framenr).arg(bcd.oldid).arg(bcd.newid).arg(bcd.cogs.x).arg(bcd.cogs.y).arg(bcd.changetype);
	QFile file(m_filenameBCD);
	bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
	Q_ASSERT(ret);
	QTextStream s(&file);
	for (int i = 0; i < blobTabString.size(); ++i)
		s << blobTabString.at(i);
	file.write("\n");	
	ret = file.flush();
	Q_ASSERT(ret);
	file.close();
	qDebug()<< "pushed out the m_filenamebcd/ changefile";
}


void PlvMouseclick::mouseReleaseEvent(QMouseEvent * event)
 {
	 QString windowname = "";
	 if (blobtracksetting != Neutral && blobtracksetting != Undo )
	 {
		 if (!m_blobchanges.isEmpty())
		 {

			m_blobchanges.last().cogs.x = event->pos().x();
			m_blobchanges.last().cogs.y = event->pos().y();
			windowname = QString("anno %1, clicked at x %2 and y %3 to improve track %4 or pid %5").arg(m_filename).arg(m_blobchanges.last().cogs.x).arg(m_blobchanges.last().cogs.y).arg(m_blobchanges.last().oldid).arg(m_blobchanges.last().newid);
			this->setWindowTitle(windowname);
			//m_blobchanges.replace(m_blobchanges.last(),exchanging);
		 }
	 }
 }


//
//QString PlvMouseclick::saveTrackToAnnotation(QString filename, BlobTrack t, cv::Point p, QString annotationstate, int newid)
//{
//	//prevent logging after closing the widget, if the widget is closed once it enters a error stage that will prevent blocking the process.
//	if (!m_error)
//	{
//	//TODO check when later read into matlab, the divided will end up using only the last blob's value, so compare d-state and processingserial?
//	//add processingserial only save the ones with a higher processing serial, i guess both will have the same processingserial?
//		//maybe better to save the values instead of calling them twice. although none of these values seem to require much proccesing power, they only return a value. 
//		//TODO also save track id, don't save direction and other vagues hit
//		//QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \t %8 \t %9 \t %10 \t %11 \t %12").arg(filename).arg(t.getId()).arg(newid).arg(p.x).arg(p.y).arg(t.getAveragePixel()).arg(t.getDirection()).arg(t.getVelocity2()).arg(t.getAge()).arg(m_skipprocessloop).arg(annotationstate).arg(this->getProcessingSerial());
//		
//		QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \t %8 \t %9").arg(filename).arg(t.getId()).arg(newid).arg(p.x).arg(p.y).arg(t.getAveragePixel()).arg(m_skipprocessloop).arg(annotationstate).arg(this->getProcessingSerial());
//		
//		for (int i=0; i<t.getPIDS().size(); i++)
//		{
//			QString toappend = QString("\t %1").arg(t.getPIDS().at(i));
//			blobTabString.append(toappend);
//		}
//
//		QFile file(m_filenameLog);
//		bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
//		Q_ASSERT(ret);
//		QTextStream s(&file);
//		for (int i = 0; i < blobTabString.size(); ++i)
//			s << blobTabString.at(i);
//		file.write("\n");	
//		ret = file.flush();
//		Q_ASSERT(ret);
//		file.close();
//		return blobTabString;
//	}
//	QString blobTabString2 = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \t %8 \t %9").arg(filename).arg(0).arg(0).arg(0).arg(0).arg(0).arg(0).arg(annotationstate).arg(this->getProcessingSerial());
//	return blobTabString2;
//}

////what state are we in?
//	//Two blob have switched ids, they have "exchanged ids"
//	//an exchange is basically a double assign
//	if (blobtracksetting == Exchange)
//	{
//		//to be sure no strange errors occur
//		if (!m_blobchanges.isEmpty())
//		{
//			if (m_firstid)
//			{
//				m_firstid = false;
//				m_blobchanges.last().oldid = m_inputnr;
//			}
//			else
//			{
//				//maybe dont reset here?
//				m_firstid = true;
//				m_blobchanges.last().newid = m_inputnr;
//				m_blobchanges.last().changetype = "A"; //somehow saved as " A"!
//				//allowed in case?
//				qDebug() << "just added switch of" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid;
//				//TODO IN THIS WAY COGGS ARE NOT TAKEN INTOACCOUNT!
//				//this wouldn't help either cv::Point tempcogs= m_blobchanges.last().cogs;
//				m_blobchanges.push_back(m_blobchanges.last());
//				int tempint = m_blobchanges.last().newid;
//				m_blobchanges.last().newid = m_blobchanges.last().oldid;
//				m_blobchanges.last().oldid = tempint;
//			}
//			qDebug() << "currently exchanging id" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid;
//		}
//	}
//	//A blob has the wrong ID, change it
//	else if (blobtracksetting == Assign)
//	{
//		if (m_firstid)
//		{
//			m_firstid = false;
//			m_blobchanges.last().oldid = m_inputnr;
//		}
//		else
//		{
//			//maybe dont reset here?
//			m_firstid = true;
//			m_blobchanges.last().newid = m_inputnr;
//			qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid;
//		}
//	}
//ONE blob with TWO ids, this one might shoud be usable again, to assign even more ids to one blob
	//else if (blobtracksetting == Merged)
	//{
	//	if (m_firstid)
	//	{
	//		m_firstid = false;
	//		m_blobchanges.last().oldid = m_inputnr;
	//	}
	//	else
	//	{
	//		//TODO checck does secondid identify third id indeed?
	//		if (m_secondid == true)
	//		{
	//			//keep one with old id, but it will have no newid?
	//			//todo why three line instead of 2? m_blobchanges.push_back(m_blobchanges.last()); 
	//			m_blobchanges.last().newid = m_inputnr;
	//			qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid << "debug asize before dupl" << m_blobchanges.size();
	//			m_secondid = false;
	//		}
	//		else
	//		{
	//			//todo duplicate only here?
	//			m_blobchanges.push_back(m_blobchanges.last());
	//			m_blobchanges.last().newid = m_inputnr;
	//			qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid << "debug asize after dupl" << m_blobchanges.size();
	//		
	//			//maybe dont reset here?
	//			m_firstid = true;
	//			m_secondid = true;
	//		}
	//	
	//	}
	//}
	////TWO blobs with 1 ID, is divided two blobs are recognised but both should have the same id. Thus select oldblob 1. select oldblob2, asssign the correct newid //codewise this seems complicated and incorrect, problem if no new is set!
	//else if (blobtracksetting == Divided)
	//{
	//	if (m_firstid)
	//	{
	//		m_firstid = false;
	//		m_blobchanges.last().oldid = m_inputnr;
	//	}
	//	else
	//	{
	//	
	//		if (m_secondid == true)
	//		{
	//			//keep one with old id
	//			m_blobchanges.last().newid = m_inputnr;
	//			m_secondid = false;
	//		}
	//		else
	//		{
	//			//m_blobchanges.push_back(m_blobchanges.last()); 
	//			int tempactualyoldid2 = m_blobchanges.last().newid;
	//			m_blobchanges.last().newid = m_inputnr;
	//			qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid;// << "debug asize before dupl" << m_blobchanges.size();

	//			//duplicate?
	//			m_blobchanges.push_back(m_blobchanges.last());
	//			m_blobchanges.last().oldid = tempactualyoldid2 ;
	//			qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid;// << "debug asize after dupl" << m_blobchanges.size();
	//		
	//			m_firstid = true;
	//			m_secondid = true;
	//		}
	//	
	//	}
	//}

	///*else if (blobtracksetting == Boring)
	//{
	//	qDebug() << "you choose to skip this frame so this is unreachable and if it is you should not type a number" << nr ;
	//}*/



////from key cases

	////	case Qt::Key_E:
	//		blobtracksetting = Exchange;
	//		m_firstid = true;
	//		//vs does not allow to create variables in a case statement
	//		setTheState("E");
	//		qDebug() << "Pressed E";
	//		this->setWindowTitle("annotationwidget, exchange/switch state");
	//	break;

	//	//case Qt::Key_A:
	//	//	blobtracksetting = Assign;
	//	//	m_firstid = true;
	//	//	//visual studio does not allow to create variables in a case statement
	//	//	setTheState('A');
	//	//	this->setWindowTitle("annotationwidget, assign state");
	//	//break;


	//	case Qt::Key_M:
	//		blobtracksetting = Merged;
	//		m_firstid = true;
	//		m_secondid = true;
	//		setTheState("M");
	//		this->setWindowTitle("annotationwidget, merged state");
	//	break;

	//	case Qt::Key_D:
	//		blobtracksetting = Divided;
	//		m_firstid = true;
	//		m_secondid = true;
	//		setTheState("D");
	//		this->setWindowTitle("annotationwidget, divided/split state");
	//	break;

	//	//not needed i think
	///*	case Qt::Key_B:
	//		blobtracksetting = Boring;
	//		setTheState('B');
	//		this->setWindowTitle("annotationwidget, boring state");
	//		m_dontskipframeitisinteresting = false;
	//	break;*/


//void PlvMouseclick::drawCenterOfGravity( cv::Mat& target, cv::Scalar color, cv::Point mousep ) 
//{
//	//draw a small cross
//	cv::Point cogtouse = mousep;
//	cv::Point tempcog = cogtouse;
//	cv::Point tempcog2 = cogtouse;
//	cv::Point tempcog3 = cogtouse;
//	cv::Point tempcog4 = cogtouse;
//	int crossdim = 5;
//	tempcog.x = cogtouse.x-crossdim;
//	tempcog.y = cogtouse.y-crossdim;
//	tempcog2.x = cogtouse.x+crossdim;
//	tempcog2.y = cogtouse.y+crossdim;
//	tempcog3.x = cogtouse.x-crossdim;
//	tempcog3.y = cogtouse.y+crossdim;
//	tempcog4.x = cogtouse.x+crossdim;
//	tempcog4.y = cogtouse.y-crossdim;
//
//	cv::line( target, tempcog, tempcog2, color );
//	cv::line( target, tempcog3, tempcog4, color );
//	
//	int x,y,w,h;
//
//    w = target.cols();// * m_zoomFactor;
//    h = target.rows();//* m_zoomFactor;
//    x = 0;
//    y = 0;
//	QRectF target(x,y,w,h);
//	QPixmap pixmap(w,h);
//    QPainter p(&pixmap);
//	QImage qtimage = CvMatDataToQImage(image);
//    p.drawImage(target, qtimage);
//}
