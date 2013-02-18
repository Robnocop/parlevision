/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * All rights reserved.
  *
  * This file is part of the plvcore module of ParleVision.
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

#ifndef PLVBLOBTRACK_LEDBLOBTRACKER_H
#define PLVBLOBTRACK_LEDBLOBTRACKER_H 

#include <plvcore/PipelineProcessor.h>
#include <plvcore/Pin.h>
#include <QStringList>

#include "Blob.h"
#include "LedBlobTrack.h"

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

namespace plvblobtracker
{
    class LedBlobTracker : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( LedBlobTracker )
        Q_CLASSINFO("author", "Robby") //based on Richard's Blob Tracker Code
        Q_CLASSINFO("name", "Led Blob Tracker")
        Q_CLASSINFO("description", "Tracks blobs with LEDs sending cyclic codes" )
		Q_PROPERTY( int blobSelector READ getBlobSelector WRITE setBlobSelector NOTIFY blobSelectorChanged )
		Q_PROPERTY( int threshold READ getThreshold WRITE setThreshold NOTIFY blobThresholdChanged )
		Q_PROPERTY( int minLedSize READ getMinLedSize WRITE setMinLedSize NOTIFY minLedSizeChanged )
		Q_PROPERTY( int maxLedSize READ getMaxLedSize WRITE setMaxLedSize NOTIFY maxLedSizeChanged )
		Q_PROPERTY( int blinkTime READ getBlinkTime WRITE setBlinkTime NOTIFY blinkTimeChanged )
        //Q_PROPERTY( double alpha READ getAlpha WRITE setAlpha NOTIFY alphaChanged )
        //Q_PROPERTY( double beta READ getBeta WRITE setBeta NOTIFY betaChanged )
        //Q_PROPERTY( double gamma READ getGamma WRITE setGamma NOTIFY gammaChanged )

        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR
		
		int getBlobSelector() const;
		int getThreshold() const;
		int getMinLedSize() const;
		int getMaxLedSize() const;
		int getBlinkTime() const;

	public slots:
		void setBlobSelector(int blobid);
		void setThreshold(int threshold);
		void setMinLedSize(int minledsize);
		void setMaxLedSize(int maxledsize);
		void setBlinkTime (int blinktime);
	
	signals:
		void blobSelectorChanged (int s);
		void blobThresholdChanged (int t);
		void minLedSizeChanged (int l);
		void maxLedSizeChanged (int l);
		void blinkTimeChanged (int b);

    public:
        LedBlobTracker();
        virtual ~LedBlobTracker();

        // overloaded function from PipelineElement
        bool init();
        bool deinit() throw();
        bool start();
        bool stop();

    private:
        plv::CvMatDataInputPin* m_inputImage;
        plv::InputPin< QList<plvblobtracker::Blob> >* m_inputBlobs;
        plv::CvMatDataOutputPin* m_outputImage;
		plv::CvMatDataOutputPin* m_outputImage2;
		plv::CvMatDataOutputPin* m_outputImage3;
		plv::CvMatDataOutputPin* m_outputImage4;
		plv::CvMatDataOutputPin* m_outputImage5;
        QList<LedBlobTrack> m_blobTracks;

		//unsigned??
		unsigned int m_blobSelector;
		unsigned int m_threshold;
		unsigned int m_minLedSize;
		unsigned int m_maxLedSize;
		unsigned int m_blinkTime;
		
        void matchBlobs(QList<Blob>& newBlobs, QList<LedBlobTrack>& ledBlobTracks); //changed to ledB instead of blobT
        //void setTrackID(QList<BlobTrack>& blobTracks, int ID, int i);
		void setTrackID(LedBlobTrack& trackunit);
		int getIDBySum(int sum);
		unsigned int m_idCounter;
		unsigned int m_iterations;
        inline unsigned int getNewId() { return ++m_idCounter; }

		//TEST FPS callback from plugin
		QTime m_timeSinceLastFPSCalculation;
		int m_numFramesSinceLastFPSCalculation;
		float m_fps; /** running avg of fps */

    };
}
#endif // BLOBTRACKER_H
