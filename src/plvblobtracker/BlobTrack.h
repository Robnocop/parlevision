#ifndef PLVBLOBTRACK_BLOBTRACK_H
#define PLVBLOBTRACK_BLOBTRACK_H

#include <opencv/cv.h>
#include <QVector>
#include "Blob.h"

namespace plvblobtracker
{
    enum PlvOpenCVBlobTrackState {
        BlobTrackBirth,  /** not enough measurements yet */
        BlobTrackNormal, /** normal state */
        BlobTrackLost,   /** not showing often enough any more */
        BlobTrackDead    /** removed from tracking */
    };

    class BlobTrackData : public QSharedData
    {
    public:


        inline BlobTrackData( unsigned int _id,
                              Blob& _blob,
                              int _birthThreshold,
                              int _birthWindow,
                              int _dieThreshold,
                              int _historySize,
                              int _trackSize) :
            id(_id),
            birthThreshold( _birthThreshold ),
            birthWindow( _birthWindow ),
            dieThreshold( _dieThreshold ),
            historySize(_historySize),
            trackSize(_trackSize),
            age(0),
			direction(361),
			lastUpdate(0),//should never be 0 i guess
			velocity(0),
			state(BlobTrackBirth),
			centroid(cv::Point(0,0))
        {
            assert(_blob.isValid());
            history.append(_blob);
        }

        unsigned int id;

        /** track birth if i matches in j frames */
        unsigned int birthThreshold;
        unsigned int birthWindow;

        /** track dies if not seen for dieThreshold frames */
        unsigned int dieThreshold;

        unsigned int historySize;
        unsigned int trackSize;
        unsigned int age;
		
		/** the last LED bit measurements */
		//std::vector< bool > bitseq; 
		////should these actually be public I doubt it
		//unsigned int lastmeasurementpoint;
		//unsigned int nrofmeasures;
		//unsigned int nrofones;
		
		//direction 
		unsigned int direction;
		//velocity
		unsigned int velocity;
		unsigned int timesincelastmeasurement;

		//for time since last update
		unsigned int lastUpdate;
		
		//last measured centroid of the three leds
		cv::Point centroid;

	    PlvOpenCVBlobTrackState state;

        QList<Blob>        history; /** the history of this blob track */
        QVector<cv::Point> track;   /** the actual route this blob has followed */
        cv::Vec2d          speed;   /** the speed of this blob track */
		cv::Vec2d          conversionFactor; /** conversion factor from pixels to millimeters. */
        cv::Scalar         color;   /** color to use when drawing this track onto an image */
    };

	//reason why it got stuck! 10*int max*point list is a lot to keep track of
	//int trackSize=INT_MAX 
	//int historySize=10, set to later on 5
	//historysize 10
    class BlobTrack
    {
    public:
        BlobTrack(unsigned int id,
                  Blob& blob,
                  int birthThreshold=3,
                  int birthWindow=9, //equaled 10 <=  historysize sounds ok
                  int dieThreshold=4000, //?should be quite long i guess, 1-60s seems reasonable, actually removed from list 
                  int historySize=10, //number of blobs in the vector
                  int trackSize=1000); //tail length

        virtual ~BlobTrack();

        /** copy constructor */
        inline BlobTrack( const BlobTrack& other ) { d = other.d; }

        /** assignment operator */
        inline BlobTrack& operator=( const BlobTrack& other )
        {
            d = other.d;
            return *this;
        }

        inline unsigned int getId() const { return d->id; }
		inline unsigned int getDirection() const { return d->direction; }
		inline unsigned int getVelocity() const { return d->velocity; }
		inline unsigned int getLastUpdate() const { return d->lastUpdate; }
		inline unsigned int getTimeSinceLastMeasurement() const {return d->timesincelastmeasurement; }
		
        /** adds a measurement to this track */
        void addMeasurement( const Blob& blob );

		/** resets the ID based on an external measurement in the blob */
		void setID(int id);
		
		/** saves the last x slots with measured bits*/
		//void BlobTrack::setBits(bool bit, int slot);
		void setDirection(std::vector< cv::Point > cogs);
		void setVelocity(std::vector< cv::Point > cogs);
		void setLastUpdate(unsigned int updatetime);
		void setTimeSinceLastUpdate(unsigned int amountoftimepast);

		/** call when track is not matched */
        void notMatched(  unsigned int frameNumber );

        /** returns last blob measurement */
        const Blob& getLastMeasurement() const;
		const Blob& getAPreviousMeasurement(unsigned int previous) const;

        /** returns the size of the history */
        inline int getHistorySize() const { return d->history.size(); }

        /** draws the blob, its track and prediction. Target must have depth CV_8U. */
        void draw( cv::Mat& target ) const;

		int matches( const Blob& blob ) const;

		//this is not the proper way to get the last framenr, this is the first framenumber from the history
        //inline unsigned int getLastFrameNr() const { return d->history[0].getFrameNr(); }
		//this is not really a useable framenumber for tracking, as it depends on the blob detector
		inline unsigned int getLastFrameNr() const { return d->history[d->history.size()].getFrameNr(); }


        inline int getAge() const { return d->age; }

        inline PlvOpenCVBlobTrackState getState() const { return d->state; }

		//resets state of a reborn blob 
		//void setState(const PlvOpenCVBlobTrackState statename);
		//inline PlvOpenCVBlobTrackState setState(int statename) const { d->state = statename; return d->state;}

    private:
        QSharedDataPointer<BlobTrackData> d;
		
    };
}

#endif
