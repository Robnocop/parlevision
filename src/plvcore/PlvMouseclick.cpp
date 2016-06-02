#include "PlvMouseclick.h"

#include <stdlib.h>
#include <limits>

#include <plvcore/CvMatData.h>
//for mutexlocker:
//#include <plvcore/PipelineProcessor.h>
//#include <plvcore/PipelineElement.h>

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

using namespace plv;


PlvMouseclick::PlvMouseclick() :
		m_zoomFactor( 1.0 ),
        m_aspectRatio( 1.0 ), //1.0
        m_zoomToFit( false ),
		m_lastWidth(640),
		m_lastHeight(480)
{
	//init shizzle
	//m_filenameLog = "annotationlog.txt";
	m_popUpExists = true;
	m_layoutqhb = new QHBoxLayout;
	m_point_wanted.x = 0;
	m_point_wanted.y = 0;
	m_point_assumed.x = 0;
	m_point_assumed.y = 0;
	m_minwantedy = 640;
	m_maxwantedy = 740;
	m_maxassumedposy =840;
	m_mouseclickedperson = false;
	m_mouseclickedball = false;
	//counter = 0;
}

PlvMouseclick::~PlvMouseclick()
{
	
}

void PlvMouseclick::deinit()
{
	//deinit shizzle
	qDebug() << "plvmouseclick has been deint";
	setPopUpExists(false);
	this->hide();
	this->close();
}

//not used at this time
void PlvMouseclick::processInit()
{
	
}

void PlvMouseclick::init()
{
	//create an empty frame to paint from
	m_lastpicture.create(640, 480, CV_8U, 3);
	//probably needed for reinit of a popup
	setZoomFactor(1.0);
    setZoomToFit(false);
	setLastWidth(640);
	setLastHeight(480);
	setAspectRatio(getLastWidth()  / getLastHeight());
	//counter = 0;
	setMouseClickedPerson(false); 
	setMouseClickedBall(false);
}

//functions:
void PlvMouseclick::closeDown()
{
	setPopUpExists(false);
	setMouseClickedPerson(false);
	setMouseClickedBall(false);
	this->close();
}

void PlvMouseclick::closeEvent(QCloseEvent *event)
{
	setPopUpExists(false);
	qDebug() << "windows closed"; 
	//TODO i dont think normal throw is allowed however, plvseterror will also not be allowed
	//throw std::runtime_error("Cannot continue annotation video playback");
	//plv::PipelineElement::setError(plv::PlvErrorType, tr("Cannot continue annotation video playback"));
	this->close();
}

void PlvMouseclick::paintEvent(QPaintEvent * event)
{

	//TODO check m_lastpicture to prevent crash!!
	//qDebug() << "enter mutex paint" << counter;
	manualmutex.lock();
		//qDebug() << "enter paint event" << counter;
		CvMatData copylastpicture = m_lastpicture;
	manualmutex.unlock();

	if (copylastpicture.width()<320)
	{
		//skip drawing
	}
	else
	{
		bool sizechanged  = false;
		int width = copylastpicture.width();
		int height = copylastpicture.height();

		//resize stuff
		int x,y,w,h;

		if (getLastWidth() != width  || getLastHeight() != height)
		{
			sizechanged = true;
			if (width>0)
				setLastWidth(width);
			if (height>0)
				setLastHeight(height);
		
			if (height>0 && width>0)
			{ 
				setAspectRatio(width / (float)height);
				if( getZoomToFit() )
				{
					computeZoomFactorToFitImage(getLastWidth(), getLastHeight());
				}
				else
				{
					setZoomFactor(1.0);
				}
			}
			else
			{
				setZoomFactor(1.0);
			}

			//newstyle: only if the size is changed
			//oldstyle
			//this->resize(image.width(), image.height());
			this->resize( sizeHint() );
		}

		w = width* getZoomFactor();
		h = height* getZoomFactor();
		x = 0;
		y = 0;
	
		//qt doc: setUpdatesEnabled() is normally used to disable updates for a short period of time, for instance to avoid screen flicker during large changes. In Qt, widgets normally do not generate screen flicker, but on X11 the server might erase regions on the screen when widgets get hidden before they can be replaced by other widgets. Disabling updates solves this.
		//setUpdatesEnabled(false);

		//rect includes a resize method if sizes differ rectf doesnt
		QRect target(x,y,w,h);
		QPixmap pixmap(w,h);
		QPainter p(&pixmap);
		QImage qtimage = CvMatDataToQImage(copylastpicture);
		//calling draw image with rect includes a resize method if sizes differs 
		//calling draw with rectf doesnt
		//somehow does not work

		p.drawImage(target, qtimage);
		manualmutex.lock();
			m_imlab1.setPixmap(pixmap); 
			//adjusts the canvas which is drawn on
			if (sizechanged)
				m_imlab1.adjustSize();
		manualmutex.unlock();
		//setUpdatesEnabled(true);
		//qDebug() << "exit paint event" <<counter;
	}
}

//correct number is painted
//what if we would use an actual picture
bool PlvMouseclick::toPaint(CvMatData image)
{
	//qDebug() << "enter to paint mutex" << counter;
	manualmutex.lock();
		//place the latest received image into this global variable, the mutex should prevent it from being accesed while read at the actual paint event
		m_lastpicture = image;
		//setUpdatesEnabled(true);
	manualmutex.unlock();
	update();
	
	//qDebug() << "exit to paint" <<counter;
	return true;
}

void PlvMouseclick::computeZoomFactorToFitImage(float width, float height)
{
	qDebug() << "computezoomfactor";
	float expthreshold = 0.1f;
    QRect rect = this->rect();
    float ratio = rect.width() / (float)rect.height();
    if( ratio > getAspectRatio()) 
    {
		if (getZoomFactor() > (rect.height() / (float)height +expthreshold) || getZoomFactor() < (rect.height() / (float)height -expthreshold))
		{
			setZoomFactor(rect.height() / (float)height);
		}
    }
    else if (ratio < getAspectRatio() )
    {
		//prevent automatic expanding with some threshold 
		if (getZoomFactor()> (rect.width() / (float)width + expthreshold) || getZoomFactor() < (rect.width() / (float)width -expthreshold))
		{
			setZoomFactor(rect.width() / (float)width);
		}
    }
	else
	{
		
	}
	
}


//quite an expensive function, the pointer is always to a local copy of an image, thus no thread safety issues should occur
QImage PlvMouseclick::CvMatDataToQImage(CvMatData& imageMat)
{
	cv::Mat& input = imageMat;
	QImage image(imageMat.width(), imageMat.height(), QImage::Format_RGB32);

    uchar* pBits = image.bits();
    int nBytesPerLine = image.bytesPerLine();

    for (int n = 0; n < imageMat.height(); n++)
    {
        for (int m = 0; m < imageMat.width(); m++)
        {
			//QRgb v = input.at<unsigned short>(n,m);
			//catch the possibility of a greyscale picture
			if (input.channels() ==3)
			{
				cv::Vec3b bgrPixel = input.at<cv::Vec3b>(n, m);
				QRgb value = qRgb((uchar)bgrPixel.val[2], (uchar)bgrPixel.val[1], (uchar)bgrPixel.val[0]);

				//greyscale
				//unsigned int v = input.at<unsigned short>(n,m);
				//QRgb value = qRgb((uchar)v, (uchar)v, (uchar)v);
			
				uchar* scanLine = pBits + n * nBytesPerLine;
				((uint*)scanLine)[m] = value;
			}
        }
    }

    return image;
}



void PlvMouseclick::createPopUp()
{
	if(!getPopUpExists())
	{	
		//need to lock with respect to m_layout and m_imlab which are also set in paint event
		//TODO check for deadlocks
		manualmutex.lock();
			//ALIGN:
			m_imlab1.setAlignment(Qt::AlignCenter);
			// create horizontal layout
			//QHBoxLayout* layoutqhb =new QHBoxLayout;
			// and add label to it
			//layoutqhb->addWidget(&m_imlab1);
			m_layoutqhb->addWidget(&m_imlab1);
		
			// set layout to widget
			//this->setLayout(layoutqhb);
			this->setLayout(m_layoutqhb);
		manualmutex.unlock();

		this->resize(640, 480);
	
		this->setWindowTitle("parlevision widget");
		this->show();
		//apparently doesnt make sense: m_popupWidget->setWindowTitle("annotationwidget");
		setPopUpExists(true);
	}
}


//SEMI EVENT INHERITED but without const
//in const we cant call the functions is const required?
//QSize PlvMouseclick::sizeHint() const
QSize PlvMouseclick::sizeHint()
{
	qDebug() << "sizeHint" ;
	QSize newSize;
	int w = getLastWidth();
	int h = getLastHeight();
	

	if( getLastWidth()>0 && getLastHeight()>0 )
    {
        QSize hint = newSize * getZoomFactor();
		newSize.setWidth(w);
		newSize.setHeight(h);    
        return hint;
    }
	
	newSize.setWidth(640);
	newSize.setHeight(480);    
    
	//BUGFIX ATTEMPT
	// return minimumSize();
	QSize hintEmpty = newSize * getZoomFactor();
  	return hintEmpty;
}


//EVENTS
//void PlvMouseclick::keyPressEvent(QKeyEvent * event)
//{
//	//qDebug() << "key pressed" << event->key();
//}
//

////http://harmattan-dev.nokia.com/docs/library/html/qt4/qt.html#Key-enum
//void PlvMouseclick::keyReleaseEvent(QKeyEvent * event)
//{
//	QString windowname = "";
//	QString changetypechar = " ";
//	//qDebug() << "key released" << event->key();
//	switch(event->key())
//	{
//		
//	}
//
//}


void PlvMouseclick::mouseReleaseEvent(QMouseEvent * event)
 {
	 qDebug() << "mouse release event";
	QString windowname = "";
	//make global values
	

	float pointx = event->pos().x();
	float pointy = event->pos().y();
	windowname = QString("mouse clicked at x %1 and y %2 ").arg(pointx).arg(pointy);
	if (pointy>getMinWantedY() && pointy<getMaxWantedY())
	{	
		windowname = QString("wanted ball pos, clicked at x %1 and y %2 ").arg(pointx).arg(pointy);
		setPointWanted(cv::Point((int)pointx,(int)pointy));
		setMouseClickedPerson(true);
	}
	else if (pointy>getMaxWantedY() && pointy<getMaxAssumedPosY())
	{
		windowname = QString("set assumed ball pos, clicked at x %1 and y %2 ").arg(pointx).arg(pointy);
		setPointAssumed(cv::Point((int)pointx,(int)pointy));
		setMouseClickedBall(true);
	}
		
	this->setWindowTitle(windowname);
 }



void PlvMouseclick::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
    //qDebug() << "resizeevent" ;
    if( getZoomToFit() )
        computeZoomFactorToFitImage(getLastWidth(),getLastHeight());
    else
        setZoomFactor(1.0);
}

///////GET SET VARIABLES/////////////
//set variables:

//noclue inheritance ?
void PlvMouseclick::setZoomFactor( float zf )
{
	QMutexLocker lock(&m_robMutex);
    m_zoomFactor = zf;
    //updateGeometry();
    update();
    adjustSize();
}

//public function to set zoomfactor on and off
void PlvMouseclick::setZoomToFit( bool zoomToFit )
{
	//in this method we also set other variables so as I am uncertain on the response of QT with respect to this possible deadlock we use a second propertymutex.
	QMutexLocker lock(&m_robMutex2);
    m_zoomToFit = zoomToFit;
    if( zoomToFit)
    {
		//todo check if the mutex in a mutex will not result in issues!
        computeZoomFactorToFitImage(getLastWidth(), getLastHeight());
    }
    else
    {
        m_zoomFactor = 1.0;
    }

    // resize and update widget
    this->resize( sizeHint() );
    update();
}


void PlvMouseclick::setAspectRatio(float f)
{
	QMutexLocker lock(&m_robMutex);
	m_aspectRatio = f;
}


void PlvMouseclick::setLastWidth(int i)
{
	QMutexLocker lock(&m_robMutex);
	m_lastWidth = i;
}

void PlvMouseclick::setLastHeight(int i)
{
	QMutexLocker lock(&m_robMutex);
	m_lastHeight = i;
}


void PlvMouseclick::setMouseClickedBall(bool b)
{
	QMutexLocker lock(&m_robMutex);
	m_mouseclickedball = b;	
}

void PlvMouseclick::setMouseClickedPerson(bool b)
{
	QMutexLocker lock(&m_robMutex);
	m_mouseclickedperson = b;	
}

void PlvMouseclick::setMaxAssumedPosY(int i)
{
	QMutexLocker lock(&m_robMutex);
	m_maxassumedposy = i;	
}

void PlvMouseclick::setMaxWantedY(int i)
{
	QMutexLocker lock(&m_robMutex);
	m_maxwantedy = i;	
}

void PlvMouseclick::setMinWantedY(int i)
{
	QMutexLocker lock(&m_robMutex);
	m_minwantedy = i;	
}

//
void PlvMouseclick::setPointWanted(cv::Point p)
{
	QMutexLocker lock(&m_robMutex);
	m_point_wanted = p;	
}
//
void PlvMouseclick::setPointAssumed(cv::Point p)
{
	QMutexLocker lock(&m_robMutex);
	m_point_assumed = p;	
}
void PlvMouseclick::setPopUpExists(bool b)
{
	QMutexLocker lock(&m_robMutex);
	m_popUpExists = b;
}

//getvariables


int PlvMouseclick::getLastWidth()
{
	QMutexLocker lock(&m_robMutex);
	return m_lastWidth;
}

int PlvMouseclick::getLastHeight()
{
	QMutexLocker lock(&m_robMutex);
	return m_lastHeight;
}

bool PlvMouseclick::getZoomToFit()
{
	QMutexLocker lock(&m_robMutex);
	return m_zoomToFit;
} 

float PlvMouseclick::getZoomFactor()
{
	QMutexLocker lock(&m_robMutex);
	return m_zoomFactor;
} 

bool PlvMouseclick::getMouseClickedBall()
{
	QMutexLocker lock(&m_robMutex);
	return m_mouseclickedball;	
}

bool PlvMouseclick::getMouseClickedPerson()
{
	QMutexLocker lock(&m_robMutex);
	return m_mouseclickedperson;
}

int PlvMouseclick::getMaxAssumedPosY()
{
	QMutexLocker lock(&m_robMutex);
	return m_maxassumedposy;
}

float PlvMouseclick::getAspectRatio()
{
	QMutexLocker lock(&m_robMutex);
	return m_aspectRatio;
}

int PlvMouseclick::getMaxWantedY()
{
	QMutexLocker lock(&m_robMutex);
	return m_maxwantedy;
}

int PlvMouseclick::getMinWantedY()
{
	QMutexLocker lock(&m_robMutex);
	return m_minwantedy;
}

//person clicked pos
cv::Point PlvMouseclick::getPointWanted()
{
	QMutexLocker lock(&m_robMutex);
	return m_point_wanted;	
}

//ball clicked pos
cv::Point PlvMouseclick::getPointAssumed()
{
	QMutexLocker lock(&m_robMutex);
	return m_point_assumed;	
}

bool PlvMouseclick::getPopUpExists()
{
	QMutexLocker lock(&m_robMutex);
	return m_popUpExists;
}