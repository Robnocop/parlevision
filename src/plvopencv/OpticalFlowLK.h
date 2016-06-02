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

#ifndef OpticalFlowLK_H
#define OpticalFlowLK_H

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
    class OpticalFlowLK : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( OpticalFlowLK )
        Q_CLASSINFO("author", "Robby van Delden")
        Q_CLASSINFO("name", "Optical Flow (LK)")
        Q_CLASSINFO("description", "Estimation of movement of interesting points."
					"\n Explenation of the coloured dots: "
					"Yellow are newly tracked feature points so after initialisation. "
					"Red are lost/no longer tracked points are "
					"In filled blue where the points were the last frame "
					"In non filled green circles it shown where they currently are ");

		//GUI to set min number of points and to reset the tracked points
        Q_PROPERTY( bool reset READ getReset WRITE setReset NOTIFY resetChanged )
        Q_PROPERTY( int minPoints READ getMinPoints WRITE setMinPoints NOTIFY minPointsChanged )
		Q_PROPERTY( int maxPoints READ getMaxPoints WRITE setMaxPoints NOTIFY maxPointsChanged )
		Q_PROPERTY( int chgPoints READ getChgPoints WRITE setChgPoints NOTIFY chgPointsChanged )
		Q_PROPERTY( int framesMovement READ getFramesMovement WRITE setFramesMovement NOTIFY framesMovementChanged )


        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR
	
	public slots:
		void setMinPoints(int i) {if(i>0) { m_minPoints = i; emit(minPointsChanged(i));}};
		void setMaxPoints(int i) {if(i>0) { m_maxPoints = i; emit(maxPointsChanged(i));}};
		void setChgPoints(int i) {if(i>0) { m_chgPoints = i; emit(chgPointsChanged(i));}};
		void setFramesMovement(int i) {if(i>0) { m_framesMovement = i; emit(framesMovementChanged(i));}};
		
	
       //if reset is clicked dont show the click just set the boolean to reset the points
	   void setReset(bool r) {m_reset = false; m_needToInit=true; emit(resetChanged(r));};

    public:
        OpticalFlowLK();
        virtual ~OpticalFlowLK();
		virtual bool init();
		bool m_needToInit;
		bool m_reset;
		int m_minPoints;
		int m_maxPoints;
		int m_chgPoints;
		int m_framesMovement;

		bool getReset() {return m_reset;};
		int getMinPoints() {return m_minPoints;};
		int getMaxPoints() {return m_maxPoints;};
		int getChgPoints() {return m_chgPoints;};
		int getFramesMovement() {return m_framesMovement;};

       /* int getThreshold() const;
        int getReplacement() const;
        bool getReset() const;
        plv::Enum getMethod() const;*/

    signals:
        void resetChanged(bool);
		void minPointsChanged(int);
        void maxPointsChanged(int);
		void chgPointsChanged(int);
		void framesMovementChanged(int);

	private:
        plv::CvMatDataInputPin* m_inInput;
       
        plv::CvMatDataInputPin* m_inDelayed;
        plv::CvMatDataInputPin* m_inInputColor;

		plv::OutputPin<QString>* m_sumXY;
		plv::CvMatDataOutputPin* m_visualisationPin;
				
		bool m_addRemovePt;
		int m_nrofpointstracked;
		int m_framesCounter;

        //points as used in demo 
		//add another set of points to be able to save the direction of change and process this
		//cv::vector<cv::Point2f> m_points[2];
		cv::vector<cv::Point2f> m_points[2];
		std::list<float> m_summationxlastframes;
		std::list<float> m_summationylastframes;
		
		//float m_summationx;
		//float m_summationy;
		
		//methods:
		
	  
	   
	   //this setting can't be altered in realtime
		//void setInfrared(bool b) {m_infrared = b; qDebug()<< "restart pipeline to incorporate change"; emit(infraredChanged(b));}
		
       
	};
}
#endif // OpticalFlowLK_H
