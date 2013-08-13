#include "PlvMouseclick.h"
#include <stdlib.h>
#include <limits>

#include <plvcore/CvMatData.h>
using namespace plv;
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

using namespace plvblobtracker;

PlvMouseclick::PlvMouseclick()
{
	//init shizzle
	m_mouseclick = false;
	m_popUpExists = false;
	m_rightbottom = false;
	m_error = false;
	framestate = NotSet;
	blobtracksetting = Neutral;
	donotcontinue = false;
	m_dontskipframeitisinteresting = true;
	m_firstid =true;
	m_secondid = false;

	//add shortcuts! 
	//no dont use keyevents
	//QShortcut* nextShortcut = new QShortcut(QKeySequence(tr("Ctrl+l")),this);
    //connect(nextShortcut, SIGNAL(activated()), this, SLOT(setNext()));
	//new QShortcut(Qt::Key_Left, this, SLOT(setNext()));
	
}

PlvMouseclick::~PlvMouseclick()
{
	m_mouseclick = false;
	m_error =true;
	m_dontskipframeitisinteresting = true;
	blobtracksetting = Neutral;
	m_firstid = true;
	m_secondid = false;
	//deinit shizzle
}

void PlvMouseclick::deinit()
{
	//deinit shizzle
	m_mouseclick = false;
	m_error=true;
	m_popUpExists = false;
	m_dontskipframeitisinteresting = true;
	framestate = GotoNextFrame;
	blobtracksetting = Neutral;
	//this->deleteLater();
	//this->setAttribute(Qt::WA_DeleteOnClose, true);
	this->hide();
	this->close();
}

void PlvMouseclick::processInit()
{
	framestate = NotSet;
	blobtracksetting = Neutral;
	m_dontskipframeitisinteresting = true;
	m_firstid = true;
	m_secondid = false;
}

void PlvMouseclick::init()
{
	//this->close();
	m_error = false;
	m_mouseclick = false;
	m_popUpExists = false;
	m_rightbottom = false;
	m_dontskipframeitisinteresting = true;
	framestate = NotSet;
	blobtracksetting = Neutral;
	donotcontinue = false;
	m_firstid = true;
	m_secondid = false;
	
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

void PlvMouseclick::closeEvent(QCloseEvent *event)
{
	
	m_rightbottom = true;
	m_popUpExists = false;
	qDebug() << "windows closed"; 
	m_error = true;
	throw std::runtime_error("Cannot continue annotation video playback");
	this->close();
}

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
		case Qt::Key_N:
			blobtracksetting = Neutral;
			if (!m_blobchanges.isEmpty())
			{
				qDebug() << "neutral: last-changes" << m_blobchanges.last().oldid << "new" << m_blobchanges.last().newid << "last cog:" << m_blobchanges.last().cogs.x << "," << m_blobchanges.last().cogs.y;
				//m_blobchanges.replace(m_blobchanges.last(),exchanging);
			}		
			
			qDebug() << "Pressed N";
			this->setWindowTitle("annotationwidget, neutral state");
		break;

		case Qt::Key_E:
			blobtracksetting = Exchange;
			m_firstid = true;
			//vs does not allow to create variables in a case statement
			setTheState('E');
			qDebug() << "Pressed E";
			this->setWindowTitle("annotationwidget, exchange/switch state");
		break;

		case Qt::Key_A:
			blobtracksetting = Assign;
			m_firstid = true;
			//vs does not allow to create variables in a case statement
			setTheState('A');
			this->setWindowTitle("annotationwidget, assign state");
		break;

		case Qt::Key_M:
			blobtracksetting = Merged;
			m_firstid = true;
			m_secondid = false;
			setTheState('M');
			this->setWindowTitle("annotationwidget, merged state");
		break;

		case Qt::Key_D:
			blobtracksetting = Divided;
			setTheState('D');
			this->setWindowTitle("annotationwidget, divided/split state");
		break;

		case Qt::Key_B:
			blobtracksetting = Boring;
			setTheState('B');
			this->setWindowTitle("annotationwidget, boring state");
			m_dontskipframeitisinteresting = false;
		break;

		//ID stuff:
		case Qt::Key_0:
			//react on number according to current state
			numberKeyHandling(0);
		break;

		case Qt::Key_1:
			numberKeyHandling(1);	
		break;
		
		case Qt::Key_2:
			numberKeyHandling(2);
		break;

		case Qt::Key_3:
			numberKeyHandling(3);
		break;
		
		case Qt::Key_4:
			numberKeyHandling(4);
		break;

		case Qt::Key_5:
			numberKeyHandling(5);
		break;

		case Qt::Key_6:
			numberKeyHandling(6);
		break;

		case Qt::Key_7:
			numberKeyHandling(7);
		break;

		case Qt::Key_8:
			numberKeyHandling(8);
		break;

		case Qt::Key_9:
			numberKeyHandling(9);
		break;


	}

}

void PlvMouseclick::numberKeyHandling(int nr)
{
	//what state are we in?
	if (blobtracksetting == Exchange)
	{
		//to be sure no strange errors occur
		if (!m_blobchanges.isEmpty())
		{
			if (m_firstid)
			{
				m_firstid = false;
				m_blobchanges.last().oldid = nr;
			}
			else
			{
				//maybe dont reset here?
				m_firstid = true;
				m_blobchanges.last().newid = nr;
				//allowed in case?
				qDebug() << "just added switch of" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid;
				//IN THIS WAY COGGS ARE NOT TAKEN INTOACCOUNT!
				m_blobchanges.push_back(m_blobchanges.last());
				int tempint = m_blobchanges.last().newid;
				m_blobchanges.last().newid = m_blobchanges.last().oldid;
				m_blobchanges.last().oldid = tempint;
			}
			qDebug() << "currently exchanging id" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid;
		}
	}
	else if (blobtracksetting == Assign)
	{
		if (m_firstid)
		{
			m_firstid = false;
			m_blobchanges.last().oldid = nr;
		}
		else
		{
			//maybe dont reset here?
			m_firstid = true;
			m_blobchanges.last().newid = nr;
			qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid;
		}
	}
	else if (blobtracksetting == Merged)
	{
		if (m_firstid)
		{
			m_firstid = false;
			m_blobchanges.last().oldid = nr;
		}
		else
		{
			

			if (!m_secondid == true)
			{
				//keep one with old id
				m_blobchanges.push_back(m_blobchanges.last()); 
				m_blobchanges.last().newid = nr;
				qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid << "debug asize before dupl" << m_blobchanges.size();
				m_secondid = true;
			}
			else
			{
				//duplicate?
				m_blobchanges.push_back(m_blobchanges.last());
				m_blobchanges.last().newid = nr;
				qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid << "debug asize after dupl" << m_blobchanges.size();
				m_secondid = false;
				//maybe dont reset here?
				m_firstid = true;
			}
			
		}
	}
	else if (blobtracksetting == Divided)
	{
		if (m_firstid)
		{
			m_firstid = false;
			m_blobchanges.last().oldid = nr;
		}
		else
		{
			
			if (!m_secondid == true)
			{
				//keep one with old id
				m_blobchanges.last().newid = nr;
				m_secondid = true;
				
			}
			else
			{
				//m_blobchanges.push_back(m_blobchanges.last()); 
				m_blobchanges.last().newid = nr;
				qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid << "debug asize before dupl" << m_blobchanges.size();

				//duplicate?
				m_blobchanges.push_back(m_blobchanges.last());
				m_blobchanges.last().newid = nr;
				qDebug() << "just assigned" << m_blobchanges.last().oldid << "to"  << m_blobchanges.last().newid << "debug asize after dupl" << m_blobchanges.size();
				m_secondid = false;
				m_firstid = true;
			}
			
		}
	}
	else if (blobtracksetting == Boring)
	{

	}
	else if (blobtracksetting == Neutral)
	{
		qDebug() << "you pressed id" << nr << "but you are in neutral state, choose first";
	}
}

void PlvMouseclick::setTheState(char c)
{
	BlobChangeData exchanging;
	exchanging.oldid = 10;
	exchanging.newid = 10;
	exchanging.cogs.x= 0;
	exchanging.cogs.y = 0;
	exchanging.changetype = c;
	m_blobchanges.push_back(exchanging);
}


//
void PlvMouseclick::mouseReleaseEvent(QMouseEvent * event)
 {
	 if (blobtracksetting != Neutral && blobtracksetting != Exchange)
	 {
		 if (!m_blobchanges.isEmpty())
		 {
			m_blobchanges.last().cogs.x = event->pos().x();
			m_blobchanges.last().cogs.y = event->pos().y();
			//m_blobchanges.replace(m_blobchanges.last(),exchanging);
		 }
	 }

	
 }
