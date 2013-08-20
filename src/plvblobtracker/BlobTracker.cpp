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
#include <QFile>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>
#include <plvcore/Util.h>
#include <set>
#include <limits>

using namespace plv;
using namespace plvblobtracker;

BlobTracker::BlobTracker() :  
	m_idCounter(0),
	m_averagePixelValue(false),
	//why here and not in init?
	m_thresholdremove(65534), //?is linked to cycling of update nr? should be linked to diethreshold? & <UINT_MAX is also important for the cycle speed of 
	m_thresholdFramesMaxNrOfBlobs(0), //used to be 2 changed to 0 for annotation //number of frames before a track is added 
	m_blobSelector(0),
	m_numberOfBlobsTracked(12),//;m_maxNrOfTrackedBlobs(12), //now set in the gui
	m_maxNrOfBlobs(0), //the number of currently tracked blobs, a track is added only if there have been more blobs for some time.
	m_biggerMaxCount(0),
	m_allowNoOverlap(false),
	m_filename_bcd("blobtrackchange.txt"),
	m_factor(90) //should be in range 0-100

{
    m_inputImage = createCvMatDataInputPin( "input image", this );
    m_inputImage->addSupportedDepth(CV_8U);
	m_inputImage->addSupportedDepth(CV_16U);
   
	m_inputImage->addSupportedChannels(3);
	m_inputImage->addSupportedChannels(1);

    m_inputBlobs = createInputPin< QList<plvblobtracker::Blob> >( "input blobs" , this );
	
	m_correctimagedirectoryboolInputPin = createInputPin<bool>("signal of imagedir readtxtfile", this);
	m_correctimagedirectoryboolInputPin->CONNECTION_OPTIONAL;
	m_outputImage = createCvMatDataOutputPin( "blob tracker image", this);
	m_outputImage2 = createCvMatDataOutputPin( "selected blob image", this);

	m_outputAnnotationNeeded = createOutputPin<bool>("annotation needed", this );
	m_outputAnnotationSituation = createOutputPin<QList<plvblobtracker::PlvBlobTrackState>>("annotation state", this );
	

	m_outputBlobTracks = createOutputPin< QList<plvblobtracker::BlobTrack> >("tracks", this);
}

BlobTracker::~BlobTracker()
{
}

bool BlobTracker::init()
{
	m_timeSinceLastFPSCalculation.start();
    //to prevent using fps before it is set, 30 is the most likely framerate for kinect and normal cameras.  
	//maybe better if it was placed at start of file?
	m_fps = 30.0;
	
	m_idPool.clear();
	//use a set of ids that can be reused --> pool containing 0...maxnr-1
	for( int i=0; i < m_numberOfBlobsTracked; i++ )
	{
		m_idPool.push_back(i);
	}

	//annotation
	m_annotationneeded = false;
	m_skipprocessbool = false;
	m_correctimagedirectorybool = true;

	return true;
}

bool BlobTracker::deinit() throw()
{
    m_blobTracks.clear();
	m_idPool.clear();
	return true;
}

bool BlobTracker::start()
{
	//qDebug() << "start"; init preceeds start
	//sometimes there are multiple starts
	m_biggerMaxCount = 0;
	m_maxNrOfBlobs = 0;
	//sometimes start is called twice to prevent any problems with that, the pool is recreated every start
	m_idPool.clear();
	//use a set of ids that can be reused --> pool containing 0...maxnr-1
	for( int i=0; i < m_numberOfBlobsTracked; i++ )
	{
		m_idPool.push_back(i);
	}

	return true;
}

bool BlobTracker::stop()
{
    m_blobTracks.clear();
	m_idPool.clear();
    return true;
}

void BlobTracker::readFile(QString filename) 
{
	m_blobchanges.clear();
	QFile inFile(filename);
	if(inFile.exists())
	{
		if ( inFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) 
		{
			QString processingserial,framechange;
			//double processingserialDouble;
			QTextStream stream( &inFile );
			QString line;
			BlobChangeData tempBCD;
			int i= 0;
			while(!stream.atEnd() )
			{
				//somehow firstline is empty
				if (i!=0)
				{
					line = stream.readLine();
					tempBCD.oldid = line.section('\t',1,1).toInt();
					tempBCD.newid = line.section('\t',2,2).toInt();
					if (m_debugstuff)
						qDebug() << "RONALD IK LEES DE CHANGEDATA IN BIJ BLOBTRACKER" << "oldid " <<  tempBCD.oldid << ", newid " << tempBCD.newid;
					//cogs will not be used:
					tempBCD.cogs.x = line.section('\t',3,3).toFloat();//int, float or double?
					tempBCD.cogs.x = line.section('\t',4,4).toFloat(); //int, float or double?
					//need to convert to std string and then to the single char
					//TODO DOESNT WORK!!
					std::string tempqt = line.section('\t',6,6).toStdString();
					tempBCD.changetype = tempqt[0];
					//qDebug() << "debug readfile blobtracker old " << tempBCD.oldid << "new " << tempBCD.newid << "changetype " <<  tempBCD.changetype;
					
					//line to allow for creating the block state fake temp bcd
					if (tempBCD.oldid >-1)
						m_blobchanges.push_back(tempBCD);
					else
					{
						if (tempBCD.newid>m_previousAnnoSerial)
						{
							//TOD check whether we will outrun scope of int in the processingserial! http://www.cplusplus.com/doc/tutorial/variables/ up to 2147483647 process loops....
							qDebug() << "set skip process in blobtracker";
							m_previousAnnoSerial = tempBCD.newid;
							m_skipprocessbool =line.section('\t',5,5).toInt();
						}
					}
					 
				}
				i++;
			}	
		}
	}
	inFile.close();
}

bool BlobTracker::process()
{
	//for anno there is a lot of debug stuff, jsut turn it of or on with a bool
	m_debugstuff = false;

	//time based measurements
	++m_numFramesSinceLastFPSCalculation;
	int elapsed = m_timeSinceLastFPSCalculation.elapsed();
	
	CvMatData image = m_inputImage->get();
    const cv::Mat& src = image;

	QList<plvblobtracker::Blob> newBlobs = m_inputBlobs->get();

	CvMatData out2 = CvMatData::create(image.width(),image.height(),16);
	cv::Mat& dst2 = out2;
    dst2 = cv::Scalar(0,0,0);

	CvMatData out = CvMatData::create(image.width(),image.height(),16);
	cv::Mat& dst = out;
    dst = cv::Scalar(0,0,0);
	unsigned int temptimesincefpscalc = m_timeSinceLastFPSCalculation.elapsed();

	//not really newtracks, but the will be send tracks.
	QList<BlobTrack> newTracks;

	//TODO ANNOTATION STATE
	m_annotationneeded = false;
	m_blobtrackstate.clear();

	//TODO RESET THE IDS based on the annotation
	//??not a constant calling tracks but actually initializing a change of track
	readFile(m_filename_bcd);
	

	if (m_skipprocessbool)
	{
		//create a try catch error!
		m_correctimagedirectorybool = m_correctimagedirectoryboolInputPin->get();
		if (m_correctimagedirectorybool)
			m_skipprocessbool = false;
	}

	if (!m_skipprocessbool)
	//if (true)
	{

		//make a temp list of blobtracks to update shit and then put it back into m_blobtracks being updated.
		QList<BlobTrack> temp_m_blobTracks;
	
		//reset ids based on annotation and set averagez of the blob before matching
		foreach( BlobTrack t, m_blobTracks )
		{
			//TODO check this logic
			bool firsttimeoldid = true;
			foreach( BlobChangeData bcd, m_blobchanges )
			{
				if (t.getId() == bcd.oldid)
				{
					if (firsttimeoldid)
					{
						t.setPID(bcd.newid);
						firsttimeoldid = false;
					}
					else
					{
						//duplicate a merged track.
						if (m_debugstuff)
							qDebug()<< "pushed back track id" << t.getId() << "with pid" << t.getPID() << "and set to " << bcd.newid << " the changetype was" << bcd.changetype;
						temp_m_blobTracks.push_back(t);
						temp_m_blobTracks.last().setPID(bcd.newid);
					}
				
				}
			}
		
			//TODO for annotation always call?
			if(true)//getAveragePixelValue()) //&& src)
			{
				if (t.getLastUpdate() >= (temptimesincefpscalc))
				{
					//qDebug() << "sourcetype" << src.type();
					switch(src.type())
					{
						//GRAY case0 
						//KINECT depth
						case 2:
						t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), src));
						break;
						//if neccesary create option for averaging RGB
					}
				}			
			}
			temp_m_blobTracks.push_back(t);
		}

		//overwrite the set of blobtracks with the updated tracks
		m_blobTracks = temp_m_blobTracks;
		
		//newtracks put into beginning

		//MATCH BLOBS
		//get is put in the beginning of this method to allow skipping alot of code when blocked
		matchBlobs(newBlobs, m_blobTracks);

		//initialisation of out2 and dst2 done in beginning of the process method
	
		//clean up dead blobs, draw all tracks and draw GUI selected track.
		for( int i=0; i < m_blobTracks.size(); i++ )
		{
			const BlobTrack& track = m_blobTracks.at(i);
		
			//the track has been updated
			if (track.getLastUpdate() >= (temptimesincefpscalc))
			{
				//draw this track on the first pin
				track.draw(dst);
				//append the track to the track output pin
				newTracks.append(track);

				//draw in GUI selected blob
				if ((track.getId() == getBlobSelector()) && (track.getState() != BlobTrackDead))
				{
					const Blob& b = track.getLastMeasurement();
					if (track.getState() == BlobTrackBirth)
					{ //only limited amount of frames
						b.drawContour(dst2,cv::Scalar(255,255,255),true);
					}
					else if (track.getState() == BlobTrackNormal)
					{
						b.drawContour(dst2,cv::Scalar(255,0,255),true);
					}
					else if (track.getState() == BlobTrackDead)
					{ //unreachable
						//b.drawContour(dst2,cv::Scalar(0,0,255),true);
						b.drawContour(dst2,cv::Scalar(155,150,0),true);
						//qDebug() << "Id off dead blob"  << (int)track.getId();
			
					}	
				}			
			
			}
			//track has not been updated or is new but not yet added to the tracks
			else
			{
				const Blob& b = track.getLastMeasurement();
				//blue? or BGR --> orange!
				b.drawContour(dst,cv::Scalar(0,100,255),true);
			
				//remove dead blobs:
				if (track.getState() == BlobTrackDead)
				{
					//b.drawContour(dst2,cv::Scalar(155,150,0),true);
					//qDebug() << "Id off dead blob"  << (int)track.getId();
					m_blobTracks.removeAt(i);
					//TODO check logic whether a gone blob should have the same ID or not. maybe push_back makes more sense.
					//m_idPool.push_front(track.getId());
					m_idPool.push_back(track.getId());

					//ANNOTATION
					m_annotationneeded = true;
					m_blobtrackstate.push_back(LessBlobs);
					if (m_debugstuff)
						qDebug() << "dead blobtrack: " << track.getId();
				}
				//draw selected blob on second output pin, indicating the current state as well, but now orange for normal as it is not updated and gray for newborns
				else if (track.getId() == getBlobSelector())
				{
					if (track.getState() == BlobTrackBirth)
					{ //only limited amount of frames
						b.drawContour(dst2,cv::Scalar(220,220,220),true);
					}
					else if (track.getState() == BlobTrackNormal)
					{
						//BGR???
						b.drawContour(dst2,cv::Scalar(0,100,255),true);
					}
				}
			}
		}
	} //end of skipprocess

	//put the image all updated tracks
	m_outputImage->put(out);
	//put the image of the selected blob
	m_outputImage2->put(out2);
	//add the track to the outputted list
	m_outputBlobTracks-> put( newTracks ) ;

	//check whether manual annotation or optimalisation might be needed:
	m_outputAnnotationNeeded-> put(m_annotationneeded);
	m_outputAnnotationSituation-> put(m_blobtrackstate);

	if(elapsed > m_thresholdremove ) //10000
	{		
		// add one so elapsed is never 0 and
		// we do not get div by 0
		m_fps = (m_numFramesSinceLastFPSCalculation * 1000) / elapsed;
		//m_fps = m_fps == -1.0f ? fps : m_fps * 0.9f + 0.1f * fps;
		qDebug() << "FPS in BlobTracker: " << (int)m_fps << "elapsed time" << elapsed;
		m_timeSinceLastFPSCalculation.restart();
		m_numFramesSinceLastFPSCalculation = 0;
	}

    return true;
}

//const cv::Mat& src = image;
unsigned int BlobTracker::averagePixelsOfBlob(Blob blob, const cv::Mat& src)
{ 
	const cv::Rect& rect = blob.getBoundingRect();

	unsigned int averagez = 0;
	unsigned int pixels = 0;
	double totalz = 0;
	double v = 0;
				
	for( int i=rect.tl().x; i < rect.br().x; i++ )
	{
		for( int j=rect.tl().y; j < rect.br().y; j++ )
		{
			v = src.at<unsigned short>(j,i);
			if ( v > 0 )
			{
				totalz = totalz + v; 
				pixels++;
			}
		}
	}
	
	if (pixels>0) 
	{
		averagez = (int) (totalz/ pixels);
	}
	else
	{
		averagez = 0;
	}
	//bitshift to get approx in metric system
	averagez = averagez >> 3;
	return averagez;
}

// match the recorded tracks to the new detected blobs using
// maximum area converage and or position-speed-direction extrapolation
// this is done using costs relative to maximum and it is called the hungarian algorithm
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

	
	//possibly it can be slightly optimized by saving at what moment maxscore is reached
	//then second loop only need to loop through the score untill then not all again
	//also distance etc does not depend on maxscores.
    double maxScore = 0;//std::numeric_limits<double>::min(); //should be zero
	double minScorePrediction = std::numeric_limits<double>::max();
	double maxScorePrediction = 0;//std::numeric_limits<double>::min(); //changed this be zero, as it should be non-negative i suppose

	float expectedx, expectedy;
	double distance;
	short tempGetFactorDirOverlap = (short) getFactorDirOverlap();

	//loop through blobs and tracks to set maximum and minimum scores to come to reasonable cost estimation
    for(int i=0; i < tracks.size(); i++ )
    {
        const BlobTrack& track = tracks.at(i);
	
		for( int j=0; j < blobs.size(); j++ )
        {
            const Blob& blob = blobs.at(j);
            double score = (double)track.matches(blob);
			if( score > maxScore ) maxScore = score;

			//could have been done neater to circumvent applying the same code again upon actual matching
			//need to use time in ms in order to circumvent rounding to 0 in int.
			if (tempGetFactorDirOverlap!=100)
			{
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
					/*expectedx = (int) cos(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
					expectedy = (int) sin(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;*/
					expectedx = (int) cos(track.getAvgDirection()-180 * PI / 180) *(track.getAvgVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
					expectedy = (int) sin(track.getAvgDirection()-180 * PI / 180) *(track.getAvgVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;
				
					distance = (double) sqrt((expectedx-blob.getCenterOfGravity().x)*(expectedx-blob.getCenterOfGravity().x)   +   (expectedy-blob.getCenterOfGravity().y)*(expectedy-blob.getCenterOfGravity().y));
				} 
				else
				{
					//should be slightly higher
					distance = ((track.getLastMeasurement().getCenterOfGravity().x)-blob.getCenterOfGravity().x)^2 + ((track.getLastMeasurement().getCenterOfGravity().y)-blob.getCenterOfGravity().y);
				}
		
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

    }
	
	//think this will star it before even entering the munkres algortihm, munkres is ment for non-negative however
    //maxScore = maxScore - 1.0;
	//i think it is better if it are to be costs to actually have all elements non-zero and non negative as is this the proper defenition of costs
	//maxScore = maxScore + 1.0;
	//qDebug() << "the closest prediction distance : is " << minScorePrediction << "maxscore is" << maxScore << "tracks:" << tracks.size() << "blobs" << blobs.size() ;
	//TODO
	//also take into account the difference between expected cog position and the actual position, instead uses a estimation using avg speed and direction., breaks down on merger due to change in direction of cog
	//make an estimation of where the blob should be, based on its last measurements. 
	//should add the number of not found measurements as well (with some maximum)
	//extrapolate this
	
	//combine scores with a weighting factor e.g. slow: .9 overlap and .1 direction, fast .1 overlap and .9 direction.
	//normalize on the maxscores for direction expection and for overlap score

	//so maximum overlap is bigger than the saved maximum overlap 

    // grouping
	// two tracks match a single blob and do not match any other blobs
	// done in second loop

	// two blobs match a single track
	// this splitting is omitted for now, can be easily done similary to the merging, however this will occur 

	//in the previous version there was a slight preference for creating a new track if one was not found.
    // matrix (track,blob);
    int matrixSize = tracks.size() > blobs.size() ? tracks.size() : blobs.size();
    const double infinity =  std::numeric_limits<double>::infinity();
    Matrix<double> m(matrixSize,matrixSize);
    for( int i = 0 ; i < m.rows() ; i++ )
    {
		//for clarity for programming noobs, i++ is slightly different as ++i, the latter will returnvalue i+1, i++ wil return i before plus was added,
		for( int j = 0 ; j < m.columns(); j++ )
        {
			m(i,j) =  infinity;
		}
	}

	int factoroverlay= getFactorDirOverlap();
	int factordistance = 100-getFactorDirOverlap();
	
	//keep track of multiple tracks to identify merged blobs
	int multipletracks = 0;
	QList<unsigned int> multipletrackslist;
	
	//reset all to not merged 9requires a non-constant and has to be done seperatly from the setting merge.
	for( int i=0; i < tracks.size(); ++i )
	{
		BlobTrack& track = tracks[i];
		track.setMerged(false);
	}
	
	//TODO if no overlap still plays a roll if the blob is near or further away, 
	//we could do infinity-distance
	//maybe only do this if the number of tracks and blobs do not agree and if the cog are within a certain area
    
	//swithed for multiple tracks in one blob
	for( int j=0; j < blobs.size(); ++j )
    {
		multipletracks = 0;
        multipletrackslist.clear();
        //swithed for multiple tracks in one blob
		for( int i=0; i < tracks.size(); ++i )
		{
			const BlobTrack& track = tracks.at(i);
            const Blob& blob = blobs.at(j);
            double score = (double)track.matches(blob);
			if (score>0)
			{
				multipletrackslist.push_back(i);
				multipletracks++;
			}

			if (tempGetFactorDirOverlap!=100)
			{
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
				if (track.getAvgDirection()!=361)
				{
					/*expectedx = (int) cos(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
					expectedy = (int) sin(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;*/
					expectedx = (int) cos(track.getAvgDirection()-180 * PI / 180) *(track.getAvgVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
					expectedy = (int) sin(track.getAvgDirection()-180 * PI / 180) *(track.getAvgVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;
					//distance squared:
					distance = (double) sqrt((expectedx-blob.getCenterOfGravity().x)*(expectedx-blob.getCenterOfGravity().x)   +   (expectedy-blob.getCenterOfGravity().y)*(expectedy-blob.getCenterOfGravity().y));
					//temp check
					//distance = (double) track.getLastMeasurement().getCenterOfGravity().x-blob.getCenterOfGravity().x;
				}
				else
				{
					//should be slightly higher
					distance = ((track.getLastMeasurement().getCenterOfGravity().x)-blob.getCenterOfGravity().x)^2 + ((track.getLastMeasurement().getCenterOfGravity().y)-blob.getCenterOfGravity().y);
				}
			}
   			//best case : maxscore=score  --> 0/(max+1) = 0 , the maxscore represents the number of overlapping pixels between the blob and the track tested.
			//worst case score=0, maxscore-score = maxscore -->score = factoroverlay*10 * maxscore/(maxscore+1)
			//worst case superspeed outside area
			//best case exact where we expected =0 or best --> distance-minScorePrediction =0
			//worst case maxScorePrediction 10*factordistance* (maxscore-minscore)/(maxscore-minscore+1)
			//should not divide by zero :(
			m(i,j) = 1 + (10*factoroverlay*(maxScore - score)/(maxScore +1) + 10*factordistance*((distance-minScorePrediction)/(maxScorePrediction-minScorePrediction + 1)))/(100);
			/*qDebug() <<  "at blobx" << blob.getCenterOfGravity().x << "and track" << track.getLastMeasurement().getCenterOfGravity().x << "ID" << track.getId();
			qDebug() <<  "influence overlap" << (10*factoroverlay*(maxScore - score)/(maxScore +1));
			qDebug() << "influence distance" << 10*factordistance*((distance-minScorePrediction)/(maxScorePrediction-minScorePrediction + 1));*/
        }
		
		if (multipletracks>1) 
		{
			for( int k=0; k < multipletrackslist.size(); ++k )
			{
				BlobTrack& track = tracks[multipletrackslist.at(k)];
				track.setMerged(true);
				//TODO ANNOTATION TOOL, maybe this is not always neccesary to annotate lets check! 
				m_annotationneeded = true;
				m_blobtrackstate.push_back(MultipleTracks);
				if (m_debugstuff)
					qDebug() <<  "multiple tracks exist for a blob in track:" << track.getId();
			}
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

		//if their are NOT less tracks than blobs; 
		//otherwise we can be certain that it is not -1 unless munkres had a bug
        if( match != -1 )
        {
			const Blob& blob = blobs.at(match);
			
			//add a test to ensure not strange flickering occurs 
			//we assume there has to be some overlap
			if(track.matches(blob)>0 || getAllowNoOverlap())
			{
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
			else //the track has been linked to a blob but is non overlapping while this is not allowed by the user
			{
				//added we also set the notupdated for a track that has been linked to an odd track as it has zero overlap!
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

				m_annotationneeded = true;
				m_blobtrackstate.push_back(NoOverlap);
				if (m_debugstuff)
					qDebug() << "track " << track.getId() << "has no overlap";
				//track.setID(m_idPool.front());
			}
        }
		//the track has not been linked to a blob
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
			//TODO directly remove?
			
			m_annotationneeded = true;
			//TODO check whether neccesary
			m_blobtrackstate.push_back(NotMatched);
			if (m_debugstuff)
				qDebug() << "track " << track.getId() << "has not matched a blob";
			//track.setID(m_idPool.front());		
		}
    }

	
	//THIS SHOULD NOT BE PART OF MATCHBLOBS METHOD
	
	// unmatched newblobs, add them to the collection if there have been more blobs for some time
	if (blobs.size()>m_maxNrOfBlobs) 
	{
		m_biggerMaxCount++;
		
		//if a track has not been updated this frame it will still ask for newid 
		//but if the number of blobs is on maximum the id is not yet given free. So it will be set to 999
		//reassign the 999 to a proper id if possible
		int id = getNewId();
		//TODO check 999 state and error stuff
		if (id!=999)
		{ 
			bool flag = true;
			for( int i=0; i < tracks.size(); ++i )
			{
				BlobTrack& track = tracks[i];
				//assign IDs to unassigned tracks
				if ((track.getId() ==999) && flag)
				{
					track.setID(id);
					track.setPID(id);
					m_idPool.removeOne(id);
					flag = false;
				}
			}
		}
	} 
	else
	{
		if (blobs.size() <m_maxNrOfBlobs)
		{
			//qDebug() << "MERGED BLOBS";
			//loop through blobs and set merged state TRUE is done in other part now.
		}
		//the tracks do not die, although not updated until after several frames e.g. 4000frames or 45 frames
		//i think the remark above is no longer true;
		m_maxNrOfBlobs = blobs.size();
		m_biggerMaxCount = 0;
	}
	
	//there have been (a number of) blobs more than the currently tracked 
	//for several frames and 
	//they still exist
	// add some of these blobs
	//TODO FOR ANNOTATION:
	if (blobs.size()>tracks.size())
	{
		m_maxNrOfBlobs= blobs.size();
		for( int j=0; j < blobs.size(); j++ )
		{
			//they are within range of GUI set trackable amount of tracks
			if( !matchedBlobs[j] && j<=getNumberOfBlobsTracked())
			{
				Blob blob = blobs.at(j);
				int id = getNewId();
				//TODO catch >65535 assigned IDS will give a crash
				// if a track has not been updated this frame it will still ask for newid but if the number of blobs 
				BlobTrack track( id, blob );
				m_idPool.removeOne(id);
				track.setLastUpdate(m_timeSinceLastFPSCalculation.elapsed());
				track.setTimeSinceLastUpdate((int) 1000/m_fps);
				//ADDED
				track.setPID(id);
				tracks.append(track);
				//ANNOTATION
				if (m_debugstuff)
					qDebug()<< "a new track" << track.getId() << "is created";
			}
		}
		//reset the trigger or only the number of blobs ?? entering would ressult in multiple blobs should then be waited the threshold number of frames again
		m_biggerMaxCount = 0;
		
		//ANNOTATION
		m_annotationneeded=true;
		m_blobtrackstate.push_back(NewBlob);
	}

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

    // only checked for collisions and not updating track if it is matched with some certainty (no ovelap) that it is the wrong combination
}

//deal with in GUI changeable values 
int BlobTracker::getBlobSelector() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_blobSelector;
}

int BlobTracker::getNumberOfBlobsTracked() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_numberOfBlobsTracked;
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
    if( num >= 0 && num < 101)
    {
        m_factor = num;
    }
    emit factorDirOverlapChanged(m_factor);
}