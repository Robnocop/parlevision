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
#include "BlobChangeData.h"

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

namespace plvblobtracker
{
	enum PlvBlobTrackState {
			Ok, //everything is fine
			NoOverlap,//there was a track assigned without overlap, some mismatch is expected
			NewBlob,  //there are more blobs than tracks, ids will be needed for the newly created tracks
			LessBlobs, //there are less blobs than tracks, an id has been removed is this correct?
			MultipleTracks,//TODO there is at least one blob that have been assigned to more than one tracks.
			NotMatched //there is no blob matched to a certain track
	};


    class BlobTracker : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( BlobTracker )
        Q_CLASSINFO("author", "Roberdus, some basics by Richard")
        Q_CLASSINFO("name", "Blob Tracker")
        Q_CLASSINFO("description", "Tracks blobs, blobselector can be used to see one blob by its ID, the factor can be set to have tracker respond on either its direction or its overlap, in a range of 0 to 100, zero indicating totaly based on direction 100 totaly based on overlap. For very fast moving tracking where ther might be no overlap one might select the boolean allow no overlap. This results in more noisy results (flickering back and forth positions). The orange colored blobs have not been updated in the particular frame and are thus not send over the tracks pin. They might for instance be not overlapping, or merged to another id. " )
		Q_PROPERTY( int numberOfBlobsTracked READ getNumberOfBlobsTracked WRITE setNumberOfBlobsTracked  NOTIFY numberOfBlobsTrackedChanged  )
		Q_PROPERTY( int blobSelector READ getBlobSelector WRITE setBlobSelector NOTIFY blobSelectorChanged )
		
		
		
		//not for annotation
		//Q_PROPERTY( bool allowNoOverlap READ getAllowNoOverlap WRITE setAllowNoOverlap NOTIFY allowNoOverlapChanged  )
		//Q_PROPERTY( int factorDirOverlap READ getFactorDirOverlap WRITE setFactorDirOverlap NOTIFY factorDirOverlapChanged )
        //Q_PROPERTY( bool averagePixelValue READ getAveragePixelValue WRITE setAveragePixelValue NOTIFY averagePixelValueChanged  )
		//int getFactorDirOverlap() const;
		//bool getAllowNoOverlap() {return m_allowNoOverlap;}
		//bool getAveragePixelValue() {return m_averagePixelValue;}

		/** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR
		
		int getBlobSelector() const;
		
		int getNumberOfBlobsTracked() const;	

	public slots:
		void setBlobSelector(int blobid);
		void setNumberOfBlobsTracked(int i) {m_numberOfBlobsTracked = i; emit (numberOfBlobsTrackedChanged(i));}
			
		//dont do this stuff for annotation: this is the boolean deciding whether or not to average the values in the track
		//void setAveragePixelValue(bool i) {m_averagePixelValue = i; emit (averagePixelValueChanged(i));}
		//void setAllowNoOverlap(bool b) {m_allowNoOverlap = b; emit (allowNoOverlapChanged(b));}
		//void setFactorDirOverlap(int factor);
	signals:
		void blobSelectorChanged (int s);
		void numberOfBlobsTrackedChanged(int i);
		
		//void allowNoOverlapChanged(bool value);
		//void factorDirOverlapChanged (int s);
		//void averagePixelValueChanged(bool value);

	public:
        BlobTracker();
        virtual ~BlobTracker();

        // overloaded function from PipelineElement
        bool init();
        bool deinit() throw();
        bool start();
        bool stop();

    private:
        //plv::CvMatDataInputPin* m_inputImage;
		plv::CvMatDataInputPin*	m_inputDepthImage;
        plv::InputPin< QList<plvblobtracker::Blob> >* m_inputBlobs;
        
		plv::InputPin<QString>* m_fileNameInputPin;
		plv::InputPin<int>* m_fileNameNrInputPin;
		//plv::InputPin<bool>* m_correctimagedirectoryboolInputPin;

		plv::OutputPin<QString>* m_fileNameOutputPin;
		plv::OutputPin<int>* m_fileNameNrOutputPin;
		plv::OutputPin<bool>* m_correctimagedirectoryboolOutputPin;
		
		plv::CvMatDataOutputPin* m_outputImage;
		plv::OutputPin<bool>* m_outputAnnotationNeeded;
		plv::OutputPin< QList<plvblobtracker::PlvBlobTrackState> >* m_outputAnnotationSituation;
		plv::CvMatDataOutputPin* m_outputImage2;
		plv::OutputPin< QList<plvblobtracker::BlobTrack> >* m_outputBlobTracks;
		
		plv::CvMatDataInputPin*	m_inputVideoImage;
		plv::CvMatDataOutputPin* m_outputImage3;

		//plv::CvMatDataOutputPin* m_outputImage3;
		//plv::CvMatDataOutputPin* m_outputImage4;
		//plv::CvMatDataOutputPin* m_outputImage5;
        QList<BlobTrack> m_blobTracks;

		plv::InputPin<bool>* m_correctimagedirectoryboolInputPin;
		
		QList<plvblobtracker::BlobChangeData> m_blobchanges;
		QList<plvblobtracker::PlvBlobTrackState> m_blobtrackstate;

		QString m_filenameBCD;
		void readFile(QString filename);

		//unsigned??
		unsigned int m_blobSelector;

		//not for annotation 
		//unsigned int m_factor;
		//bool m_averagePixelValue;
		//bool m_allowNoOverlap;

        void matchBlobs(QList<Blob>& newBlobs, QList<BlobTrack>& blobTracks);
		unsigned int averagePixelsOfBlob(Blob blob,const cv::Mat& src);
      
		unsigned int m_idCounter;
		unsigned int m_iterations;
		int m_biggerMaxCount;
		int m_maxNrOfBlobs; //max nr of seen blobs
		
		unsigned int m_numberOfBlobsTracked;
		int m_thresholdFramesMaxNrOfBlobs;
		        
		QList<unsigned int> m_idPool;
		inline unsigned int getNewId() { if(!m_idPool.isEmpty()) {return m_idPool.first();} else{return 999;}; }
		
		//don't use this for annotation
		//TEST FPS callback from plugin
		//QTime m_timeSinceLastFPSCalculation;
		//int m_numFramesSinceLastFPSCalculation;
		//float m_fps; /** running avg of fps */

		//ANNOTATION TOOL
		bool m_annotationneeded;
		bool m_debugstuff;
		bool m_skipprocessbool;
		bool m_firstloop;
		int m_previousAnnoSerial;
		bool m_correctimagedirectorybool;
    };
}//need to add metatype in order to use it for pins. //TODO doesnt work
Q_DECLARE_METATYPE( QList<plvblobtracker::PlvBlobTrackState> )
#endif // BLOBTRACKER_H
