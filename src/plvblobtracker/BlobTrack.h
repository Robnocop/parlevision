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

		//For annotation we take out some stuff A
        inline BlobTrackData( unsigned int _id,
                              Blob& _blob,
                              //int _birthThreshold,
                              //int _dieThreshold,
                              int _historySize,
                              int _trackSize) :
							  //int _avgOver) :
            id(_id),
            historySize(_historySize),
            trackSize(_trackSize),
			pid(0),
			averagepixel(0),
            state(BlobTrackBirth),
			merged(false)
			
			//ANNO 
			//,
			//birthThreshold( _birthThreshold ),
            //dieThreshold( _dieThreshold ), 
			//avgOver(_avgOver),
			//lastUpdate(0),//should never be 0 i guess
			//age(0),
			//direction(361),
			// avgvelocity(0),
			// avgdirection(0)*/
        {
            assert(_blob.isValid());
            history.append(_blob);
        }
		
		
		
		unsigned int id;
		unsigned int pid;
		QList<unsigned int > pids;
		
		unsigned int averagepixel; 
				
		//not unsigned to be able compare to GUI set values.
        int historySize;
        int trackSize;
		int avgOver;

		   /** track birth if i matches in j frames */
        // unsigned int birthThreshold;
        // unsigned int birthWindow;

		//unsigned int age;
        /** track dies if not seen for dieThreshold frames */
       // unsigned int dieThreshold;
		
		// velocity
		// unsigned int velocity;
		// float velocity2;

		//direction 
		// unsigned int direction;
		// unsigned int avgvelocity;
		// unsigned int avgdirection;
		// unsigned int timesincelastmeasurement;
		// QVector<unsigned int>       speed;   /** the speed of this blob track */
		// QVector<unsigned int>       rotation;   /** the speed of this blob track */
		// cv::Vec2d          conversionFactor; /** conversion factor from pixels to millimeters. */
  
		//for time since last update
		//unsigned int lastUpdate;

		bool merged;
		
		PlvOpenCVBlobTrackState state;
		
		QList<Blob>        history; /** the history of this blob (track) */
        QVector<cv::Point> track;   /** the actual route this blob has followed */
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
                  //int birthThreshold=1, //legacy naming, actually number of frames a previous dead frame has been given a value before it seen as a normal track again, //number of frames before a track is added is set in BlobTracker.cpp now
				  //int dieThreshold=2, //30 number of frames a track has not been updated before it is removed from the list was 400, ?should be quite long i guess, 1-60s seems reasonable, actually removed from list, TODO set to 2 frames for annotation 
                  int historySize=10, //perhaps 2 for anno 10 number of complete blobs in the track vector that can be recalled, TODO make GUI setting and is set to 3 frames for annotation
                  int trackSize=60); //90 drawn and saved tail length also arraysize of recallable speed and rotation values, TODO set to 60 for annotationtool
				  //int avgOver= 2); //value over which the direction and speed are averaged, now set to 5 suiting, TODO is this possible and needed for annotation?

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
		inline unsigned int getPID() const; // const { return d->pid; }
		inline QList<unsigned int > getPIDS() const { return d->pids;}
		
		//Framebased
		inline unsigned int getAveragePixel() const {return d->averagepixel; }
		/**  ANNOTATION TAKE OUT STUFF
		inline float getVelocity2() const { return d->velocity2; }
		inline unsigned int getDirection() const { return d->direction; }
		inline unsigned int getVelocity() const { return d->velocity; }
		inline unsigned int getAvgVelocity() const { return d->avgvelocity; }
		inline unsigned int getAvgDirection() const { return d->avgdirection; }
		inline unsigned int getLastUpdate() const { return d->lastUpdate; }
		inline unsigned int getTimeSinceLastMeasurement() const {return d->timesincelastmeasurement; }
		
		this is not the proper way to get the last framenr, this is the first framenumber from the history
        inline unsigned int getLastFrameNr() const { return d->history[0].getFrameNr(); }
		this is not really a useable framenumber for tracking, as it depends on the blob detector
		inline unsigned int getLastFrameNr() const { return d->history[d->history.size()].getFrameNr(); }
		inline int getAge() const { return d->age; }
		// saves the last x slots with measured bits
		void BlobTrack::setBits(bool bit, int slot);
		void setDirection(std::vector< cv::Point > cogs);
		void setVelocity(std::vector< cv::Point > cogs);
		void setVelocity2(std::vector< cv::Point > cogs);
		void setLastUpdate(unsigned int updatetime);
		void setTimeSinceLastUpdate(unsigned int amountoftimepast);
		*/

		//inline unsigned int getLastUpdate() const { return d->lastUpdate; }
		//inline unsigned int getTimeSinceLastMeasurement() const {return d->timesincelastmeasurement; }

		/** returns last blob measurement */
        const Blob& getLastMeasurement() const;
		const Blob& getAPreviousMeasurement(int previous) const;
		//TODO check why also as an unsigned int?
		const Blob& getAPreviousMeasurement(unsigned int previous) const;

        /** returns the size of the history */
        inline int getHistorySize() const { return d->history.size(); }
		
        inline PlvOpenCVBlobTrackState getState() const { return d->state; }
		void setState(PlvOpenCVBlobTrackState s) {d->state = s;}

        /** adds a measurement to this track */
        void addMeasurement( const Blob& blob );

		/** resets the ID based on an external measurement in the blob */
		void setID(int id);
		
		void setPID(int id);
		void addPID(int id);
		void removePID(int id);
		void removeAllPIDs();

		//void setAveragePixel(unsigned int pixelvalueofaveragez);
		void setAveragePixelValue(unsigned int i) {d->averagepixel = i;}

		//an attempt to send when a blob is merged, this will only hold for as long as it is overlapping with an non-dead track.
		void setMerged(bool b) {d->merged = b;}
		//inline int getHistorySize() const { return d->history.size(); }
		inline bool getMerged() const { return d->merged;}

		/** call when track is not matched */
        //void notMatched(  unsigned int frameNumber );

        /** draws the blob, its track and prediction. Target must have depth CV_8U. */
        void draw( cv::Mat& target ) const;
		
		//to override blobs overlapping the info
		void drawInfo( cv::Mat& target ) const;

		int matches( const Blob& blob ) const;

	private:
        QSharedDataPointer<BlobTrackData> d;
		
		
		//doesnt work in draw somehow cv::Scalar red;, blue, green, yellow, chocolate, deeppink, purple, orangered, salmon, darkgoldenrod, steelblue;
		
    };
}//need to add metatype in order to use it for pins. 
Q_DECLARE_METATYPE( QList<plvblobtracker::BlobTrack> )

#endif
