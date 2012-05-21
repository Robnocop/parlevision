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
    d->age = 0;
	d->bitsBeenSet = false;
	d->direction = 361;
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

/** called when this track did not match any blob */
void BlobTrack::notMatched( unsigned int frameNumber )
{
    ++d->age;
	//i believe framenumber changes on init
	//blob gets the framenumbers
    if( frameNumber - d->history.last().getFrameNr() > d->dieThreshold )
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

void BlobTrack::setBits(bool bit, int measurepoint)
{
	//init track shouldnt be here but it is for now
	if (!getBitsBeenSet())
	{
		//create seven entries for measured bits
		d->bitseq.push_back(false); //0
		d->bitseq.push_back(false); //1
		d->bitseq.push_back(false); //2
		d->bitseq.push_back(false); //3
		d->bitseq.push_back(false); //4
		d->bitseq.push_back(false); //5
		d->bitseq.push_back(false); //6

		//create the averaging values
		d->lastmeasurementpoint = 0;
		d->nrofmeasures = 0; //never divide by zero
		d->nrofones = 0; 

		d->bitsBeenSet = true;
	}

	int division = 0; 
	
	//next block of code saves measurements per timeslot and sets a bitvalue for the last timeslot when the nexttimeslot is entered 
	//actually a save assumption:
	if (measurepoint < 7)
	{
		if (measurepoint==(d->lastmeasurementpoint))
		{
			d->nrofmeasures = d->nrofmeasures +1;
			if (bit)
			{
				d->nrofones = d->nrofones +1;
			}
		}
		else //first measurement of new slot
		{			
			if (d->nrofmeasures> 0 )
			{
				//integer division keeps only the integer part
				division = (int) (d->nrofones * 10 )	/ d->nrofmeasures;
				qDebug() << "bit at " << d->lastmeasurementpoint << "ones: " << d->nrofones<< "measures: " << d->nrofmeasures << division;
				if(division > 5)
				{
					d->bitseq[d->lastmeasurementpoint] = 1;
					//qDebug() << "ID" << d->id << "at" << measurepoint << "set on one" << "measures" << d->nrofmeasures << "ones" << d->nrofones ;
					d->nrofmeasures = 0;
					d->nrofones = 0;
				}
				else
				{
					d->bitseq[d->lastmeasurementpoint] = 0;
					//qDebug() << "ID" << d->id << "at" << measurepoint << "set on zero" << "measures" << d->nrofmeasures << "ones" << d->nrofones;
					d->nrofmeasures = 0;
					d->nrofones = 0;
				}
			}
			else 
			{
				//this is a tricky choice
				d->bitseq[d->lastmeasurementpoint] = 0;
			}
		}
		d->lastmeasurementpoint = measurepoint;
	}
	
}

bool BlobTrack::getBit(int measurepoint)
{
	if (getBitsBeenSet())
		return d->bitseq[measurepoint];
		//return true;
	else 
		return false;
}

void BlobTrack::setCentroidLed(std::vector< cv::Point > cogs)
{
	//assuming three points
	d->centroid = (cv::Point( ( (cogs.at(0).x+cogs.at(1).x+cogs.at(2).x)/3), ((cogs.at(0).y+cogs.at(1).y+cogs.at(2).y)/3) ) );
}

void BlobTrack::setDirection(std::vector< cv::Point > cogs)
{
	//ADD a vote for setting direction with some tollerance e.g. within 15 degrees

	std::vector< cv::Point > threeBase(3);
	threeBase[0] =cv::Point(5,0);
	threeBase[1] =cv::Point(0,10);
	threeBase[2] =cv::Point(10,10);
	//shoudln't that be done in the header?
	#define PI 3.14159265
	
	std::vector< cv::Point > reorderd(3);
	
	//maybe allready require it to be three in the tracker it self, before calling setdirection.
	if (cogs.size() < 3)
	{
		qDebug() << "nr of LEDS <3,  this is not supposed to happen";
	}
	else if (cogs.size() > 3)
	{
		qDebug() << "nr of LEDS >3, this is not supposed to happen";
	}
	//TODO maybe assume size is transformed to 3 in tracker??
	else
	{
		//for (int i=0;i<cogs.size();i++)
		//{
		//	//save the found points to the known set (so left of front point =1, right of front point = 2, front point = 0)
		//	//so we simply have a line of the two bottom points, we need the third point here to know which point is left and right.
		//	//if (threeBase[1].x<threeBase[2].x)
		//	//{
		//	threeBase[i] = cogs.at(i);
		//}
		int onex,oney;
		long qone;
		//??width width +-= 640*640>int maxval?
		onex = cogs.at(0).x - cogs.at(1).x;
		oney = cogs.at(0).y - cogs.at(1).y;
		qone = onex*onex+oney*oney;

		int twox,twoy;
		long qtwo;
		twox = cogs.at(0).y - cogs.at(2).y;
		twoy = cogs.at(0).y - cogs.at(2).y;
		qtwo = twox*twox+twoy*twoy;
		
		//TODO use qthree as well and compare with smallest side value will directly give the top point
		if ((qone*3)>(qtwo*2) && (qtwo*3)>(2*qone))	
		{
			if (qone<qtwo)
			{
				//point two is top point 
				reorderd[0] = cogs.at(2);
				reorderd[1] = cogs.at(0);
				reorderd[2] = cogs.at(1);
			}
			else
			{
				//point one is top point	
				reorderd[0] = cogs.at(1);
				reorderd[1] = cogs.at(0);
				reorderd[2] = cogs.at(2);	
			}

			
			//??width width +-= 640*640>int maxval?
			onex = reorderd.at(0).x - reorderd.at(1).x;
			oney = reorderd.at(0).y - reorderd.at(1).y;
			qone = onex*onex+oney*oney;

			twox = reorderd.at(0).y - reorderd.at(2).y;
			twoy = reorderd.at(0).y - reorderd.at(2).y;
			qtwo = twox*twox+twoy+twoy;
		}
		else
		{
			reorderd[0] = cogs.at(0);
			reorderd[1] = cogs.at(1);
			reorderd[2] = cogs.at(2);
		}

		//now one and zero should be the shortedge points;
		//TODO already do this in tracker, 
		//if both are approximatly the same size than 0 is the top point, use approx 1.5 size
		if (! ((qone*3)>(qtwo*2) && (qtwo*3)>(2*qone)))
		{
			int avgy = (reorderd.at(1).y + reorderd.at(2).y)/2;
			threeBase[0] = cogs.at(0);
			// it is the long edge point
			
			if (reorderd.at(0).y< avgy)
			{
				//correct assumption

				/////	[0,0] 					 ->
				/////		     2				| 	 		   1
				////	0						|_								0
				////	
				////            1      [w,h]	-up to-			 2
				if(reorderd.at(1).x <  reorderd.at(2).x)
				{
					threeBase[1]= reorderd.at(1);
					threeBase[2]= reorderd.at(2);
				}
				else
				{
					threeBase[1]= reorderd.at(2);
					threeBase[2]= reorderd.at(1);
				}

			}

			else if (reorderd.at(0).y== avgy)
			{	
				// A:  x0 < x1 && y2<y1		  B:		! (x0 < x1) && y1<y2		
				//               2    or      1         
				//		0									0
				//				 1			  2
				
				//A
				if (reorderd.at(0).x < reorderd.at(1).x)
				{
					if (reorderd.at(1).y < reorderd.at(2).y)
					{
						threeBase[1]= reorderd.at(2);
						threeBase[2]= reorderd.at(1);
					}
					else
					{
						threeBase[1]= reorderd.at(1);
						threeBase[2]= reorderd.at(2);
					}
				}
				//B
				else
				{
					if (cogs.at(1).y < reorderd.at(2).y)
					{
						threeBase[1]= reorderd.at(2);
						threeBase[2]= reorderd.at(1);
					}
					else
					{
						threeBase[1]= reorderd.at(1);
						threeBase[2]= reorderd.at(2);
					}
				}
			}
			else //cogs.at(0).y> avgy
			{
				//correct assumption

				/////	[0,0] 				     ___
				/////			2				| 	 		      1
				////							|								
				////	0						 -->							0
				////              1      [w,h]	-up to-			2
				if(cogs.at(1).x >  reorderd.at(2).x)
				{
					threeBase[1]= reorderd.at(1);
					threeBase[2]= reorderd.at(2);
				}
				else
				{
					threeBase[1]= reorderd.at(2);
					threeBase[2]= reorderd.at(1);
				}

			}
			
			//todo assign the angle by tangens of two bottom points
			/////	 			
			/////  0
			/////
			////       2
			////   1     

			//float atan2 (       float y,       float x );
			float bottom = (float) (threeBase[2].x-threeBase[1].x);
			float side = (float) threeBase[2].y-threeBase[1].y;
//			d->direction = 361;
			
			//atan2 gives a value of -PI upto PI so +180 makes it from 0 to 360.
			d->direction = (int) (atan2(side,bottom)*180/PI +180); 

			//qDebug() << "the angle in degrees is: " << d->direction;		
		}
		else //so cog[0] is a shortedge point not the top point
		{
			qDebug() << "a strange thing happend in setting the direction apparently so we simply end it";
		}
	}//cogs size is 3

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

    d->history.append(blob);
    if( d->history.size() > d->historySize )
    {
        d->history.removeFirst();
    }

    // TODO use ringbuffer
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
        int from = d->history.size() - d->birthWindow;
        from = from > 0 ? from : 0;

        unsigned int count = 0;
        for( int i = from; i < d->history.size(); ++i )
        {
			//added results in count always 3 so birthwindow is 3
			//count++;
        }
        if( count >= d->birthThreshold )
            d->state = BlobTrackNormal;
		//count is indeed always 0 at this point qDebug() << "count is: " << (int)count;
		//qDebug() << "count is: " << (int)count << "at x:" << (int)blob.getCenterOfGravity().x << "at y:" << (int)blob.getCenterOfGravity().y;
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
    QString info = QString("ID: %1").arg(d->id);
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
