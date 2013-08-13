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
//#include <plvcore/Enum.h>
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
        Q_CLASSINFO("author", "Richard, changed a lot by Roberdus")
        Q_CLASSINFO("name", "Blob Tracker")
        Q_CLASSINFO("description", "Tracks blobs, blobselector can be used to see one blob by its ID, the factor can be set to have tracker respond on either its direction or its overlap, in a range of 0 to 100, zero indicating totaly based on direction 100 totaly based on overlap. For very fast moving tracking where ther might be no overlap one might select the boolean allow no overlap. This results in more noisy results (flickering back and forth positions). The orange colored blobs have not been updated in the particular frame and are thus not send over the tracks pin. They might for instance be not overlapping, or merged to another id. " )
		Q_PROPERTY( int numberOfBlobsTracked READ getNumberOfBlobsTracked WRITE setNumberOfBlobsTracked  NOTIFY numberOfBlobsTrackedChanged  )
		Q_PROPERTY( int blobSelector READ getBlobSelector WRITE setBlobSelector NOTIFY blobSelectorChanged )
		Q_PROPERTY( int factorDirOverlap READ getFactorDirOverlap WRITE setFactorDirOverlap NOTIFY factorDirOverlapChanged )
		Q_PROPERTY( bool averagePixelValue READ getAveragePixelValue WRITE setAveragePixelValue NOTIFY averagePixelValueChanged  )
		Q_PROPERTY( bool allowNoOverlap READ getAllowNoOverlap WRITE setAllowNoOverlap NOTIFY allowNoOverlapChanged  )

        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR
		
		int getBlobSelector() const;
		int getFactorDirOverlap() const;
		int getNumberOfBlobsTracked() const;
		//{return m_numberOfBlobsTracked;}
		bool getAveragePixelValue() {return m_averagePixelValue;}
		bool getAllowNoOverlap() {return m_allowNoOverlap;}

	public slots:
		void setBlobSelector(int blobid);
		void setFactorDirOverlap(int factor);
		//his is the boolean deciding whether or not to average the values in the track
		void setAveragePixelValue(bool i) {m_averagePixelValue = i; emit (averagePixelValueChanged(i));}
		void setAllowNoOverlap(bool b) {m_allowNoOverlap = b; emit (allowNoOverlapChanged(b));}
		void setNumberOfBlobsTracked(int i) {m_numberOfBlobsTracked = i; emit (numberOfBlobsTrackedChanged(i));}
		
	signals:
		void blobSelectorChanged (int s);
		void factorDirOverlapChanged (int s);
		void averagePixelValueChanged(bool value);
		void numberOfBlobsTrackedChanged(int i);
		void allowNoOverlapChanged(bool value);

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
		plv::OutputPin<bool>* m_outputAnnotationSituation;
		plv::OutputPin< QList<plvblobtracker::BlobTrack> >* m_outputBlobTracks;
		//plv::CvMatDataOutputPin* m_outputImage3;
		//plv::CvMatDataOutputPin* m_outputImage4;
		//plv::CvMatDataOutputPin* m_outputImage5;
        QList<BlobTrack> m_blobTracks;

		//unsigned??
		unsigned int m_blobSelector;
		unsigned int m_factor;
		bool m_averagePixelValue;
		bool m_allowNoOverlap;

        void matchBlobs(QList<Blob>& newBlobs, QList<BlobTrack>& blobTracks);
		unsigned int averagePixelsOfBlob(Blob blob,const cv::Mat& src);
        //void setTrackID(QList<BlobTrack>& blobTracks, int ID, int i);
		//void setTrackID(BlobTrack& trackunit);
		//int getIDBySum(int sum);
		unsigned int m_idCounter;
		unsigned int m_iterations;
		int m_biggerMaxCount;
		int m_maxNrOfBlobs; //max nr of seen blobs
		//int m_maxNrOfTrackedBlobs; //needs to be adjustable if it works!
		unsigned int m_numberOfBlobsTracked;
		int m_thresholdFramesMaxNrOfBlobs;
		int m_thresholdremove;
        //inline unsigned int getNewId() { return ++m_idCounter; }
		QList<unsigned int> m_idPool;
		inline unsigned int getNewId() { if(!m_idPool.isEmpty()) {return m_idPool.first();} else{return 999;}; }
		
		//TEST FPS callback from plugin
		QTime m_timeSinceLastFPSCalculation;
		int m_numFramesSinceLastFPSCalculation;
		float m_fps; /** running avg of fps */

		//ANNOTATION TOOL
		bool m_annotationneeded;

    };
}
#endif // BLOBTRACKER_H
