/*
 *   Copyright (c) 2007 John Weaver
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

//to include in class header file to include this popup
//#include <plvcore/PlvMouseclick.h>
//using namespace plv;
//and as private variables a name for the popup and a boolean to keep track if it exists or not.
//plv::PlvMouseclick m_plvannotationwidget;
//bool m_popUpExists;

//in init of cpp file
////popup creation.
//m_plvannotationwidget.init();	
//m_plvannotationwidget.createPopUp();	
//m_popUpExists = m_plvannotationwidget.m_popUpExists; 

//in process of cpp file
//m_plvannotationwidget.processInit(0);
//evt m_plvannotationwidget.setWindowTitle(windowname);
//m_plvannotationwidget.toPaint(out);

//in deinit of cpp file
////popup creation
//m_plvannotationwidget.deinit();

//preferably link the createpopup and booleans to a gui boolean to recreate the popup if neccesary.

#include <QWidget>

#include <plvcore/PipelineProcessor.h>
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>

//#include "BlobChangeData.h"
using namespace plv;
//using namespace plvblobtracker;

#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QtGui>
#include <QScopedPointer>

#include <QMutexLocker>

#if !defined(_PLVMOUSECLICK_H_)
#define _PLVMOUSECLICK_H_

namespace plv
{
	//enum PlvFrameSetting {
	//		NotSet,
	//		GotoNextFrame,  /** I skip this frame I have been here or the track data was fine, so proceed */
	//		GotoPreviousFrame, /** Something went wrong before*/
	//		GotoCurrentFrame   /** I need to process the same frame, unknown yet when this is exactly needed */
	//};

	//enum PlvBlobSetting {
	//		Neutral, //N: everything is fine
	//		Add,
	//		Remove,
	//		Undo,
	//		Insert,
	//		//adding a pid without a recognised blob
	//		New
	//		//Escape 
	//		//TODO deal with problem: only at the frame a blob is split it will result in an error! Later on it will keep tracking it seemingly "correctly". However every frame it needs to be manually annotated. 
	//		//---------OLD-------------------------
	//		//	Exchange,//E: two tracks have been switched, different than assigning as otherwise order would matter (e.g. 3=2 and then 2=3 would assign 3=3 instead of 3=2,2=3
	//		////E, id1, id2 --> swap id1 and id2
	//		//Assign,  //A: a track needs another id name
	//		////A, oldid, newid1
	//		//Merged, //M: one track has two blobs we need to duplicate the track and assign two ids and perhaps different cogs to it.
	//		////M, oldid, newid#1,  newid#2,
	//		//Divided,    //D: blob has been split due to erode dilate or whatever, two tracks need to have same ID and same cog. 
	//		////D, id1, id2, newid (id1 && id2 == newid)
	//		//Boring, //B this is a non-interesting fram to annotate, e.g. recess or empty frame
	//		////Prevent Switch and Split to have the same letter
	//};
	
	
	//LESSON OF THE DAY THE EXPORT IS NEEDED TO LOAD IT ELSEWHERE OUTSIDE THIS SUB-PROJECT
	class PLVCORE_EXPORT PlvMouseclick : public QWidget
	{
		//Q_OBJECT
		public:
			PlvMouseclick(
				//constants if needed e.g. int birthThreshold=3,
				); 
			virtual ~PlvMouseclick();

			virtual void paintEvent(QPaintEvent * event);
			void init();
			void deinit();
			
						
			QImage CvMatDataToQImage(plv::CvMatData& input);
			//PlvFrameSetting framestate;
			//PlvBlobSetting blobtracksetting;
			
			//functions:
			void closeEvent(QCloseEvent *event);
			bool toPaint(CvMatData image);
			//void drawCenterOfGravity( cv::Mat& target, cv::Scalar color, cv::Point mousep);
			void createPopUp();
			void processInit();
			void closeDown();
			//QList<plvblobtracker::BlobChangeData> m_blobchanges;

			//from imagewidget.h
			/*explicit ImageWidget( QWidget *parent = 0 );
			void setImage( const QImage &image );
			QImage getImage();
			float getZoomFactor();
			void setZoomFactor( float zoomFactor );
			void setZoomToFit( bool zoomToFit );*/
			
			//these public variables should be mutexlock protected, as (two) different threads can call them
			float getZoomFactor(); 
			bool getZoomToFit();
			void setZoomFactor( float zoomFactor );
			void setZoomToFit( bool zoomToFit );

			void setMinWantedY(int i);
			void setMaxWantedY(int i);
			void setMaxAssumedPosY(int i);
			void setMouseClickedPerson(bool b);
			void setMouseClickedBall(bool b);
			void setPopUpExists(bool b);
			void setPointWanted(cv::Point p);
			void setPointAssumed(cv::Point p);

			void setLastWidth(int i );
			void setLastHeight(int i);
			void setAspectRatio(float f);

			int getMinWantedY();
			int getMaxWantedY();
			int getMaxAssumedPosY();
			bool getMouseClickedPerson();
			bool getMouseClickedBall();
			bool getPopUpExists();
			cv::Point getPointWanted();
			cv::Point getPointAssumed();

			int getLastWidth();
			int getLastHeight();
			float getAspectRatio();
	
	private:
			int m_minwantedy;
			int m_maxwantedy;
			int m_maxassumedposy;
			bool m_mouseclickedperson;
			bool m_mouseclickedball;
			//global stuff needed by trackannotation
			bool m_popUpExists;
			bool m_error;
			//perhaps we shouldn't use these to set point of mouseclick
			cv::Point m_point_wanted;
			cv::Point m_point_assumed;			

		protected:
			//resize
			float  m_zoomFactor;
			float  m_aspectRatio;
			bool   m_zoomToFit;
			bool m_updating;
			void computeZoomFactorToFitImage(float w, float h);

			int m_lastWidth;
			int m_lastHeight;
			
			void resizeEvent(QResizeEvent * event);
			/** Iherited from QWidget. Returns our preferred size */
			//QSize sizeHint() const;
			QSize sizeHint();

			//TODO check aint I creating an empty ref?
			QHBoxLayout* m_layoutqhb;

			//gui stuff
			void mouseReleaseEvent(QMouseEvent *event);
			//void m
			//keyboard events instead of shortcuts
			//virtual void keyPressEvent(QKeyEvent * event);
			//virtual void keyReleaseEvent(QKeyEvent * event);
			

			//from imagewidget.h in order to resize:
			//float  m_zoomFactor;
			//float  m_aspectRatio;
			//bool   m_zoomToFit;

			//void resizeEvent(QResizeEvent * event);

			///** Custom paint method inherited from QWidget */
			//void paintEvent( QPaintEvent * );

			///** Iherited from QWidget. Returns our preferred size */
			//QSize sizeHint() const;

			//void computeZoomFactorToFitImage();
			
			
		private:
			QMutex mutexunit;
			
			//QMutexLocker locker;
			mutable QMutex m_robMutex;
			mutable QMutex m_robMutex2;
			//QMutexLocker lock( &m_pleMutex );
			//QMutexLocker mutex;

			QWidget* m_popupWidget;
			QLabel m_imlab1;
			CvMatData m_lastpicture;

			int m_inputnr;
			//int counter;

			QMutex manualmutex;
	};
}


#endif /* !defined(_MUNKRES_H_) */