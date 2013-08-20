#include "BlobTrack.h"
#include <stdlib.h>
#include <limits>
//added for debuging offcourse
#include <QDebug>

using namespace plvblobtracker;

BlobTrack::BlobTrack(unsigned int id,
                     Blob& blob,
                     int birthThreshold,
                     int dieThreshold,
                     int historySize,
                     int trackSize,
					 int avgOver)
{
    d = new BlobTrackData(id, blob, birthThreshold, dieThreshold, historySize, trackSize, avgOver);

    d->id = id;
    d->historySize = historySize;
    d->trackSize = trackSize;
	d->avgOver = avgOver;
    //not really neccesary for age, direction and velocity i guess
	d->age = 0;
	d->averagepixel = 0;
	d->direction = 361; //(361)
	d->velocity = 0;
	d->avgvelocity = 0;
	d->avgdirection = 361;
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


const Blob& BlobTrack::getAPreviousMeasurement(int prevsteps) const
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
	//for annotation just always kill non-overlapping tracks
	d->state = BlobTrackDead;
}

void BlobTrack::setID(int newid)
{
	d->id = newid;
}

void BlobTrack::setPID(int newid)
{
	 d->pid = newid;
}

//realtime
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

//framebased
void BlobTrack::setVelocity2(std::vector< cv::Point > cogs)
{
	int onex,oney;
	double qone;
	//??width width +-= 640*640>int maxval?
	onex = abs(cogs.at(0).x - cogs.at(1).x);
	oney = abs(cogs.at(0).y - cogs.at(1).y);
	qone = onex*onex+oney*oney;
	//velocity per frame	
	//d->velocity = (int) (sqrt(qone));
	//pixels of movement between cogs in this frame.
	d->velocity2 = (float) (sqrt(qone));
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


	} 
	else 
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
    //might run out of scope of uint 
	if (d->age >65000)
	{
		//qDebug() << "age is about to explode";
	}
	else
	{	
		++d->age;
	}

	//TODO average value over history don't take two random recent values only

	//doensn't need to be 5 anymore as we average it lateron
	//if (d->age>5)
	//{
	//	std::vector< cv::Point > cogstoset;
	//	//try to average later on instead of using age
	//	//why do this over 5 steps?
	//	//cogstoset.push_back(getAPreviousMeasurement(5).getCenterOfGravity());
	//	cogstoset.push_back(getAPreviousMeasurement(1).getCenterOfGravity());
	//	cogstoset.push_back(blob.getCenterOfGravity());
	//	setDirection(cogstoset);
	//	cogstoset[0] = getLastMeasurement().getCenterOfGravity();
	//	setVelocity(cogstoset);
	//}
	if (d->age>1)
	{
		std::vector< cv::Point > cogstoset; 
		cogstoset.push_back(getLastMeasurement().getCenterOfGravity());
		cogstoset.push_back(blob.getCenterOfGravity());
		setDirection(cogstoset);
		setVelocity(cogstoset);
		setVelocity2(cogstoset);
	}


	//d->direction = (int) (d->direction + tempdirection /2);
	//should only be possible if !age>1
	if (getDirection() >360)
	{
		//qDebug() << "somehow the direction washigher than 360";
		if (d->age!=0)
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
	//added for averaging
	d->speed.push_back(getVelocity());
    d->rotation.push_back(getDirection());
    if( d->track.size() > d->trackSize )
    {
        d->track.remove(0);
		d->speed.remove(0);
		d->rotation.remove(0);
    }

	//AVERAGING 
	double totaldir,total;
	total = 0;
	totaldir= 0;
	//TODO check historysize
	int averageover = d->avgOver; //40 (+-2s) or 500?? why not use tracksize for this as well?
	//int minspeed = 1000;
	//int maxspeed = 0;

	if( d->speed.size() > averageover )
	{
		for(int i=d->speed.size()-averageover; i< d->speed.size(); i++)
		{
			total = total + d->speed[i];
			/*if (d->speed[i] >maxspeed)
			{
				maxspeed = d->speed[i]; 
			} 
			if (d->speed[i] <= minspeed)
			{
				minspeed = d->speed[i];
			} 
*/
		}
		//d->velocity = (unsigned int) total / averageover ;
		d->avgvelocity = (unsigned int) total / averageover ;
		//qDebug() << "average" << d->velocity << "minspeed"  << minspeed << " but maxspeed:" << maxspeed ;
		//qDebug() << "average speed" << d->velocity << "velocity actual" << d->speed[d->speed.size()-1];

		//for direction we should take into account rapid changes in direction
		double differencetemp = 0;
		//double difference = 0;
		int j = 0;
		int i = 1;
		//while ((difference <45) && i<(averageover+1))
		//TODO create a MERGED blob condition in this case you do not use the last measurements of the blobs but use measurements untill the blob was merged.
		//TODO incorporate 360=0 it is not a fair comparison now, changes around 0 will be way more extreme and not averaged
		while (i<(averageover+1))
		{
			j = d->rotation.size()-i; 
			//whatever explicit cast
			differencetemp = (int) abs((double) (d->rotation[j] - d->rotation[d->rotation.size()]));
			if (differencetemp >60)
			{
				//TODO probablyy negative and >361 will give better results for averaging than brute shortcutting it
				if ((d->rotation[j] - (d->rotation[d->rotation.size()]+360)) <60)
				{
					if (d->avgdirection >300)
					{
						totaldir= totaldir+360;
					}
				}
				else if ((d->rotation[j]+360) - d->rotation[d->rotation.size()] <60)
				{
					if (d->avgdirection >300)
					{
						totaldir= totaldir+360;
					}
					/*else //totaldir= totaldir +0;
					{
						totaldir= totaldir;
					}*/
				}
				else
				{
					break;
				}

			}
			else
			{
				totaldir= totaldir+d->rotation[j];
			}
			i++;
		}

		/*if (difference > 45)
		{
			qDebug() << "i until difference is" << i;
		}*/
		
		//d->direction = totaldir/i; 
		if (i>1)
		{
			d->avgdirection = totaldir/(i-1); 
		}
		else
		{
			d->avgdirection = getDirection(); 
		}
		//qDebug() << "average dir" << d->direction << "velocity actual" << d->rotation[d->rotation.size()-1];
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

//odd trail: matches --> blob.matchingArea(&blob) -->(blob.)overlappingAreaRect( &blob, area ) -->  overlappingArea
int BlobTrack::matches( const Blob& blob ) const
{
    if( d->state == BlobTrackDead )
        return 0;
    const Blob& last = getLastMeasurement();
	//seems to be the actual overlapping pixels, as it uses both the blob and track
	//qDebug()<< "matching area" << blob.matchingArea(last) << "id" << d->id;
    return blob.matchingArea(last);
}

/** draws the blob, its track and prediction */
void BlobTrack::draw( cv::Mat& target ) const
{
    assert( d->history.size() > 0 );
    assert( target.depth() == CV_8U );

	//TODO a better placement of setting colors, now every draw all the colors are created little bit overkill
	//colors
	const double maxVal    = 255.0;
    //0
	const cv::Scalar red   = CV_RGB(maxVal,0,0);
    //1
	const cv::Scalar blue  = CV_RGB(0,0,maxVal);
    //2
	const cv::Scalar green = CV_RGB(0,maxVal,0);
	//3 yellow yellow	N,V,X	255;255;0 inspired by: http://web.njit.edu/~kevin/rgb.txt.html
	const cv::Scalar yellow = CV_RGB(maxVal,maxVal,0);
	//4 210;105;30
	const cv::Scalar chocolate = CV_RGB(210,105,30);
	//5 DeepPink 255;20;147
	const cv::Scalar deeppink = CV_RGB(255,20,147);
	//6 HotPink3 [purple]	205;96;144
	const cv::Scalar purple = CV_RGB(205,96,144);
	//7 OrangeRed
	const cv::Scalar orangered = CV_RGB(255,69,0);
	//8 salmon: pink	X	255;192;203
	const cv::Scalar salmon = CV_RGB(255,192,203);
	//9	DarkGoldenrod1	X	255;185;15
	const cv::Scalar darkgoldenrod = CV_RGB(255,185,15);
	//10 Steel Blue	N	35;107;142
	const cv::Scalar steelblue = CV_RGB(35,107,142);
	QList<cv::Scalar> colors;
	colors.push_back(red);
	colors.push_back(blue);
	colors.push_back(green);
	colors.push_back(yellow);
	colors.push_back(chocolate);
	colors.push_back(deeppink);
	colors.push_back(purple);
	colors.push_back(orangered);
	colors.push_back(salmon);
	colors.push_back(darkgoldenrod);
	colors.push_back(steelblue);
	
    // draw tail
    const int tailLength = d->historySize;
    const double step = maxVal / tailLength;
    int count = 0;
    
	for( int i=d->history.size() <= tailLength ? 0 : d->history.size()-tailLength; i < d->history.size(); ++i)
    {
        const Blob& b = d->history[i];
        //CHANGED for annotation:
		int shade = (step*++count)/4;
        b.drawContour(target, CV_RGB(shade,shade,shade), true);
    }

    // draw last measurement
    const Blob& b = getLastMeasurement();
	//CHANGED
	b.drawContour(target, colors.at(getPID()%11), true);

    b.drawCenterOfGravity(target, blue);
    b.drawBoundingRect(target,red);
	//CHANGED: 
	//QString info = QString("ID: %1 D: %2 V: %3 M: %4 Z: %5").arg(d->id).arg(getDirection()).arg(d->avgvelocity).arg(d->merged).arg(getAveragePixel());
	QString info = QString("ID: %1 PID: %2 M: %3 Z: %4").arg(getId()).arg(getPID()).arg(d->merged).arg(getAveragePixel());
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
