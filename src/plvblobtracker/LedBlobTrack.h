#ifndef PLVBLOBTRACK_LEDBLOBTRACK_H
#define PLVBLOBTRACK_LEDBLOBTRACK_H

#include <opencv/cv.h>
#include <QVector>
#include "Blob.h"


namespace plvblobtracker
{
	//PlvOpenCVBlobTrackState is defined in blobtrack if  that is available. don't know how to solve it :(

	
    enum PlvOpenCVLedBlobTrackState {
        LedBlobTrackBirth,  /** not enough measurements yet */
        LedBlobTrackNormal, /** normal state */
        LedBlobTrackLost,   /** not showing often enough any more */
        LedBlobTrackDead    /** removed from tracking */
    };

    class LedBlobTrackData : public QSharedData
    {
    public:


        inline LedBlobTrackData( unsigned int _id,
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
			bitsBeenSet(false),
			direction(361),
			state(LedBlobTrackBirth),
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

        int historySize;
        int trackSize;
        unsigned int age;
		
		/** the last LED bit measurements */
		std::vector< bool > bitseq; 
		//should these actually be public I doubt it
		unsigned int lastmeasurementpoint;
		unsigned int nrofmeasures;
		unsigned int nrofones;
		//direction based on three LEDs
		unsigned int direction;
		//last measured centroid of the three leds
		cv::Point centroid;

		//
		bool bitsBeenSet;

        PlvOpenCVLedBlobTrackState state;

        QList<Blob>        history; /** the history of this blob track */
        QVector<cv::Point> track;   /** the actual route this blob has followed */
        cv::Vec2d          speed;   /** the speed of this blob track */
		cv::Vec2d          conversionFactor; /** conversion factor from pixels to millimeters. */
        cv::Scalar         color;   /** color to use when drawing this track onto an image */
    };

	//reason why it got stuck! 10*int max*point list is a lot to keep track of
	//int trackSize=INT_MAX 
	//int historySize=10,
    class LedBlobTrack
    {
    public:
        LedBlobTrack(unsigned int id,
                  Blob& blob,
                  int birthThreshold=3,
                  int birthWindow=10,
                  int dieThreshold=10,
                  int historySize=5,
                  int trackSize=1000);

        virtual ~LedBlobTrack();

        /** copy constructor */
        inline LedBlobTrack( const LedBlobTrack& other ) { d = other.d; }

        /** assignment operator */
        inline LedBlobTrack& operator=( const LedBlobTrack& other )
        {
            d = other.d;
            return *this;
        }

        inline unsigned int getId() const { return d->id; }
		inline unsigned int getBitsBeenSet() const { return d->bitsBeenSet; }
		inline unsigned int getDirection() const { return d->direction; }
		inline cv::Point getCentroidLed() const { return d->centroid; }

        /** adds a measurement to this track */
        void addMeasurement( const Blob& blob );

		/** resets the ID based on an external measurement in the blob */
		void setID(int id);
		/** sets the average of the three leds being the centroid of the three leds*/
		void setCentroidLed( std::vector< cv::Point > cogs );

		/** saves the last x slots with measured bits*/
		//void BlobTrack::setBits(bool bit, int slot);
		void setBits(bool bit, int slot);
		void setDirection(std::vector< cv::Point > cogs);

		/** gets the x-ed bit*/
		bool LedBlobTrack::getBit(int measurepoint);

		/** call when track is not matched */
        void notMatched(  unsigned int frameNumber );

        /** returns last blob measurement */
        const Blob& getLastMeasurement() const;

        /** returns the size of the history */
        inline int getHistorySize() const { return d->history.size(); }

        /** draws the blob, its track and prediction. Target must have depth CV_8U. */
        void draw( cv::Mat& target ) const;

		

        int matches( const Blob& blob ) const;

        inline cv::Vec2d getSpeed() const { return d->speed; }

        inline unsigned int getLastFrameNr() const { return d->history[0].getFrameNr(); }

        inline int getAge() const { return d->age; }

        inline PlvOpenCVLedBlobTrackState getState() const { return d->state; }

		//resets state of a reborn blob 
		//void setState(const PlvOpenCVLedBlobTrackState statename);
		//inline PlvOpenCVLedBlobTrackState setState(int statename) const { d->state = statename; return d->state;}

    private:
        QSharedDataPointer<LedBlobTrackData> d;
		
    };
}

#endif
