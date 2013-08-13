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

#include <QWidget>

#include <plvcore/CvMatData.h>
#include "BlobChangeData.h"
using namespace plv;
//using namespace plvblobtracker;

#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QtGui>
#include <QScopedPointer>

#if !defined(_PLVMOUSECLICK_H_)
#define _PLVMOUSECLICK_H_

namespace plvblobtracker
{
	enum PlvFrameSetting {
			NotSet,
			GotoNextFrame,  /** I skip this frame I have been here or the track data was fine, so proceed */
			GotoPreviousFrame, /** Something went wrong before*/
			GotoCurrentFrame   /** I need to process the same frame, unknown yet when this is exactly needed */
	};

	enum PlvBlobSetting {
			Neutral, //N: everything is fine
			Exchange,//E: two tracks have been switched, different than assigning as otherwise order would matter (e.g. 3=2 and then 2=3 would assign 3=3 instead of 3=2,2=3
			//E, id1, id2 --> swap id1 and id2
			Assign,  //A: a track needs another id name
			//A, oldid, newid1
			Merged, //M: one track has two blobs we need to duplicate the track and assign two ids and perhaps different cogs to it.
			//M, oldid, newid#1,  newid#2,
			Divided,    //D: blob has been split due to erode dilate or whatever, two tracks need to have same ID and same cog. 
			//D, id1, id2, newid (id1 && id2 == newid)
			Boring //B this is a non-interesting fram to annotate, e.g. recess or empty frame
			//Prevent Switch and Split to have the same letter
			//TODO deal with problem: only at the frame a blob is split it will result in an error! Later on it will keep tracking it seemingly "correctly". However every frame it needs to be manually annotated. 
	};
	
	

	class PlvMouseclick : public QWidget
	{
		//Q_OBJECT
		public:
			PlvMouseclick(
				//constants if needed e.g. int birthThreshold=3,
				); 
			virtual ~PlvMouseclick();
			void setNext();
			void init();
			void deinit();
			
			//global stuff needed by trackannotation
			bool m_popUpExists;
			bool m_mouseclick;
			bool m_error;
			bool m_dontskipframeitisinteresting;
			
			//temp bool to check whether right bottom was clicked to alter a frame;
			bool m_rightbottom;

			QImage CvMatDataToQImage(plv::CvMatData& input);
			PlvFrameSetting framestate;
			PlvBlobSetting blobtracksetting;
			
			//functions:
			void closeEvent(QCloseEvent *event);
			void testFunction();
			bool toPaint(CvMatData& image);
			void createPopUp();
			void processInit();
			void setTheState(char c);
			
			QList<plvblobtracker::BlobChangeData> m_blobchanges;

			void numberKeyHandling(int i);

		protected:
			void mouseReleaseEvent(QMouseEvent *event);
			//keyboard events instead of shortcuts used for buttons?
			virtual void keyPressEvent(QKeyEvent * event);
			virtual void keyReleaseEvent(QKeyEvent * event);

		private:
			QWidget* m_popupWidget;
			QLabel m_imlab1;
			bool donotcontinue;
			QString m_filename;
			bool m_firstid;
			bool m_secondid;

	};

	
}

#endif /* !defined(_MUNKRES_H_) */
