#include "BlobTrack.h"
#include <stdlib.h>
#include <limits>
//added for debuging offcourse
#include <QDebug>

using namespace plvblobtracker;

BlobTrack::BlobTrack(unsigned int id,
                     Blob& blob,
                     //int birthThreshold,
                     //int dieThreshold,
                     int historySize,
                     int trackSize)
					 //int avgOver)
{
    d = new BlobTrackData(id, blob, historySize, trackSize);
    d->id = id;
    d->historySize = historySize;
    d->trackSize = trackSize;
	d->averagepixel = 0;
	d->color = cvScalar( qrand()%255, qrand()%255, qrand()%255);
    d->history.append(blob); 

	

    //not really neccesary for age, direction and velocity i guess
	//d->age = 0;
	//d->avgOver = avgOver;
	//d->direction = 361; //(361)
	//d->velocity = 0;
	//d->avgvelocity = 0;
	//d->avgdirection = 361;
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


//not matched no longer used in ANNOTATION
//might be good to combine this code with timesincelast update-method of the track.
/** called when this track did not match any blob */
//void BlobTrack::notMatched( unsigned int timesinceupdatetime )
//{
//	//++d->age;
//////i believe framenumber changes on init
//////blob gets the framenumbers
////   //if( frameNumber - d->history.last().getFrameNr() > d->dieThreshold )
//////change into directly using the timesincelastupdate
//
//	//if (timesinceupdatetime > d->dieThreshold)
// //   {
//	//	//qDebug() << "the die threshold is" << (int) d->dieThreshold;
//	//	//threshold was/is set in header to 10
// //       d->state = BlobTrackDead;
// //   }
//	////for annotation just always kill non-overlapping tracks
//	//d->state = BlobTrackDead;
//}

void BlobTrack::setID(int newid)
{
	d->id = newid;
}

void BlobTrack::setPID(int newid)
{
	 d->pid = newid;
}

void BlobTrack::addPID(int newid)
{
	 d->pids.push_back(newid);
}

void BlobTrack::removePID(int newid)
{
	//d->pids.
	//REMOVE DEADTRACKS

	qDebug() << "start removing the size is " << d->pids.size();
	QList<unsigned int>::iterator it = d->pids.begin();
	while (it != d->pids.end()) {
		unsigned int pidunit = *it;
		if (pidunit == newid)
		{
			it = d->pids.erase(it);
			qDebug() << "remove";
		}
		else
			++it;
	}
	//TODO not for now!
	//also allow removal of the last pid
	//if (it == d->pids.end())
	//{
	//	unsigned int pidunit2 = *it;
	//	if (pidunit2 == newid)
	//	{
	//		//it = d->pids.erase(it);
	//		it = d->pids.erase(it);
	//	}
	//}
	//qDebug() << "after removing the size is " << d->pids.size();
}

void BlobTrack::removeAllPIDs()
{
	qDebug() << "start removing the size is " << d->pids.size();
	QList<unsigned int>::iterator it = d->pids.begin();
	while (it != d->pids.end()) {
		unsigned int pidunit = *it;
		//if (pidunit == newid)
		//{
			it = d->pids.erase(it);
			qDebug() << "remove";
		//}
		//else
		//	++it;
	}
}



/** adds a measurement to this track */
void BlobTrack::addMeasurement( const Blob& blob )
{
	//qDebug() << "add measurement for blobtrack " << d->id;
    std::vector< cv::Point > cogstoset; 
	cogstoset.push_back(getLastMeasurement().getCenterOfGravity());
	cogstoset.push_back(blob.getCenterOfGravity());
	
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
		qDebug() << "TODO I have come alive, this should be very unlikely";
	}

    if( d->state == BlobTrackBirth )
    {
  		//now set in blobtracker
		//d->state = BlobTrackNormal;
    }
}

//odd trail: matches --> blob.matchingArea(&blob) -->(blob.)overlappingAreaRect( &blob, area ) -->  overlappingArea
int BlobTrack::matches( const Blob& blob ) const
{
    //no longer makes sense to punish the dead
	/*if( d->state == BlobTrackDead )
        return 0;*/
    const Blob& last = getLastMeasurement();
	//seems to be the actual overlapping pixels, as it uses both the blob and track
	//qDebug()<< "matching area" << blob.matchingArea(last) << "id" << d->id;
    return blob.matchingArea(last);
}

unsigned int BlobTrack::getPID() const
{
	if (d->pids.size()>0)
	{
		int toreturnis = d->pids.first();
		if (toreturnis %11 == 0)
		{
			return 1;
		}
		else
		{
			return toreturnis;
		}
	}
	else
	{
		//red if no id in any case
		return 0;
	}
}

/** draws the blob, its track and prediction */
void BlobTrack::draw( cv::Mat& target ) const
{
    assert( d->history.size() > 0 );
    assert( target.depth() == CV_8U );

	//TODO a better placement of setting colors, now every draw all the colors are created little bit overkill
	
	
    // draw tail
    const int tailLength = d->historySize;
	//usd to be maxval
    const double step = 255.0 / tailLength;
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
	
	//set colours:
	double maxVal = 255.0;
	//0
	cv::Scalar red = CV_RGB(maxVal,0,0);
    //1
	//cv::Scalar newblue = CV_RGB(0,20,5*maxVal/6);
	cv::Scalar newblue = CV_RGB(0,120,210);
    //2
	cv::Scalar green = CV_RGB(0,maxVal,0);
	//3 yellow yellow	N,V,X	255;255;0 inspired by: http://web.njit.edu/~kevin/rgb.txt.html
	cv::Scalar yellow = CV_RGB(maxVal,maxVal,0);
	//4 210;105;30
	cv::Scalar chocolate = CV_RGB(210,105,30);
	//5 DeepPink 255;20;147
	cv::Scalar deeppink = CV_RGB(255,20,147);
	//6 HotPink3 [purple]	205;96;144
	cv::Scalar purple = CV_RGB(205,96,144);
	//7 OrangeRed
	cv::Scalar orangered = CV_RGB(255,69,0);
	//8 salmon: pink	X	255;192;203
	cv::Scalar salmon = CV_RGB(255,192,203);
	//9	DarkGoldenrod1	X	255;185;15
	cv::Scalar darkgoldenrod = CV_RGB(255,185,15);
	//10 Steel Blue	N	35;107;142
	cv::Scalar steelblue = CV_RGB(35,107,142);
	//colors should be a global variable, didnt work fuckit.
	QList<cv::Scalar> colors;
	colors.push_back(red);
	colors.push_back(newblue);
	colors.push_back(green);
	colors.push_back(yellow);
	colors.push_back(chocolate);
	colors.push_back(deeppink);
	colors.push_back(purple);
	colors.push_back(orangered);
	colors.push_back(salmon);
	colors.push_back(darkgoldenrod);
	colors.push_back(steelblue);
	
	if (this->getState() == BlobTrackBirth)
	{ //only limited amount of frames
		//blue is new
		b.drawContour(target,cv::Scalar(255,0,0),true);
	}
	else if(this->getMerged())
	{
		//red is wrong
		b.drawContour(target,cv::Scalar(0,0,255),true);
	}
	else if(!colors.size()<11)
		b.drawContour(target, colors.at(getPID()%11), true);
	else
		b.drawContour(target,  CV_RGB(0,0,255.0), true);
	
	//old drawing process:
	/*if(!colors.size()<11)
		b.drawContour(target, colors.at(getPID()%11), true);
	else
		b.drawContour(target,  CV_RGB(0,0,255.0), true);*/

    b.drawCenterOfGravity(target, newblue); //why the random colours then?
    b.drawBoundingRect(target,red);
	
	//QString info = QString("ID: %1 D: %2 V: %3 M: %4 Z: %5").arg(d->id).arg(getDirection()).arg(d->avgvelocity).arg(d->merged).arg(getAveragePixel());
	
	//drawInfo(target);
	
	////TODO mak a pid qList
	////QString info = QString("ID: %1 PID: %2 M: %3 Z: %4").arg(getId()).arg(getPID()).arg(d->merged).arg(getAveragePixel());
	//QString info = QString("ID: %1 M: %2 Z: %3 PID").arg(getId()).arg(d->merged).arg(getAveragePixel());
	//QString toappend2 = QString(",");
	//for (int i=0; i<d->pids.size(); i++)
	//{
	//	QString toappend = QString(" %1").arg(d->pids.at(i));
	//	info.append(toappend);
	//	if (i < (d->pids.size()-1))
	//	{
	//		//qDebug() << "append , bastard";
	//		info.append(toappend2);
	//	}
	//}
	//b.drawString(target, info);

    // draw tail
    if( d->track.size() > 0 )
    {
        for(int i=0; i< d->track.size()-1; ++i)
        {
            //cv::line(target, d->track[i], d->track[i+1], d->color, 1, CV_AA);
			cv::line(target, d->track[i], d->track[i+1], colors.at(getPID()%11), 1, CV_AA);
	    }
    }
}

/** draws the blob, its track and prediction */
void BlobTrack::drawInfo( cv::Mat& target ) const
{
    assert( d->history.size() > 0 );
    assert( target.depth() == CV_8U );

    // draw last measurement
    const Blob& b = getLastMeasurement();
	
	//QString info = QString("ID: %1 M: %2 Z: %3 PID").arg(getId()).arg(d->merged).arg(getAveragePixel());
	QString info = QString("ID: %1 Z: %2 PID").arg(getId()).arg(getAveragePixel());
	QString toappend2 = QString(",");
	for (int i=0; i<d->pids.size(); i++)
	{
		QString toappend = QString(" %1").arg(d->pids.at(i));
		info.append(toappend);
		if (i < (d->pids.size()-1))
		{
			//qDebug() << "append , bastard";
			info.append(toappend2);
		}
	}
	b.drawString(target, info);
}
