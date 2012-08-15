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

//TODO create an averaging for direction an velocity
//TODO add the situation in which two blobs merge, one blob  should be assigne two tracks and both tracks should be sent. 
//we can assume that blobs will only exit if they are close to the borders and thus we know when merging or exiting is more likely.
//TODO add a configurable diethreshold this is very case dependent.
//TODO add a way of sending the tracks instead of the blobs (add something like VPBlobToString) e.g. with option of sending vector with contours of blob as well 

#include "BlobTracker.h"
#include "matrix.h"
#include "munkres.h"

//#include <cmath>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>
#include <plvcore/Util.h>
#include <limits>

using namespace plv;
using namespace plvblobtracker;

BlobTracker::BlobTracker() :  
	m_idCounter(0),
	//why here and not in init?
	m_thresholdremove(65534), //should be linked to diethreshold? & <UINT_MAX is also important for the cycle speed of 
	m_thresholdFramesMaxNrOfBlobs(10),
	m_blobSelector(0),
	m_factor(2) //should be in range 0-10

{
    m_inputImage = createCvMatDataInputPin( "input image", this );
    m_inputImage->addSupportedDepth(CV_8U);
	m_inputImage->addSupportedDepth(CV_16U);
    //m_inputImage->addSupportedChannels(4);
	m_inputImage->addSupportedChannels(3);
	//added but TODO
	m_inputImage->addSupportedChannels(1);

    m_inputBlobs = createInputPin< QList<plvblobtracker::Blob> >( "input blobs" , this );
    m_outputImage = createCvMatDataOutputPin( "blob tracker image", this);
	m_outputImage2 = createCvMatDataOutputPin( "selected blob image", this);
	//m_outputImage3 = createCvMatDataOutputPin( "output test image", this);
	//temp
	//m_outputImage4 = createCvMatDataOutputPin( "pointselection image", this);
	//m_outputImage5 = createCvMatDataOutputPin( "above threshold in blob image", this);
}

BlobTracker::~BlobTracker()
{
}

bool BlobTracker::init()
{
	m_timeSinceLastFPSCalculation.start();
    m_fps = 30.0; //to prevent using fps before it is set, 30 is the most likely framerate for kinect and normal cameras.  
	return true;
}

bool BlobTracker::deinit() throw()
{
    m_blobTracks.clear();
    return true;
}

bool BlobTracker::start()
{
	m_biggerMaxCount = 0;
	m_maxNrOfBlobs = 0;
    return true;
}

bool BlobTracker::stop()
{
    m_blobTracks.clear();
    return true;
}

bool BlobTracker::process()
{
	//time based measurements
	++m_numFramesSinceLastFPSCalculation;
	int elapsed = m_timeSinceLastFPSCalculation.elapsed();

    CvMatData in = m_inputImage->get();
	//cv::Mat& source = in;

    //CvMatData out = CvMatData::create(in.properties());
	CvMatData out = CvMatData::create(640,480,16);
    cv::Mat& dst = out;
    dst = cv::Scalar(0,0,0);
	
	//TODO create new mattching algorithm that i can undcerstand and that uses direction, velocity and overlap.
	//MATCH BLOBS
	QList<plvblobtracker::Blob> newBlobs = m_inputBlobs->get();
    matchBlobs(newBlobs, m_blobTracks);

    foreach( BlobTrack t, m_blobTracks )
    {
        t.draw(dst);
    }
	
    m_outputImage->put(out);

	//TODO reduce number of images
	CvMatData out2 = CvMatData::create(640,480,16);
	cv::Mat& dst2 = out2;
    dst2 = cv::Scalar(0,0,0);
	
	//testing of alive versus dead blobs.
//#if 0
	for( int i=0; i < m_blobTracks.size(); i++ )
    {
        const BlobTrack& track = m_blobTracks.at(i);
		const Blob& b = track.getLastMeasurement();
		//each frame?
		if (track.getState() == BlobTrackDead)
		{
		//at least get it out of our list
		//delete(&track);
			b.drawContour(dst2,cv::Scalar(155,150,0),true);
			qDebug() << "Id off dead blob"  << (int)track.getId();
			m_blobTracks.removeAt(i);
		} 
		else if (track.getId() == getBlobSelector())
		{
			if (track.getState() == BlobTrackBirth)
			{
				b.drawContour(dst2,cv::Scalar(255,255,255),true);
			}
			else if (track.getState() == BlobTrackNormal)
			{
				b.drawContour(dst2,cv::Scalar(255,0,255),true);
			} //unreachable
			else if (track.getState() == BlobTrackDead)
			{
				//b.drawContour(dst2,cv::Scalar(0,0,255),true);
			
	//			getLastMeasurement() const
	//{
	//    return d->history.last();
	//}
				b.drawContour(dst2,cv::Scalar(155,150,0),true);
				qDebug() << "Id off dead blob"  << (int)track.getId();
			
			}	
		}
    }
//#endif
	
	//get the FPS and reset the elapsed so we have a XXX (bit) second code.
	//remove the dead tracks
	if(elapsed > m_thresholdremove ) //10000
	{		
		// add one so elapsed is never 0 and
		// we do not get div by 0
		m_fps = (m_numFramesSinceLastFPSCalculation * 1000) / elapsed;
		//m_fps = m_fps == -1.0f ? fps : m_fps * 0.9f + 0.1f * fps;
		qDebug() << "FPS in BlobTracker: " << (int)m_fps << "elapsed time" << elapsed;
		m_timeSinceLastFPSCalculation.restart();
		m_numFramesSinceLastFPSCalculation = 0;
		
		//loop through all the ids for test on dead, the threshold for diening is set somewhere else this is only to get to disappear from the list as well 
		//for( int i=0; i < m_blobTracks.size(); i++ )
		//{		
		//	//const BlobTrack& track = m_blobTracks.at(i);
		//	BlobTrack& track = m_blobTracks[i];
		//	//const BlobTrack& track = tracks.at(i);
		////	const Blob& b = track.getLastMeasurement();
		//
		//	// a pre-equisite per se, not necceraly but doesn't improve it much
		//	if (track.getState() == BlobTrackDead)
		//	{
		//		//at least get it out of our list
		//		//delete(&track);
		//		m_blobTracks.removeAt(i);
		//	}
		//	//emit framesPerSecond(m_fps);
		//}
	}

	//show image of leds in one selected blob and show all detected drawn blobs
	m_outputImage2->put(out2);
    return true;
}

//

//assign a id according to the bitcode combination
//TODO implement the function
void BlobTracker::setTrackID(BlobTrack& trackunit)
{
//	//TODO make a case like 
//	int summed =0;
//	//summed = trackunit.getBit(0)*1+trackunit.getBit(1)*2+trackunit.getBit(2)*4+trackunit.getBit(3)*8+trackunit.getBit(4)*16+trackunit.getBit(5)*32+trackunit.getBit(6)*64;
//	summed = trackunit.getBit(0)*1+trackunit.getBit(1)*2+trackunit.getBit(2)*4+trackunit.getBit(3)*8+trackunit.getBit(4)*16+trackunit.getBit(5)*32+trackunit.getBit(6)*64;
//	
//	int number = getIDBySum(summed);
//	
//	//if (number < 99)
//	//{
//			trackunit.setID(number);
////}	
	
}


// match the old blob to a new detected blob using
// maximum area converage
// also called the hungarian algorithm
void BlobTracker::matchBlobs(QList<Blob>& blobs, QList<BlobTrack>& tracks)
{
    //const int sizeDiffThreshold = 1000;
    //const int minBlobThreshold = 0;
	#define PI 3.14159265

    //const int acceptedSizeDiff = (width * height * sizeDiffThreshold) / 1000.0f;
    //const int minMatch = (width * height * minBlobThreshold) / 10000.0f;

    // max accepted size difference between two blobs in pixels
    //const int maxSizeDiff = INT_MAX;

    // filter on minimum blob size
    //const int minArea = 1;

    //const unsigned int frameNr = this->getProcessingSerial();

    // make matrix of blobs and tracks
    // a1 b1
    // a2 b2
    // a3 b3

	//doesn't seemed to be applied nor neccesry
    //QVector< QList<int> > candidatesForBlobs( blobs.size() );
    //QVector< QList<int> > candidatesForTracks( tracks.size() );

	//could be slightly optimized by saving at what moment maxscore is reached
	//only need to loop through the score untill then not all again
    double maxScore = std::numeric_limits<double>::min();
	double minScorePrediction = std::numeric_limits<double>::max();
	double maxScorePrediction = std::numeric_limits<double>::min();

	float expectedx, expectedy;
	double distance;

    for(int i=0; i < tracks.size(); i++ )
    {
        const BlobTrack& track = tracks.at(i);
        //QList<int>& candidatesForTrack = candidatesForTracks[i];
		
		//maybe it starts at 1 instead of 0 due ++j instead of j
        for( int j=0; j < blobs.size(); j++ )
        {
            const Blob& blob = blobs.at(j);
            //QList<int>& candidatesForBlob = candidatesForBlobs[j];

            double score = (double)track.matches(blob);
          /*  if( score > 0 )
            {
                candidatesForBlob.append(i);
                candidatesForTrack.append(j);
            }*/

			//before setting actual lastupdate in the blob

			//could have been done neater to circumvent applying the same code again upon actual matching
			//need to use time in ms in order to circumvent rounding to 0 in int.
			unsigned int timesinceupdate;
			if (m_timeSinceLastFPSCalculation.elapsed() > track.getLastUpdate()) 
			{
					timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() - track.getLastUpdate();
			}
			else //you can assume it has cycled 
			{
					timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() + m_thresholdremove - track.getLastUpdate();
			}
			
			//ugly way to check if the directions is set.
			//if it stands still a strange problem occurs 
			if (track.getDirection()!=361)
			{
				expectedx = (int) cos(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
				expectedy = (int) sin(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;
				//abs should noot be neccesary btw as a root square cant be negative:
				distance = (double) abs(sqrt((expectedx-blob.getCenterOfGravity().x)*(expectedx-blob.getCenterOfGravity().x)   +   (expectedy-blob.getCenterOfGravity().y)*(expectedy-blob.getCenterOfGravity().y)));
				//temp check
				//distance = (double) track.getLastMeasurement().getCenterOfGravity().x-blob.getCenterOfGravity().x;
			} 
			else 
			{
				//this one of the problem qDebug() << "there is the problem tracks:" << tracks.size() << "blobs" << blobs.size();
			}
			//temp check
			//distance = (double) abs(track.getLastMeasurement().getCenterOfGravity().x-blob.getCenterOfGravity().x);

			if( score > maxScore ) maxScore = score;
			if( distance < minScorePrediction) 
			{
				//qDebug() << "I do still reach this";
				minScorePrediction = distance;
				//qDebug()<< "lastmeasurement" << track.getLastMeasurement().getCenterOfGravity().x << "blobx" <<blob.getCenterOfGravity().x << "V" <<track.getVelocity();
				
			}
			if( distance > maxScorePrediction) 
			{
				maxScorePrediction = distance; 
			}

        }

    }
	
	//think this will star it before even entering the munkres algortihm, munkres is ment or non-negative however
    //maxScore = maxScore - 1.0;
	//i think it is better if are to be costs to actually have all elements non-zero and non negative as is this the proper defenition of costs
	//maxScore = maxScore + 1.0;
	qDebug() << "the closest prediction distance : is " << maxScorePrediction << "maxscore is" << maxScore << "tracks:" << tracks.size() << "blobs" << blobs.size() ;
	//TODO
	//also take into account the difference between expected cog position and the actual position
	//make an estimation of where the blob should be, based on its last measurements. 
	//should add the number of not found measurements as well (with some maximum)
	//extrapolate this
	
	//combine scores with a weighting factor e.g. slow: .9 overlap and .1 direction, fast .1 overlap and .9 direction.
	//normalize on the maxscores for direction expection and for overlap score

	//so maximum overlap is bigger than the saved maximum overlap 

    // grouping
	// two tracks match a single blob and do not match any other blobs
	// two blobs match a single track

    // degrouping
	//why this and why here?
    //int blobsSize = blobs.size();

	//removed the if=0 -ed out stuff

	//in the previous version there was a slight preference for creating a new track if one was not found. 
    // matrix (track,blob);
    int matrixSize = tracks.size() > blobs.size() ? tracks.size() : blobs.size();
    const double infinity =  std::numeric_limits<double>::infinity();
    Matrix<double> m(matrixSize,matrixSize);
    for( int i = 0 ; i < m.rows() ; i++ )
    {
		//for clarity for programming noobs, i++ is slightly different as it will return ++i will returnvalue i+1, i++ wil return i before plus was added,
		for( int j = 0 ; j < m.columns(); j++ )
        {
			m(i,j) =  infinity;
		}
	}

	int factoroverlay= getFactorDirOverlap();
	int factordistance = 10-getFactorDirOverlap();
	//TODO if no overlap still plays a roll if the blob is near or further away, 
	//we could do infinity-distance, but this will make the processor slower, 
	//maybe only do this if the number of tracks and blobs do not agree and if the cog are within a certain area
    for( int i=0; i < tracks.size(); ++i )
    {
        const BlobTrack& track = tracks.at(i);
        for( int j=0; j < blobs.size(); ++j )
        {
            const Blob& blob = blobs.at(j);
            double score = (double)track.matches(blob);

			//TODO set something with maxscore+distance score
			//instead of infinty this will reduce new amount of tracks enormously hopefully!
			unsigned int timesinceupdate;
			if (m_timeSinceLastFPSCalculation.elapsed() > track.getLastUpdate()) 
			{
					timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() - track.getLastUpdate();
			}
			else //you can assume it has cycled 
			{
					timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() + m_thresholdremove - track.getLastUpdate();
			}
			if (track.getDirection()!=361)
			{
				expectedx = (int) cos(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
				expectedy = (int) sin(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;
				//abs should noot be neccesary btw as a root square cant be negative:
				distance = (double) abs(sqrt((expectedx-blob.getCenterOfGravity().x)*(expectedx-blob.getCenterOfGravity().x)   +   (expectedy-blob.getCenterOfGravity().y)*(expectedy-blob.getCenterOfGravity().y)));
				//temp check
				//distance = (double) track.getLastMeasurement().getCenterOfGravity().x-blob.getCenterOfGravity().x;
			} 

   //         if( score > 0 )
   //         {
   //             // convert matching score to costs as required for the algorithm
			//
			//	// maxScore is allready set earlier over existing tracks & incoming blobs, 
			//	// the same score values is used and maxscor of that is subtrtacted by 1 thus could be zero (or several) values resulting in score -1.
   //             //
			//	//orignal costs
			//	//m(i,j) = maxScore - score;
			//	m(i,j) = (10*factoroverlay*(maxScore - score)/maxScore + 10*factordistance*((distance-maxScorePrediction) +1))/(10);
   //         } else
			//{
			//	m(i,j) = (10*factoroverlay*(maxScore)/maxScore + 10*factordistance*((distance-maxScorePrediction) +1))/(factoroverlay+factordistance);
			//}
			//maxscoreprediction is 0 tot grotere fout e.g. 100.
			//distance => maxscore
			//(maxScore - score)/maxScore  was minimaal 1/maxscore 
			//nu best case : maxscore=score  --> 0/max = 0
			//worst case score=0 maxscore = 1 -->factoroverlay*10
			//worst case supersnelheid buiten gebied 
			//best case exact waar we verwachtten =0 of beste --> distance-minScorePrediction =0
			//worst case maxScorePrediction
			//should not divide by zero :(
			m(i,j) = 1 + (10*factoroverlay*(maxScore - score)/(maxScore +1) + 10*factordistance*((distance-minScorePrediction)/(maxScorePrediction-minScorePrediction + 1)))/(10);
        }
    }

    // run munkres matching algorithm
	// returns a matrix with zeros and -1s. Zeros identify the optimal combinations, -1 is set for the others
    Munkres munk;
    munk.solve(m);

	//false is 0 and non-zero (e.g.) 1 is true
    QVector<bool> matchedTracks( tracks.size(), 0 );
    QVector<bool> matchedBlobs( blobs.size(), 0);
	//++i & ++j 
    for( int i=0; i < tracks.size(); i++ )
    {
        BlobTrack& track = tracks[i];
        int match = -1;
		//find matches
        for( int j=0; j < blobs.size(); j++ )
        {
            if( m(i,j) == 0 )
                match = j;
        }
		//we can be certain that it is not -1 unless munkres had a bug or that there were less tracks than blobs and somehow 
        if( match != -1 )
        {
			const Blob& blob = blobs.at(match);
            //on creation lastupdate will be zero so gives an enormous velocity at first, not here btw, only in the append 
			//all these time-things will be in milliseconds i uppose as I used *1000 for fps
			unsigned int timesincelastmeasurement; 
			//set the amount of time that has past for appropriate velocity in pixels/second instead pixel/lasttwoframes
			if (m_timeSinceLastFPSCalculation.elapsed() > track.getLastUpdate()) 
			{
					timesincelastmeasurement = m_timeSinceLastFPSCalculation.elapsed() - track.getLastUpdate();
					track.setTimeSinceLastUpdate(timesincelastmeasurement);
			}
			else //you can assume it has cycled 
			{
					timesincelastmeasurement = m_timeSinceLastFPSCalculation.elapsed()+ m_thresholdremove - track.getLastUpdate();
					track.setTimeSinceLastUpdate(timesincelastmeasurement);
			}

			//TODO solve cyclecd elapsed
			track.setLastUpdate(m_timeSinceLastFPSCalculation.elapsed());
			//cycles in 0- m_thresholdremove
			matchedBlobs[match] = true;
            matchedTracks[i]    = true;
			track.addMeasurement(blob);
        }
        else
        {
			//qDebug() << "not matched " << track.getId();
			//not correct processingserial  does not have to be the number of frames processed by blobdetector i assume
            //track.notMatched( this->getProcessingSerial() );
			unsigned int timesinceupdate;
			if (m_timeSinceLastFPSCalculation.elapsed() > track.getLastUpdate()) 
			{
					timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() - track.getLastUpdate();
			}
			else //you can assume it has cycled 
			{
					timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() + m_thresholdremove - track.getLastUpdate();
			}
			track.notMatched( timesinceupdate );
		}
    }

	
//    QSet<int> usedBlobs;
//    for( int i=0; i < candidatesForTracks.size(); ++i )
//    {
//        if( candidatesForTracks[i].size() > 0 )
//        {
//            BlobTrack& track = tracks[i];
//        }
//    }

//    QVector< QVector<int> > legalCombinations( candidatesForTracks.size() );
//    for( int i=0; i < candidatesForTracks.size(); ++i )
//    {
//        const QVector<int>& candidatesForTrack = candidatesForTracks[i];
//        QVector<int> combinations;
//        for( int j=0; j < candidatesForTrack.size(); ++j )
//        {
//            if( candidatesForTrack[j] > 0 )
//            {
//                combinations.push_back(j);
//            }
//        }
//        legalCombinations[i] = combinations;
//    }

	//somewhere up there was this code  
	//int blobsSize = blobs.size();
    //used to be ++j 
	// unmatched newblobs, add them to the collection if there have been more blobs for some time
	if (blobs.size()>m_maxNrOfBlobs) 
	{
		m_biggerMaxCount++;
	} 
	else
	{
		m_maxNrOfBlobs = blobs.size();
		m_biggerMaxCount = 0;
	}
	

	if (m_biggerMaxCount>m_thresholdFramesMaxNrOfBlobs && blobs.size()>m_maxNrOfBlobs)
	{
		//slightly incorrect should have counted the actual nr of blobs 
		//m_maxNrOfBlobs = m_maxNrOfBlobs++;
		m_maxNrOfBlobs= blobs.size();

		for( int j=0; j < blobs.size(); j++ )
		{
			//if j is not matched (all are set to false and only matched to true)
			if( !matchedBlobs[j] )
			{
				Blob blob = blobs.at(j);
				//TODO catch >65535 assigned IDS will give a crash
				BlobTrack track( getNewId(), blob );
				//on creation lastupdate will be zero so gives an enormous velocity at first, not here btw, only in the append 
				track.setLastUpdate(m_timeSinceLastFPSCalculation.elapsed());
				//approximation for first measurement
				track.setTimeSinceLastUpdate((int) 1000/m_fps);
				tracks.append(track);
				//todo add time of update
			}
		}

	}

    // remove old items
//    QMutableListIterator<BlobTrack> itr( m_blobTracks );
//    while( itr.hasNext() )
//    {
//        BlobTrack& t = itr.next();
//        if( t.getState() == BlobTrackDead )
//        {
//            itr.remove();
//        }
//    }

    // a blob can match multiple tracks
    // a blob can match one track
    // - right track
    // - wrong track

    // a track can match multiple blobs
    // - right blob
    // - wrong blob

    // a track can match no blob
    // - but there is a blob

    // a track can match a single blob
    // - right blob
    // - wrong blob

    // check for collisions
}

//deal with in GUI changeable values 

int BlobTracker::getBlobSelector() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_blobSelector;
}

void BlobTracker::setBlobSelector(int num)
{
    QMutexLocker lock(m_propertyMutex);
    if( num >= 0 )
    {
        m_blobSelector = num;
    }
    emit blobSelectorChanged(m_blobSelector);
}

int BlobTracker::getFactorDirOverlap() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_factor;
}

void BlobTracker::setFactorDirOverlap(int num)
{
    QMutexLocker lock(m_propertyMutex);
    if( num >= 0 && num < 11)
    {
        m_factor = num;
    }
    emit factorDirOverlapChanged(m_factor);
}

//dead pieces of code I might need later on, 

//was placed in the loop in checking points of blob so after val==255:
//switch(in.type())
//{
//	//GRAY
//	case 0:
//		orgimageval[0] = source.at<cv::Vec3b>(k,3)[j];
//		orgimageval[1] = orgimageval[0];
//		orgimageval[2] = orgimageval[0];
//		//orgimagevector.push_back(source.at<cv::Vec3b>(k,3)[j]);
//		qDebug() << "it was gray indeed";
//		//save the points that are beyond a certain threshold within the blob
//		if (orgimageval[0]>getThreshold())
//		{
//			areapoints_t.push_back(p);
//		}
//
//		break;
//
//	//RGB
//	case 16:
//		//draw these points

//		orgimageval[0] = source.at<cv::Vec3b>(k,j)[0];
//		orgimageval[1] = source.at<cv::Vec3b>(k,j)[1];
//		orgimageval[2] = source.at<cv::Vec3b>(k,j)[2];
//		//orgimagevector.push_back(((orgimageval[0]+orgimageval[1]+orgimageval[2])/3));
//									
//		//save the points that are beyond a certain threshold within the blob
//		//also need to draw them to do 
//		if (((orgimageval[0]+orgimageval[1]+orgimageval[2])/3) >getThreshold())
//		{
//			areapoints_t.push_back(p);
//
//		}
//																																								
//		break;
//	//RGBA
//	case 24:
//		//TODO fix for e.g. RGBA channels, it will only show 3/4th of the image.
//		//val = (matin.at<cv::Vec3b>(y,x)[0]+matin.at<cv::Vec3b>(y,x)[1]+matin.at<cv::Vec3b>(y,x)[2])/3;
//		areapoints_t.push_back(p);
//		//orgimagevector.push_back(((orgimageval[0]+orgimageval[1]+orgimageval[2])/3));
//		break;
//}
//
////draw the image with the original incoming colour 
////when it is the selected blob
////	if (track.getId()==getBlobSelector())
////	{
////		qDebug() << "ID: " << (int) track.getId() << " #0 " << (int) track.getBit(0) << " #1 " << (int) track.getBit(1) << "#2: " << (int) track.getBit(2)<< " #3: " << (int) track.getBit(3) << " #4: " << (int) track.getBit(4) << " #5: " << (int) track.getBit(5);
////		cv::circle(dst3, p, 0, orgimageval, 0, 8,0 );
////	}


//// draws the selected blob in birth and process(). 
//		if ( track.getId()==getBlobSelector() )
//		{
			//draw the selected blob
			//for( int l=0; l < areapoints.size(); ++l )
			//{		
			//	//qDebug() << "xvalues" << (int) areapoints.at(l).x;
			//	orgimageval[0] = orgimagevector.at(l); orgimageval[1] = orgimagevector.at(l); orgimageval[2] = orgimagevector.at(l); 
			//	cv::Point puntje = cv::Point(areapoints.at(l).x,areapoints.at(l).y);	
			//	cv::circle(dst3, puntje, 0, orgimageval, 0, 8,0 );
			//}
			//qDebug() << "ID: " << (int) track.getId();

//		}

	//test to verify the table
	//int testcode =	getThreshold();
	//qDebug() << "value :" << testcode << "returns: " << getIDBySum(testcode);

	
//a start towards working with kmeans instead
//http://tech.dir.groups.yahoo.com/group/OpenCV/message/77421 
//double kmeans(const Mat& samples, int clusterCount, Mat& labels, TermCriteria termcrit, int attempts, int flags, Mat* centers)
//cv::termCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0 )
//cv::Mat samples = (cv::Mat_<float>(8, 1) << 31 , 2 , 10 , 11 , 25 , 27, 2, 1);
//cv::Mat labels, centers;
//cv::kmeans(dst4, 3, labels , cv::TermCriteria() ,2, cv::KMEANS_PP_CENTERS,  &centers);
//
//for(int m = 0; m < centers.rows; ++m)
//{
//	//int arg = static_cast<int> (centers.at<float>(0, m));
//	//qDebug()<< "value of points" << arg; 
//}


//TODO accept the libfreenect input style
//if( dst3.depth() == CV_16U )
//{
//	dst3.convertTo(dst3, CV_8U, 1.0 / std::numeric_limits<unsigned short>::max() );
//}


//temp we know j is supposed to be the top point
//onex = abs(cogs.at(triangleids[0]).x - cogs.at(triangleids[1]).x); 
//oney = abs(cogs.at(triangleids[0]).y - cogs.at(triangleids[1]).y);
//qone = onex*onex+oney*oney; //a
//			
//twox = abs(cogs.at(triangleids[0]).x - cogs.at(triangleids[2]).x);
//twoy = abs(cogs.at(triangleids[0]).y - cogs.at(triangleids[2]).y);
//qtwo = twox*twox+twoy*twoy;	//b	

//threex = abs(cogs.at(triangleids[1]).x - cogs.at(triangleids[2]).x);
//threey = abs(cogs.at(triangleids[1]).y - cogs.at(triangleids[2]).y);
//qthree = threex*threex+threey*threey; //c