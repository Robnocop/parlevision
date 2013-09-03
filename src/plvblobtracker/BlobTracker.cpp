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
	//why here and not in init?
	//?is linked to cycling of update nr? should be linked to diethreshold? & <UINT_MAX is also important for the cycle speed of 
	m_thresholdFramesMaxNrOfBlobs(0), //used to be 2 changed to 0 for annotation //number of frames before a track is added 
	m_blobSelector(0),
	m_numberOfBlobsTracked(12),//;m_maxNrOfTrackedBlobs(12), //now set in the gui
	m_maxNrOfBlobs(0), //the number of currently tracked blobs, a track is added only if there have been more blobs for some time.
	m_biggerMaxCount(0),
	m_filenameBCD("blobtrackchange.txt")
{
    	//cue problems, solved by having send stuff through blob tracker instead of parrallel to, number of process skips would otherwise difffer.
	m_fileNameInputPin = createInputPin<QString> ("filename of frame", this);
	m_fileNameNrInputPin = createInputPin<int> ("filenameNr of frame", this);
	m_correctimagedirectoryboolInputPin = createInputPin<bool>("signal of imagedir readtxtfile", this);
	
	m_inputVideoImage = createCvMatDataInputPin("input videos image", this );
    m_inputVideoImage->addSupportedDepth(CV_8U);
	m_inputVideoImage->addSupportedDepth(CV_16U);
   	//m_inputImage->addSupportedChannels(3);
	m_inputVideoImage->addSupportedChannels(3);

	m_inputDepthImage = createCvMatDataInputPin( "depth image", this );
    m_inputDepthImage ->addSupportedDepth(CV_8U);
	m_inputDepthImage ->addSupportedDepth(CV_16U);
   	//m_inputImage->addSupportedChannels(3);
	m_inputDepthImage->addSupportedChannels(1);

//	m_inputImage = createCvMatDataInputPin( "input image", this );
//   m_inputImage->addSupportedDepth(CV_8U);
//	m_inputImage->addSupportedDepth(CV_16U);
// 	m_inputImage->addSupportedChannels(3);
//	m_inputImage->addSupportedChannels(1);

    m_inputBlobs = createInputPin< QList<plvblobtracker::Blob> >( "input blobs" , this );
	
	//cue problems, solved by having send stuff through blob tracker instead of parrallel to, number of process skips would otherwise difffer.

	m_fileNameOutputPin  = createOutputPin<QString>("file name", this );
	m_fileNameNrOutputPin = createOutputPin<int> ("the nr for annotation", this );
	m_correctimagedirectoryboolOutputPin = createOutputPin<bool> ("a signal of textfile anno", this );
	//
	m_outputImage = createCvMatDataOutputPin( "blob tracker image", this);
	m_outputAnnotationNeeded = createOutputPin<bool>("annotation needed", this );
	m_outputAnnotationSituation = createOutputPin<QList<plvblobtracker::PlvBlobTrackState>>("annotation state", this );
	m_outputBlobTracks = createOutputPin< QList<plvblobtracker::BlobTrack> >("tracks", this);
	m_outputImage2 = createCvMatDataOutputPin( "unaltered depth image", this);

	m_outputImage2->addSupportedDepth(CV_16U);
	m_outputImage2->addSupportedChannels(1);

	

	m_outputImage3 = createCvMatDataOutputPin( "videofootage image", this);
	m_outputImage3->addSupportedDepth(CV_8U);
	m_outputImage3->addSupportedChannels(3);
}

BlobTracker::~BlobTracker()
{
}

bool BlobTracker::init()
{
    //to prevent using fps before it is set, 30 is the most likely framerate for kinect and normal cameras.  
	//maybe better if it was placed at start of file?
	
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
	m_firstloop = true;
	m_previousAnnoSerial = -1;

	//remove the blobtrackchange data
	QFile filerem(m_filenameBCD);
	if (filerem.exists())
		filerem.remove();

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
	//qDebug() << "start"; //init preceeds start
	//sometimes there are multiple starts
	m_biggerMaxCount = 0;
	m_maxNrOfBlobs = 0;
	//sometimes start is called twice to prevent any problems with that, the pool is recreated every start
	m_idPool.clear();
	//use a set of ids that can be reused --> pool containing 0...maxnr-1
	//TODO check int =0 
	for( int i=1; i < m_numberOfBlobsTracked; i++ )
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
			QTextStream stream( &inFile );
			QString line;
			BlobChangeData tempBCD;
			
			int i= 0;
			while(!stream.atEnd() )
			{
				//somehow firstline is empty so try to skip it (without succes)
				line = stream.readLine();
				if (i!=0)
				{
					//																								0				1				2									3					4					5								6
					//in trackanno QString toBlobTrackerSignal = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7")	.arg(filename)	.arg(-1)		.arg(this->getProcessingSerial())	.arg(0)				.arg(0)				.arg(m_skipprocessloop)	.arg("S");
					//in plvmouseclick QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6")			.arg(m_framenr)	.arg(bcd.oldid)	.arg(bcd.newid)						.arg(bcd.cogs.x)	.arg(bcd.cogs.y)	.arg(bcd.changetype);
					tempBCD.oldid = line.section('\t',1,1).toInt();
					tempBCD.newid = line.section('\t',2,2).toInt();
					//cogs will not be used:
					tempBCD.cogs.x = line.section('\t',3,3).toFloat();//int, float or double?
					tempBCD.cogs.x = line.section('\t',4,4).toFloat(); //int, float or double?
					//tempBCD.changetype = line.section('\t',6,6);
					//QString teststring = line.section('\t',5,5).toAscii();
					
					std::string tempqt = line.section('\t',5,5).toStdString();
					QString teststring = QString::fromStdString(tempqt);
					//tempBCD.changetype = tempqt[0];
					//qDebug() << "tempqt line of blobtracker changes" << teststring; 
					if (teststring ==" R" || teststring ==  "R" )
					{
						if (m_debugstuff)
							qDebug()<<"Removed in blobtracker  indeed" << tempBCD.oldid << "new " << tempBCD.newid;
						tempBCD.changetype = "R"; //? would this help?
					}
					else if (teststring == " A" || teststring == "A" )
					{
						if (m_debugstuff)
							qDebug()<<"Added in blobtracker indeed" << tempBCD.oldid << "new " << tempBCD.newid;
						tempBCD.changetype = "A";
					}
					else if (teststring == " I" || teststring == "I" )
					{
						if (m_debugstuff)
							qDebug()<<"Removed all in blobtracker from " << tempBCD.oldid << " and indeed added" << tempBCD.newid;
						tempBCD.changetype = "I";
					}
					else
					{
						//TODO teststring is 1, might be a problem!!!
						//qDebug() << "teststring is " << teststring;
					}
					//else if(teststring == " S")
						//qDebug()<<"Stateofchangeline";
					//else
						//qDebug()<<"another";
					//qDebug() << "debug readfile blobtracker old " << tempBCD.oldid << "new " << tempBCD.newid << "changetype " <<  tempBCD.changetype;
					
					//line to allow for creating the block state fake temp bcd
					//QString toBlobTrackerSignal = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7").arg(filename).arg(-1).arg(this->getProcessingSerial()).arg(0).arg(0).arg(m_skipprocessloop).arg("S");
					
					if (tempBCD.oldid >-1)
					{
						m_blobchanges.push_back(tempBCD);
						//qDebug() << "I do put in tempBCD" << tempBCD.oldid;
					}
					else
					{
						//prevent one block to keep on setting skip
						//TODO CHECK
						if ( (!(tempBCD.changetype == "N" || tempBCD.changetype == " N") &&  tempBCD.newid>m_previousAnnoSerial))
						{
							//TODO check whether we will outrun scope of int in the processingserial! http://www.cplusplus.com/doc/tutorial/variables/ up to 2147483647 process loops....
							//qDebug() << "set skip process in blobtracker";
							m_previousAnnoSerial = tempBCD.newid;
							if (line.section('\t',5,5).toInt() == 1) 
								m_skipprocessbool = true;
							else
								m_skipprocessbool = false;

							//qDebug() << "end of processingsignal thingy";
						}
						else
						{
							qDebug() << "recognised that you succesfully manually added a nonexisting track";
						}
					}				 
				}
				i++;
			}	
		}
		//only if infile exist it can be closed
		inFile.close();
	}
	QFile filerem(m_filenameBCD);
	if (filerem.exists())
		filerem.remove();
}

bool BlobTracker::process()
{
	//cue problems, with naming that is not used further on
	QString nameOfFile = m_fileNameInputPin->get();
	int numberOfFile = m_fileNameNrInputPin->get();
	
	//for anno there is a lot of debug stuff, jsut turn it of or on with a bool
	m_debugstuff = true;
	
//	CvMatData image = m_inputImage->get();
//  const cv::Mat& src = image;

	CvMatData imageDepth = m_inputDepthImage->get();
    const cv::Mat& srcDepth = imageDepth;

	CvMatData imageVideo = m_inputVideoImage->get();
    const cv::Mat& srcVideo = imageVideo;

	QList<plvblobtracker::Blob> newBlobs = m_inputBlobs->get();

	//CvMatData out2 = CvMatData::create(image.width(),image.height(),16);
	CvMatData out2 = CvMatData::create(imageDepth.width(),imageDepth.height(),imageDepth.type());
	cv::Mat& dst2 = out2;
    dst2 = cv::Scalar(0,0,0);

	//CvMatData out = CvMatData::create(image.width(),image.height(),16);
	//16?? should be 
	CvMatData out = CvMatData::create(imageDepth.width(),imageDepth.height(),16);
	cv::Mat& dst = out;
    dst = cv::Scalar(0,0,0);
	
	//not really newtracks, but the will be send tracks.
	QList<BlobTrack> newTracks;

	m_annotationneeded = false;
	m_blobtrackstate.clear();
	readFile(m_filenameBCD);
	
	//somehow although new tracks are created or deleted annoneeded is set to no anno needed.
	//simply override annoneeded if the number of tracks changes as well
	int prevnumberofm_blobTracks = m_blobTracks.size();

	//make a temp list of blobtracks to update shit and then put it back into m_blobtracks being updated.
	QList<BlobTrack> temp_m_blobTracks;
	//this foreach resets ids based on annotation and set averagez of the blob before matching
	//TODO actually reimplement the setaverageZ value
	foreach( BlobTrack t, m_blobTracks )
	{
		//if (m_debugstuff)
			//qDebug() << "check all blobs from m_blobtracks"; 
		
		//bool firsttimeoldid = true; //not needed in an add remove structure.
		foreach( BlobChangeData bcd, m_blobchanges )
		{
			if (t.getId() == bcd.oldid)
			{
				if (bcd.changetype == "A" || bcd.changetype == " A") //might be dependent on OS or something,  strange space in there
				{
					if(m_debugstuff)
						qDebug() << "start adding pid in blobtracker" << bcd.newid;
					t.addPID(bcd.newid);
				}
				else if (bcd.changetype == "R" || bcd.changetype == " R")
				{
					if(m_debugstuff)
						qDebug() << "start removing pid in blobtracker" << bcd.newid;
					t.removePID(bcd.newid);
				}
				else if (bcd.changetype == "I" || bcd.changetype == " I")
				{
					if(m_debugstuff)
						qDebug() << "start removing pids in blobtracker from" << bcd.oldid << " and to add pid" << bcd.newid;
					t.removeAllPIDs(); 
					t.addPID(bcd.newid);
				}
				else if (bcd.changetype == "S" ||bcd.changetype == " S" || bcd.changetype == "")
				{
					int toskipthis= 0;
				}
				else
				{
					qDebug() << "ERROR in blobtracker changetype is not defined properly";
				}
	
			}
		}
	
		temp_m_blobTracks.push_back(t);
	}
	//qDebug() << "temp size of blobtracks" << temp_m_blobTracks.size();

	//overwrite the set of blobtracks with the updated tracks
	m_blobTracks = temp_m_blobTracks;

	m_correctimagedirectorybool = m_correctimagedirectoryboolInputPin->get();
	
	if (m_skipprocessbool)
	{
		if(m_debugstuff)
			qDebug() << "I will skip process in blobtracker unless i get a processsignal from imagedir";
		if (m_correctimagedirectorybool)
		{
			m_skipprocessbool = false;
			qDebug() << "I won't skip afterall due to the sprocessbool";
		}
	}

	
	if (!m_skipprocessbool)
	{
		//nonetheless runs this process 9 times after keeping a frame still!
		
		//MATCH BLOBS
		//get is put in the beginning of this method to allow skipping alot of code when blocked
		
		matchBlobs(newBlobs, m_blobTracks);

		//for annotation dont make use of dead etc based on frametime but on not matched time
		//clean up dead blobs, draw all tracks and draw GUI selected track.
	//requires constant that wont work, no using t instead of track.
	//	for( int i=0; i < m_blobTracks.size(); i++ )
		foreach( BlobTrack t, m_blobTracks )
		{
			//draw the blob, the track etc but not the infoline
			t.draw(dst);
			
			//removed some code here, put at end of file
		}
		//override the text and set this seperatly to minimuze overlapping stuff
		foreach( BlobTrack t, m_blobTracks )
		{
			//setaveragepixel now also depends on whether the loop is skipped or not.
			QString r = "-";
			switch(srcDepth.type())
			{

				case CV_8U:  r = "8U"; t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), srcDepth)); break;
				case CV_8S:  r = "8S"; t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), srcDepth)); break;
				case CV_16U: r = "16U";	t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), srcDepth)); break;
				case CV_16S: r = "16S"; t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), srcDepth)); break;
				case CV_32S: r = "32S"; t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), srcDepth)); break;
				case CV_32F: r = "32F"; t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), srcDepth)); break;
				case CV_64F: r = "64F"; t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), srcDepth)); break;
				case 16:	 r = "Unknow 16"; t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), srcDepth)); break;
				default:     r = "User"; t.setAveragePixelValue(averagePixelsOfBlob(t.getLastMeasurement(), srcDepth)); break;
				//GRAY case0 
				//KINECT depth case 2 png cas 16
			}
			
			t.drawInfo(dst);
			//the track has been updated, no longer is an option in annotation style
			//append the track to the track output pin
			newTracks.append(t);
		}
	} //end of skipprocess
	else {
		//if(m_debugstuff)
			//qDebug() << "skipped the process in blobtracker";
	}
	
	//the firstframe should always be annotated although it might be ok with respect to normal standards.
	if (m_firstloop)
	{
		m_firstloop = false;
		m_annotationneeded = true;
	}

	//TODO check some particular situations although a track is added or remove annotation is needed is set to no.
	if (prevnumberofm_blobTracks != newTracks.size())
	{
		if (!m_annotationneeded)
		{
			qDebug() << "somewhere in blobtracker a track is added or removed  in an unexpected manor";
		}
		m_annotationneeded = true;
	}

	//put the image all updated tracks
	m_outputImage->put(out);
	//put the image of the selected blob
	//m_outputImage2->put(out2);
	m_outputImage2->put(srcDepth);

	m_outputImage3->put(srcVideo);

	//add the track to the outputted list
	m_outputBlobTracks-> put( newTracks );

	//check whether manual annotation or optimalisation might be needed:
	m_outputAnnotationNeeded-> put(m_annotationneeded);
	//could be empty.. problem?
	m_outputAnnotationSituation-> put(m_blobtrackstate);

	//cue problems
	m_fileNameOutputPin->put(nameOfFile);
    m_fileNameNrOutputPin->put(numberOfFile);
	m_correctimagedirectoryboolOutputPin-> put( m_correctimagedirectorybool );
	return true;
}

//TODO still not working!
//const cv::Mat& src = image;
unsigned int BlobTracker::averagePixelsOfBlob(Blob blob, const cv::Mat& srcDepthIm)
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
			v = srcDepthIm.at<unsigned short>(j,i);
			if ( v > 0 )
			{
				totalz = totalz + v; 
				pixels++;
			}
		}
	}
	
	if (pixels>0) 
	{
		//qDebug() << "blob at x" << blob.getCenterOfGravity().x << " and y " <<blob.getCenterOfGravity().y << "has totalz" << totalz;
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
// maximum area converage
// this could have been done using costs relative to maximum which is called the hungarian algorithm/munk
//however a simple gready algorithm might be easier to bugfix and should suffice for our purposes
void BlobTracker::matchBlobs(QList<Blob>& blobs, QList<BlobTrack>& tracks)
{
    #define PI 3.14159265
	//if (m_debugstuff)
		//qDebug() << " I do enter match blobs";
	
	int matrixSize = tracks.size() > blobs.size() ? tracks.size() : blobs.size();
	//if(m_debugstuff)
		//qDebug()<< "matrixsize " << matrixSize;

    const double infinity =  std::numeric_limits<double>::infinity();
    Matrix<double> m(matrixSize,matrixSize);
    for( int i = 0 ; i < m.rows() ; i++ )
    {
		for( int j = 0 ; j < m.columns(); j++ )
        {
			//m(i,j) =  infinity;
			//NOMUNK
			//check would make a shit of a difference as it is a square matrix
			m(i,j) = -1;
		}
	}

	//keep track of multiple tracks to identify merged blobs
	int multipletracks = 0;
	QList<unsigned int> multipletrackslist;
	
	//reset all to not merged requires a non-constant and has to be done seperatly from the setting merge.
	for( int i=0; i < tracks.size(); ++i )
	{
		BlobTrack& track = tracks[i];
		track.setMerged(false);
	}

	//only use overlap as a track indicator don't use direction or distance
    double maxScore = 0;//std::numeric_limits<double>::min(); //should be zero

	//loop through blobs and tracks to set maximum score to come to reasonable cost estimation
    //CHECK temporary just set the best linked track here to -1. 
	for(int i=0; i < tracks.size(); i++ )
    {
        const BlobTrack& track = tracks.at(i);
	
		for( int j=0; j < blobs.size(); j++ )
        {
            const Blob& blob = blobs.at(j);
			//save the overlap between tracks (of previous frame) and blobs 
            double score = (double)track.matches(blob); 
			if( score > maxScore ) maxScore = score;
			//removed the direction stuf and min and maxscore prediction temporary at the bottom of this file commented
        }
    }

	//only one track is set to a blob, but multiple tracks might have a shared historyoverlap indicating an overlap&,merge quite clearly
	//switched for multiple tracks in one blob
	//m(i,j) == -1
	int besttrackcandidate = -1;
	bool isalreadyused = false;

	//TODO improve by looping blobs ordered on size, might improve the track linking process.
	for( int j=0; j < blobs.size(); j++ )
    {
		multipletracks = 0;
        multipletrackslist.clear();
    
		//nomunkres tempsolution
		maxScore = 0;
		besttrackcandidate = -1;
		
		const Blob& blob = blobs.at(j);
		//swithed for multiple tracks in one blob 
		for( int i=0; i < tracks.size(); i++ )
		{
			const BlobTrack& track = tracks.at(i);
            
            double score = (double)track.matches(blob);
			if (score>0)
			{
				multipletrackslist.push_back(i);
				multipletracks++;
				//the number of blobs and tracks with overlap is too LOW!!
			}
			
			//TODO add allready used
			isalreadyused = false;
			int prevMaxScore = maxScore;
			if (score>maxScore) //so also >0!
			{
				for (int k=0;k<blobs.size();k++)
				{
					if (m(i,k)==0)
						isalreadyused = true;
				}
				if (!isalreadyused)
				{
					besttrackcandidate = i;
					maxScore = score;
				}
				else
				{
					//add a check whether the overlap of this blob is bigger than the other. 

					//not needed just for clarity
					maxScore = prevMaxScore;
				}
			}

			//removed a piece of code using direction, put it in the bottom of the file

   			//best case : maxscore==score  --> 0/(max+1) = 0 , the maxscore represents the number of overlapping pixels between the blob and the track tested.
			//worst case score=0, 
			//worst case superspeed outside area
			//best case exact, exact overlap with previous
			//should not divide by zero :(
			double scoretoset = score;
			if (scoretoset == 0)
				scoretoset = infinity;
			else
				scoretoset = maxScore - score;
			
			//m(i,j) = (maxScore - score);
			
			//nomunkres temp solution
			//commented m(i,j) = scoretoset;
		}
		
		if (multipletracks>1) 
		{
			for( int k=0; k < multipletrackslist.size(); ++k )
			{
				//DEBUG:
				if (multipletrackslist.at(k)<tracks.size())
				{
					//if a blob has multiple tracks with overlap than it these tracks have probably been merged.
					BlobTrack& track = tracks[multipletrackslist.at(k)];
					track.setMerged(true);
					if (m_debugstuff)
						qDebug() <<  "multiple tracks exist that have overlap with one blob, probably signifies a merged blob" << track.getId();
				}
			}
			//it is neccesary to annotate this situation.
			m_annotationneeded = true;
			m_blobtrackstate.push_back(MultipleTracks);
		}
		
		//temp nomunkres solution
		isalreadyused = false;
		if (besttrackcandidate>-1 && besttrackcandidate < tracks.size())
		{
			for (int k=0;k<blobs.size();k++)
			{
				if (m(besttrackcandidate,k)==0)
					isalreadyused = true;
			}

			if (!isalreadyused)
			{
				m(besttrackcandidate,j) = 0;
				//qDebug() << "linked blob" << j << " to track " << tracks.at(besttrackcandidate).getId();
			}
			else
			{
				//supposed to be unreachable
				qDebug()<< "at blob" << j << "already used track " << besttrackcandidate << "and multipletracks value" << multipletracks;
				m_annotationneeded = true;
			}
		}

		/*if (m_debugstuff)
			qDebug() << "currently at blob j" << j;*/
		
    }

	
    // run munkres matching algorithm
	// i dont trust result of di

	// returns a matrix with zeros and -1s. Zeros identify the optimal combinations, -1 is set for the others
	//check whether i and j are not mixed up or m!
    //tracks,blobs

	//checking whether the problems are in munkres, probably not...
	//Munkres munk;
	////if 0 blobs perhaps it crashes.
	//if (blobs.size()>0)
	//	munk.solve(m);
	//

	//create to "empty matched" vectors
	QVector<bool> matchedTracks( tracks.size(), 0 );
    QVector<bool> matchedBlobs( blobs.size(), 0);
	
	//++i & ++j 
	QList<int> notmatchedabloblist;
    for( int i=0; i < tracks.size(); i++ )
    {
        BlobTrack& track = tracks[i]; //almost the same as.at ; .at never uses a deep copy
        int match = -1;
		//find matches
        int trackmunkcheck = 0; 
        for( int j=0; j < blobs.size(); j++ )
        {
			//m is track,blob
		    if( m(i,j) == 0 )
            {
				match = j;
				trackmunkcheck++;
			}
		}
		if (trackmunkcheck>1)
		{
			qDebug() << "ERROR in blobtracker: Houston we got a major problem multiple tracks have been assigned to one blob";
			m_annotationneeded = true;
		}
        

		//if there are NOT less tracks than blobs; 
		if (match == -1 && tracks.size()<=blobs.size())
		{
			//qDebug() << "BUG IN MUNKRES IMPLEMENTATION";
		}
		else if (match== -1)
		{
			//basically this track has not been matched to a blob with the munkres algorithm
			//qDebug() << "a -1 == no match with less blobs";
		}
		
		//otherwise we can be certain that it is not -1 for a match
        if( match != -1 )
        {
			const Blob& blob = blobs.at(match);
			
			//add a test to ensure not strange flickering occurs 
			//we assume there has to be some overlap
			if(track.matches(blob)>0) //
			{
				//perhaps reimplement a framebased history 
				//track.setTimeSinceLastUpdate(timesincelastmeasurement);
				//???
				track.setState(BlobTrackNormal);

				//match == j , the blob that has been matched
				//blob j/match has been matched to a track
				matchedBlobs[match] = true;
				//track i has beeen matched to a blob
				matchedTracks[i]    = true;
				track.addMeasurement(blob);
			}
			else //the track has been linked to a blob but is non overlapping while this makes no sense, make a new track instead.
			{
				//TODO reimplement
				//track.notMatched( timesinceupdate );
				
				//a new track should be created for this blob instead of using this old track again;
				track.setState(BlobTrackDead);

				m_annotationneeded = true;
				m_blobtrackstate.push_back(NoOverlap);
				if (m_debugstuff)
					qDebug() << "ERROR in blobtracker track " << track.getId() << "has no overlap this is not supposed to be allowed";
				//track.setID(m_idPool.front());
			}
        }
		//the track has not been linked to a blob
        else
        {
			//TODO reimplement
			//track.notMatched( timesinceupdate );
			track.setState(BlobTrackDead);

			m_annotationneeded = true;
			//TODO check whether neccesary
			m_blobtrackstate.push_back(NotMatched);
			notmatchedabloblist.push_back(track.getId());
		//	if (m_debugstuff)
		//		qDebug() << "track " << track.getId() << "has not matched a blob";
			//track.setID(m_idPool.front());		
		}
    }
	
	if (m_debugstuff && notmatchedabloblist.size()>0)
	{
		QString thenomatchedblobs = "";
		QString toappendnotmatchedblobs = "";
		foreach(int numberoftrack, notmatchedabloblist)
		{
			toappendnotmatchedblobs = QString("%1 ,").arg(numberoftrack);
			thenomatchedblobs.append(toappendnotmatchedblobs);
			//toappendnotmatchedblobs  = QString
		}
		qDebug() << "there are nonmatchedblobs" << thenomatchedblobs;
	}
	//CREATE TRACKS FOR ALL THE BLOBS THAT HAVE NOT BEEN MATCHED, only then later on remove th tracks in order to keep the same indices etc.
	// unmatched blobs, add them to the collection and create tracks for them
	for( int i=0; i < blobs.size(); i++ )
    {
		if(!matchedBlobs[i])
		{
			Blob& blob = blobs[i];
			int id = getNewId();
			
			//TODO check 999 state and error stuff
			if (id!=999)
			{ 
				BlobTrack newTrackForNonMatchedBlob( id, blob );
				//seems redundant?
				newTrackForNonMatchedBlob.setID(id);
				newTrackForNonMatchedBlob.setPID(id);
				newTrackForNonMatchedBlob.addPID(id);
				//assume it is a new track although it might have been a mismatched one as well
				newTrackForNonMatchedBlob.setState(BlobTrackBirth);
				m_idPool.removeOne(id);
				tracks.append(newTrackForNonMatchedBlob);
				m_blobtrackstate.push_back(NewBlob);
			}
			//a new track annotation is needed;
			m_annotationneeded = true;
		}
	} 

	//qDebug() << "tracks size in match before removing deadblobs " << tracks.size();

	//REMOVE DEADTRACKS
	QList<BlobTrack>::iterator it = tracks.begin();
	bool outputdebugannoneeded = false;
	while (it != tracks.end()) {
		BlobTrack track = *it;
		if (track.getState() == BlobTrackDead)
		{
			it = tracks.erase(it);
			m_idPool.push_back(track.getId());
			outputdebugannoneeded = true;
			
			m_blobtrackstate.push_back(NotMatched);
			m_annotationneeded = true;
		}
		else
			++it;
	}
	if (outputdebugannoneeded)
		qDebug() << "a track has been removed annotation is needed";

	//check for debug if there are more ttracks than blobs, this should be impossible at this point
	if (tracks.size()>blobs.size())
	{
		qDebug() << "ERROR in blobtracker still more tracks  " <<  tracks.size() << "than blobs!" << blobs.size();
		m_annotationneeded = true;
	}

	if (tracks.size()<blobs.size())
	{
		qDebug() << "ERROR in blobtracker still less tracks than blobs!";
		m_annotationneeded = true;
	}


    // only checked for collisions and not updating track if it is matched with some certainty (almost no ovelap) that it is the wrong combination
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



			//if (tempGetFactorDirOverlap!=100)
			//{
			//	unsigned int timesinceupdate;
			//	if (m_timeSinceLastFPSCalculation.elapsed() > track.getLastUpdate()) 
			//	{
			//			timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() - track.getLastUpdate();
			//	}
			//	else //you can assume it has cycled 
			//	{
			//			timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() + m_thresholdremove - track.getLastUpdate();
			//	}
			//
			//	//ugly way to check if the directions is set.
			//	//if it stands still a strange problem occurs 
			//	if (track.getDirection()!=361)
			//	{
			//		/*expectedx = (int) cos(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
			//		expectedy = (int) sin(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;*/
			//		expectedx = (int) cos(track.getAvgDirection()-180 * PI / 180) *(track.getAvgVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
			//		expectedy = (int) sin(track.getAvgDirection()-180 * PI / 180) *(track.getAvgVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;
			//	
			//		distance = (double) sqrt((expectedx-blob.getCenterOfGravity().x)*(expectedx-blob.getCenterOfGravity().x)   +   (expectedy-blob.getCenterOfGravity().y)*(expectedy-blob.getCenterOfGravity().y));
			//	} 
			//	else
			//	{
			//		//should be slightly higher
			//		distance = ((track.getLastMeasurement().getCenterOfGravity().x)-blob.getCenterOfGravity().x)^2 + ((track.getLastMeasurement().getCenterOfGravity().y)-blob.getCenterOfGravity().y);
			//	}
		
			//	if( distance < minScorePrediction) 
			//	{
			//		//qDebug() << "I do still reach this";
			//		minScorePrediction = distance;
			//		//qDebug()<< "lastmeasurement" << track.getLastMeasurement().getCenterOfGravity().x << "blobx" <<blob.getCenterOfGravity().x << "V" <<track.getVelocity();
			//	
			//	}
			//	if( distance > maxScorePrediction) 
			//	{
			//		maxScorePrediction = distance; 
			//	}
			//}

//BLOB Options
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



//if (tempGetFactorDirOverlap!=100)
//{
//	//TODO set something with maxscore+distance score
//	//instead of infinty this will reduce new amount of tracks enormously hopefully!
//	unsigned int timesinceupdate;
//	if (m_timeSinceLastFPSCalculation.elapsed() > track.getLastUpdate()) 
//	{
//			timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() - track.getLastUpdate();
//	}
//	else //you can assume it has cycled 
//	{
//			timesinceupdate = m_timeSinceLastFPSCalculation.elapsed() + m_thresholdremove - track.getLastUpdate();
//	}
//	if (track.getAvgDirection()!=361)
//	{
//		/*expectedx = (int) cos(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
//		expectedy = (int) sin(track.getDirection()-180 * PI / 180) *(track.getVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;*/
//		expectedx = (int) cos(track.getAvgDirection()-180 * PI / 180) *(track.getAvgVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().x ;
//		expectedy = (int) sin(track.getAvgDirection()-180 * PI / 180) *(track.getAvgVelocity()*timesinceupdate/1000) + track.getLastMeasurement().getCenterOfGravity().y ;
//		//distance squared:
//		distance = (double) sqrt((expectedx-blob.getCenterOfGravity().x)*(expectedx-blob.getCenterOfGravity().x)   +   (expectedy-blob.getCenterOfGravity().y)*(expectedy-blob.getCenterOfGravity().y));
//		//temp check
//		//distance = (double) track.getLastMeasurement().getCenterOfGravity().x-blob.getCenterOfGravity().x;
//	}
//	else
//	{
//		//should be slightly higher
//		distance = ((track.getLastMeasurement().getCenterOfGravity().x)-blob.getCenterOfGravity().x)^2 + ((track.getLastMeasurement().getCenterOfGravity().y)-blob.getCenterOfGravity().y);
//	}
//}



//OLD munkres remarks
	//think this will star it before even entering the munkres algortihm, munkres is ment for non-negative however
    //maxScore = maxScore - 1.0;
	//i think it is better if it are to be costs to actually have all elements non-zero and non negative as is this the proper definition of costs
	//maxScore = maxScore + 1.0;
	//"the closest prediction distance : is " << minScorePrediction << "maxscore is" << maxScore << "tracks:" << tracks.size() << "blobs" << blobs.size() ;
	
	//perhaps
	//also take into account the difference between expected cog position and the actual position, 
	
	// grouping
	// two tracks match a single blob and do not match any other blobs
	// done in second loop

	// two blobs match a single track
	// this splitting is omitted for now, can be easily done similary to the merging, however this will occur 



//if ((t.getId() == getBlobSelector()) && (t.getState() != BlobTrackDead))
//			{
//				if (t.getState() == BlobTrackBirth)
//				{ 
//					//only limited amount of frames
//					b.drawContour(dst2,cv::Scalar(255,255,255),true);
//				}
//				else if (t.getState() == BlobTrackNormal)
//				{
//					b.drawContour(dst2,cv::Scalar(255,0,255),true);
//				}
//				else if (t.getState() == BlobTrackDead)
//				{ //unreachable
//					//b.drawContour(dst2,cv::Scalar(0,0,255),true);
//					b.drawContour(dst2,cv::Scalar(155,150,0),true);
//					//qDebug() << "Id off dead blob"  << (int)track.getId();
//			
//				}	
//			}	


////get the blob of the track to draw and stuff!!!
//const Blob& b = t.getLastMeasurement();

//removed the geblobselector bs as it is not needed to draw the in GUI selected blob
//instead use it for annotation : merged, birth and normal,
//TODO? make a colour that signal a new or unmatched track
//draw selected blob on second output pin, indicating the current state as well, but now orange for normal as it is not updated and gray for newborns
//if (t.getState() == BlobTrackBirth)
//{ //only limited amount of frames
//	//blue is new
//	b.drawContour(dst,cv::Scalar(255,0,0),true);
//}
//else if( t.getMerged())
//{
//	//red is wrong
//	b.drawContour(dst,cv::Scalar(0,0,255),true);
//}
//else if (t.getState() == BlobTrackNormal)
//{
//	//normal is ok
//	//b.drawContour(dst2,cv::Scalar(0,255,0),true);
//}
				
//remove dead blobs is done in match now