/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * All rights reserved.
  *
  * This file is part of the plvopencv module of ParleVision.
  *
  * ParleVision is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * ParleVision is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * A copy of the GNU General Public License can be found in the root
  * of this software package directory in the file LICENSE.LGPL.
  * If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef CamShift_H
#define CamShift_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/Enum.h>
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

namespace plvopencv
{
    class CamShift: public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( CamShift )
        Q_CLASSINFO("author", "Robby van Delden")
        Q_CLASSINFO("name", "CamShift")
        Q_CLASSINFO("description", "OpenCV CamShift algorithm implementation tracking based on hue and set trackwindow");

		//GUI to set min number of points and to reset the tracked points
        Q_PROPERTY( bool reset READ getReset WRITE setReset NOTIFY resetChanged )
		Q_PROPERTY( int minXPoint READ getMinXPoint WRITE setMinXPoint NOTIFY minXPointChanged )
		Q_PROPERTY( int minYPoint READ getMinYPoint WRITE setMinYPoint NOTIFY minYPointChanged )
		Q_PROPERTY( int minV READ getMinV WRITE setMinV NOTIFY minVChanged )
		Q_PROPERTY( int maxV READ getMaxV WRITE setMaxV NOTIFY maxVChanged )
		Q_PROPERTY( int minS READ getMinS WRITE setMinS NOTIFY minSChanged )

		Q_PROPERTY( int width READ getWidth WRITE setWidth NOTIFY widthChanged )
		Q_PROPERTY( int height READ getHeight WRITE setHeight NOTIFY heightChanged )
        /*Q_PROPERTY( int minPoints READ getMinPoints WRITE setMinPoints NOTIFY minPointsChanged )
		Q_PROPERTY( int maxPoints READ getMaxPoints WRITE setMaxPoints NOTIFY maxPointsChanged )
		Q_PROPERTY( int chgPoints READ getChgPoints WRITE setChgPoints NOTIFY chgPointsChanged )
		Q_PROPERTY( int framesMovement READ getFramesMovement WRITE setFramesMovement NOTIFY framesMovementChanged )*/


        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR
	
	public slots:
	/*	void setMinPoints(int i) {if(i>0) { m_minPoints = i; emit(minPointsChanged(i));}};
		void setMaxPoints(int i) {if(i>0) { m_maxPoints = i; emit(maxPointsChanged(i));}};
		void setChgPoints(int i) {if(i>0) { m_chgPoints = i; emit(chgPointsChanged(i));}};
		void setFramesMovement(int i) {if(i>0) { m_framesMovement = i; emit(framesMovementChanged(i));}};*/
		
	
       //if reset is clicked dont show the click just set the boolean to reset the points
	   void setReset(bool r); 
	   void setMinXPoint(int i) {if(i>0 && i<480) { m_minXPoint = i; emit(minXPointChanged(i));}};
	   void setMinYPoint(int i) {if(i>0 && i<640) { m_minYPoint = i; emit(minYPointChanged(i));}};
	   void setMinV(int i) {if(i>0 && i<640) { m_minV = i; emit(minVChanged(i));}};
	   void setMaxV(int i) {if(i>0 && i<640) { m_maxV = i; emit(maxVChanged(i));}};
	   void setMinS(int i) {if(i>0 && i<640) { m_minS = i; emit(minSChanged(i));}};
	   
	   //TODO make these as a method in camshift and independent of standard cam size but based on actual src size.
	   void setHeight(int i); 
	   void setWidth(int i); 

	public:
        CamShift();
        virtual ~CamShift();
		virtual bool init();

		bool m_reset;
		bool m_needToInit;
		
		bool getReset() {return m_reset;};
		int getMinXPoint() {return m_minXPoint;};
		int getMinYPoint() {return m_minYPoint;};
		int getMinV() {return m_minV;};
		int getMaxV() {return m_maxV;};
		int getMinS() {return m_minS;};

		int getWidth() {return m_width;};
		int getHeight() {return m_height;};

		int m_minXPoint;
		int m_minYPoint;
		int m_minV;
		int m_minS;
		int m_maxV;

		int m_width;
		int m_height;

		int m_maxwidth;
		int m_maxheight;

		/*		
		int m_minPoints;
		int m_maxPoints;
		int m_chgPoints;
		int m_framesMovement;
				
		int getMinPoints() {return m_minPoints;};
		int getMaxPoints() {return m_maxPoints;};
		int getChgPoints() {return m_chgPoints;};
		int getFramesMovement() {return m_framesMovement;};
*/
       /* int getThreshold() const;
        int getReplacement() const;
        bool getReset() const;
        plv::Enum getMethod() const;*/

    signals:
        void resetChanged(bool);
		void minXPointChanged(int);
		void minYPointChanged(int);
		void minVChanged(int);
		void maxVChanged(int);
		void minSChanged(int);

		void heightChanged(int);
		void widthChanged(int);

		/*
		void minPointsChanged(int);
        void maxPointsChanged(int);
		void chgPointsChanged(int);
		void framesMovementChanged(int);*/

	private:
        plv::CvMatDataInputPin* m_inInput;
       // plv::CvMatDataInputPin* m_inDelayed;
        
		//plv::CvMatDataInputPin* m_inInputColor;

		//plv::OutputPin<QString>* m_sumXY;
		plv::CvMatDataOutputPin* m_visualisationPin;
		plv::CvMatDataOutputPin* m_histPin;
		plv::OutputPin<QString>* m_cogRotatedRect;
		
		int m_trackObject;
		bool m_selectObject;
		
		//the trackwindow is hopefully the one that is used to search and is updated
		cv::Rect m_trackWindow;
		//the initial selection
		cv::Rect m_selection;
		cv::Mat m_hist;
	};
}
#endif // CamShift _H
