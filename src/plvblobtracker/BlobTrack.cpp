#include "BlobTrack.h"
#include <stdlib.h>
#include <limits>
//added for debuging offcourse
#include <QDebug>

using namespace plvblobtracker;

BlobTrack::BlobTrack(unsigned int id,
                     Blob& blob,
                     int birthThreshold,
                     int birthWindow,
                     int dieThreshold,
                     int historySize,
                     int trackSize)
{
    d = new BlobTrackData(id, blob, birthThreshold, birthWindow, dieThreshold, historySize, trackSize);

    d->id = id;
    d->historySize = historySize;
    d->trackSize = trackSize;
    d->birthWindow = birthWindow;
	//not really neccesary for age, direction and velocity i guess
	d->age = 0;
	d->direction = 361; //(361)
	d->velocity = 0;
	d->color = cvScalar( qrand()%255, qrand()%255, qrand()%255);
    d->history.append(blob);
}

BlobTrack::~BlobTrack()
{
    // nothing to do, QSharedDataPointer does ref counting
}

const Blob& BlobTrack::getLastMeasurement() const
{
    return d->history.last();
}

const Blob& BlobTrack::getAPreviousMeasurement(unsigned int prevsteps) const
{
	if (d->history.size() >prevsteps)
	{	
		int temp = d->history.size()-prevsteps;
		return d->history.at(temp);
	} else
	{
		return d->history.last();
	}
}

//might be good to combine this code with timesincelast update-method of the track.
/** called when this track did not match any blob */
void BlobTrack::notMatched( unsigned int timesinceupdatetime )
{
    ++d->age;
	//i believe framenumber changes on init
	//blob gets the framenumbers
    //if( frameNumber - d->history.last().getFrameNr() > d->dieThreshold )
	//change into directly using the timesincelastupdate
	if (timesinceupdatetime > d->dieThreshold)
    {
		//qDebug() << "the die threshold is" << (int) d->dieThreshold;
		//threshold was/is set in header to 10
        d->state = BlobTrackDead;
    }
}

void BlobTrack::setID(int newid)
{
	d->id = newid;
}

void BlobTrack::setVelocity(std::vector< cv::Point > cogs)
{
	int onex,oney;
	double qone;
	//??width width +-= 640*640>int maxval?
	onex = abs(cogs.at(0).x - cogs.at(1).x);
	oney = abs(cogs.at(0).y - cogs.at(1).y);
	qone = onex*onex+oney*oney;
	//velocity per frame	
	//d->velocity = (int) (sqrt(qone));
	//velocity per second ?? gettime is in milliseconds
	d->velocity = (int) 1000*(sqrt(qone))/getTimeSinceLastMeasurement();
}

void BlobTrack::setLastUpdate(unsigned int lastupdate)
{
	d->lastUpdate = lastupdate;
}

void BlobTrack::setTimeSinceLastUpdate(unsigned int timeamount)
{
	d->timesincelastmeasurement = timeamount;
}



//old, new
//will set to 180 on zero speed 
void BlobTrack::setDirection(std::vector< cv::Point > cogs)
{
	//ADD a vote for setting direction with some tollerance e.g. within 15 degrees
	//onex = cogs.at(0).x - cogs.at(1).x;

	//std::vector< cv::Point > threeBase(3);
	//threeBase[0] =cv::Point(5,0);
	
	//shoudln't that be done in the header?
	#define PI 3.14159265
		
	if (cogs.size()==2)
	{	
		float bottom,side;
		//??width width +-= 640*640>int maxval?
		bottom = cogs.at(0).x - cogs.at(1).x;
		side = cogs.at(0).y - cogs.at(1).y;
		d->direction = (int) (atan2(side,bottom)*180/PI +180);
		//qDebug() << "jawel size is 2" << onex;


	} else 
	{
		//atan2 gives a value of -PI upto PI so *180/PI +180 makes it from 0 to 360.
		//d->direction = (int) (atan2(side,bottom)*180/PI +180); 
		d->direction = (int) 0;
	}
		

}
//void BlobTrack::setState(PlvOpenCVBlobTrackState statename)
//{
//	d->state = statename;
//	//d->state = BlobTrackBirth;
//	qDebug() << "I have set state of ID " << (int)this->getId();
//}

/** adds a measurement to this track */
void BlobTrack::addMeasurement( const Blob& blob )
{
    //might run out of scop of uint 
	if (d->age >65000)
	{
		qDebug() << "age is about to explode";
	}
	else
	{	
		++d->age;
	}

	//TODO average value over history don't take two random recent values only
	if (d->age>5)
	{
		std::vector< cv::Point > cogstoset;
		//try to average later on instead of using age
		cogstoset.push_back(getAPreviousMeasurement(5).getCenterOfGravity());
		cogstoset.push_back(blob.getCenterOfGravity());
		setDirection(cogstoset);
		cogstoset[0] = getLastMeasurement().getCenterOfGravity();
		setVelocity(cogstoset);
		//d->direction = (int) (d->direction + tempdirection /2);
	}
	else if (d->age>1)
	{
		std::vector< cv::Point > cogstoset; 
		cogstoset.push_back(getLastMeasurement().getCenterOfGravity());
		cogstoset.push_back(blob.getCenterOfGravity());
		setDirection(cogstoset);
		setVelocity(cogstoset);
	}

	if (getDirection() >360)
	{
		qDebug() << "somehow the direction washigher than 360";
		d->direction = getDirection() % 360;
	}

    d->history.append(blob);
    if( d->history.size() > d->historySize )
    {
        d->history.removeFirst();
    }

    // TODO use ringbuffer
	//??this will not be saved in to its history?
    d->track.push_back(blob.getCenterOfGravity());
    if( d->track.size() > d->trackSize )
    {
        d->track.remove(0);
    }

	//check if the dead has come to live
	if (d->state == BlobTrackDead)
	{
		//qDebug() << "the die threshold is" << (int) d->dieThreshold;
		//threshold was/is set in header to 10
		d->state = BlobTrackBirth;
		//qDebug() << "I have come alive";
		//works
	}

    if( d->state == BlobTrackBirth )
    {
		//does not make sense!
   //     int from = d->history.size() - d->birthWindow;
   //     from = from > 0 ? from : 0;

   //     unsigned int count = 0;
   //     for( int i = from; i < d->history.size(); ++i )
   //     {
			////added results in count always 3 so birthwindow is 3
			//count++;
   //     }
   //     if( count >= d->birthThreshold )
   //         d->state = BlobTrackNormal;
		if (d->age > d-> birthThreshold)
			d->state = BlobTrackNormal;
		//count is indeed always 0 at this point qDebug() << "count is: " << (int)count;
		//qDebug() << "count is: " << (int)count << "for" << d->id << "at x:" << (int)blob.getCenterOfGravity().x << "at y:" << (int)blob.getCenterOfGravity().y;
    }
}

int BlobTrack::matches( const Blob& blob ) const
{
    if( d->state == BlobTrackDead )
        return 0;
    const Blob& last = getLastMeasurement();
    return blob.matchingArea(last);
}

/** draws the blob, its track and prediction */
void BlobTrack::draw( cv::Mat& target ) const
{
    assert( d->history.size() > 0 );
    assert( target.depth() == CV_8U );

    const double maxVal    = 255.0;
    const cv::Scalar red   = CV_RGB(maxVal,0,0);
    const cv::Scalar blue  = CV_RGB(0,0,maxVal);
    const cv::Scalar green = CV_RGB(0,maxVal,0);

    // draw tail
    const int tailLength = d->historySize;
    const double step = maxVal / tailLength;
    int count = 0;
    for( int i=d->history.size() <= tailLength ? 0 : d->history.size()-tailLength; i < d->history.size(); ++i)
    {
        const Blob& b = d->history[i];
        int shade = step*++count;
        b.drawContour(target, CV_RGB(0,shade,0), true);
    }

    // draw last measurement
    const Blob& b = getLastMeasurement();
    b.drawContour(target, green, true);
    b.drawCenterOfGravity(target, blue);
    b.drawBoundingRect(target,red);
    QString info = QString("ID: %1 D: %2 V: %3").arg(d->id).arg(getDirection()).arg(d->velocity);
    b.drawString(target, info);

    // draw track
    if( d->track.size() > 1 )
    {
        for(int i=0; i< d->track.size()-1; ++i)
        {
            cv::line(target, d->track[i], d->track[i+1], d->color, 1, CV_AA);
        }
    }
}
