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

#ifndef PLVBLOBTRACK_BLOBTRACKER_H
#define PLVBLOBTRACK_BLOBTRACKER_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/Pin.h>
#include <QStringList>

#include "Blob.h"
#include "BlobTrack.h"

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

namespace plvblobtracker
{
    class BlobTracker : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( BlobTracker )
        Q_CLASSINFO("author", "Richard")
        Q_CLASSINFO("name", "Blob Tracker")
        Q_CLASSINFO("description", "Tracks blobs, blobselector can be used to see one blob by its ID, the factor can be set to have tracker respond on either its direction or its overlap, in a range of 0 to 10, zero indicating totaly based on direction 10 totaly based on overlap" )
		Q_PROPERTY( int blobSelector READ getBlobSelector WRITE setBlobSelector NOTIFY blobSelectorChanged )
		Q_PROPERTY( int factorDirOverlap READ getFactorDirOverlap WRITE setFactorDirOverlap NOTIFY factorDirOverlapChanged )

        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR
		
		int getBlobSelector() const;
		int getFactorDirOverlap() const;
	
	public slots:
		void setBlobSelector(int blobid);
		void setFactorDirOverlap(int factor);

	signals:
		void blobSelectorChanged (int s);
		void factorDirOverlapChanged (int s);
	

    public:
        BlobTracker();
        virtual ~BlobTracker();

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
		//plv::CvMatDataOutputPin* m_outputImage3;
		//plv::CvMatDataOutputPin* m_outputImage4;
		//plv::CvMatDataOutputPin* m_outputImage5;
        QList<BlobTrack> m_blobTracks;

		//unsigned??
		unsigned int m_blobSelector;
		unsigned int m_factor;

        void matchBlobs(QList<Blob>& newBlobs, QList<BlobTrack>& blobTracks);
        //void setTrackID(QList<BlobTrack>& blobTracks, int ID, int i);
		void setTrackID(BlobTrack& trackunit);
		//int getIDBySum(int sum);
		unsigned int m_idCounter;
		unsigned int m_iterations;
		int m_biggerMaxCount;
		int m_maxNrOfBlobs;
		int m_thresholdFramesMaxNrOfBlobs;
		unsigned int m_thresholdremove;
        inline unsigned int getNewId() { return ++m_idCounter; }

		//TEST FPS callback from plugin
		QTime m_timeSinceLastFPSCalculation;
		int m_numFramesSinceLastFPSCalculation;
		float m_fps; /** running avg of fps */

    };
}
#endif // BLOBTRACKER_H
