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

//TODO create reset boolean and a ID selector
#include "LedBlobTracker.h"
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

LedBlobTracker::LedBlobTracker() :  
	m_idCounter(0),
	m_threshold(200),
	m_blobSelector(0),
	m_minLedSize(1),
	m_maxLedSize(100),
	m_blinkTime(1250)
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
	m_outputImage4 = createCvMatDataOutputPin( "pointselection image", this);
	m_outputImage5 = createCvMatDataOutputPin( "above threshold in blob image", this);
}

LedBlobTracker::~LedBlobTracker()
{
}

bool LedBlobTracker::init()
{
	m_timeSinceLastFPSCalculation.start();
    return true;
}

bool LedBlobTracker::deinit() throw()
{
    m_blobTracks.clear();
    return true;
}

bool LedBlobTracker::start()
{
    return true;
}

bool LedBlobTracker::stop()
{
    m_blobTracks.clear();
    return true;
}

bool LedBlobTracker::process()
{
	//time based measurements
	++m_numFramesSinceLastFPSCalculation;
	int elapsed = m_timeSinceLastFPSCalculation.elapsed();

    CvMatData in = m_inputImage->get();
	cv::Mat& source = in;

    //CvMatData out = CvMatData::create(in.properties());
	CvMatData out = CvMatData::create(640,480,16);
    cv::Mat& dst = out;
    dst = cv::Scalar(0,0,0);
	
	QList<plvblobtracker::Blob> newBlobs = m_inputBlobs->get();
    matchBlobs(newBlobs, m_blobTracks);

    foreach( LedBlobTrack t, m_blobTracks )
    {
        t.draw(dst);
    }

    m_outputImage->put(out);

	CvMatData out2 = CvMatData::create(640,480,16);
	cv::Mat& dst2 = out2;
    dst2 = cv::Scalar(0,0,0);

	CvMatDataProperties props = in.properties();
    props.setNumChannels( 1 );
	props.setDepth(CV_8U);
    CvMatData out3 = CvMatData::create( props );
	cv::Mat& dst3 = out3;
    dst3 = cv::Scalar(0);

	CvMatData out4 = CvMatData::create(in.width(), in.height(), CV_8UC3 );
    cv::Mat& dst4 = out4;
	dst4 = cv::Scalar(0,0,0);

	CvMatData out5 = CvMatData::create(in.width(), in.height(), CV_8UC3 );
    cv::Mat& dst5 = out5;
	dst5 = cv::Scalar(0,0,0);

	//testing of alive versus dead blobs.
#if 0
	for( int i=0; i < m_blobTracks.size(); ++i )
    {
        const LedBlobTrack& track = m_blobTracks.at(i);
		const Blob& b = track.getLastMeasurement();
        if (track.getState() == LedBlobTrackBirth)
		{
			b.drawContour(dst2,cv::Scalar(255,255,255),true);
		}
		else if (track.getState() == LedBlobTrackNormal)
		{
			b.drawContour(dst2,cv::Scalar(255,0,255),true);
		}
		else if (track.getState() == LedBlobTrackDead)
		{
			//b.drawContour(dst2,cv::Scalar(0,0,255),true);
			
//			getLastMeasurement() const
//{
//    return d->history.last();
//}
			//BGR format
			//DOES NOT WORK AFTER RERUN due the getFrameNr usage
			//if it is really old remove it, dieThrehold is 10
			if( this->getProcessingSerial() - track.getLastMeasurement().getFrameNr() > 100)
			{
				//a choice whether to clean up the mess faster:
				//m_blobTracks.removeAt(i);
				//darkgreen shows old blobs
				b.drawContour(dst2,cv::Scalar(155,150,0),true);
			}
			else if (this->getProcessingSerial() - track.getLastMeasurement().getFrameNr() < 10)
			{
				//did not get a setstate construction working due to implemented 
				//BlobTrack tracknonconst = m_blobTracks.at(i);
				//tracknonconst.setState(BlobTrackBirth);

				//red
				b.drawContour(dst2,cv::Scalar(0,0,255),true);
				qDebug() << "Id set"  << (int)track.getId();
				
			}
			else 
			{
				//green between 10 and 100 dieing blobs
				b.drawContour(dst2,cv::Scalar(0,255,0),true);
			}
		}
		
		
    }
#endif
	
	//int id = 0;
	//non constant size
	for( int i=0; i < m_blobTracks.size(); ++i )
    {		
        //const BlobTrack& track = m_blobTracks.at(i);
		LedBlobTrack& track = m_blobTracks[i];
		//const BlobTrack& track = tracks.at(i);
		const Blob& b = track.getLastMeasurement();
		//need to reset dst2 dst 3 as we need it per blob
		dst2 = cv::Scalar(0,0,0);
		dst3 = cv::Scalar(0);

		// a pre-equisite per se, not necceraly but doesn't improve it much
		if (track.getState() == LedBlobTrackBirth)
		{	
			const cv::Rect& rect = b.getBoundingRect();
			//draw the bounding rectangle
			//cv::rectangle( dst3, rect, cv::Scalar(0,0,100) );
			//draw the topleft corner
			//cv::circle(dst3, rect.tl(), 10, cv::Scalar(255,0,0), 0, 8,0 );
			//draw the bottom right corner
			//cv::circle(dst3, rect.br(), 10, cv::Scalar(255,0,0), 0, 8,0 );
				
			//draw the selected blob filled with red colour
			b.drawContour(dst2,cv::Scalar(0,0,255),true);
								
			int val = 0;
			//cv::Scalar orgimageval;
			//orgimageval[0] = 255; orgimageval[1] = 0; orgimageval[2] = 0; 
			int orgimageval;

			cv::Point p = cv::Point(0,0);
			//save a list of all points within the contour above threshold
			std::vector< cv::Point  > areapoints_t;
			areapoints_t.clear();
			//save a list of all points within the contour
			std::vector< cv::Point  > areapoints;
			areapoints.clear();

			//assume 1 channel??
			for (int j=rect.tl().x;j < rect.br().x; ++j)
			{
					for (int k=rect.tl().y;k < rect.br().y; ++k)
					{
						p = cv::Point(j,k);
							
						//get value from the filled drawn blob
						//assumes the value is set to red.
						//if(dst2.channels() == 3)
						//{
						//	//val = dst2.at<cv::Vec3b>(k,j)[2];
						//	if (dst2.rows > 1 && dst2.cols > 1)
						//	{							
						//		val = dst2.at<cv::Vec3b>(k,j)[2];
						//	}
						//	//val = dst2.at<cv::Vec3b>(k,2)[j];
						//	//val=255;
						//}

						val = 255;
						//val = 0; //check on bug it is the bug

						// check if this point of the blob is within contour by checking to the drawn RGB image
						//probably faster than using a shit off type conversions and cv::pointPolygonTest()
						//but needs improvement non the less, this seems quite uggly
						if (val==255)
						{
							//count the number of points belonging to the blob ?why
							//area++;
								
							//the way to get a value from a mat asserts a gray-scale type of input, thus all values have equal value
							//need to build in a security check
							orgimageval = source.at<cv::Vec3b>(k,3)[j];
							//if(source.channels() == 3)
							//{
								//orgimageval = source.at<cv::Vec3b>(k,3)[j]
								//orgimageval = 0;
							//}
							//else
							//{
							//	qDebug() << "incorrect type";
							//	orgimageval = 0;
							//}
								
							//save all the points in this particular blob (not only the contour)
							areapoints.push_back(p);

							//save the points that are beyond a certain threshold within the blob
							if (orgimageval>getThreshold())
							{
								areapoints_t.push_back(p);
							}
								
							//save the values above a certain threshold in a vector containing these points per blob.
							//both using BGR and Gray values will need something similar in the future as well ?do we				
						}
					}
			} //end of checking points inside the blob, but still inside one blob from the incoming list

			//there is a difference of about 500 pixels between the drawn and actual size.
			//qDebug() << "size of contour +actual " << (int)area+b.getContour().size();
			//qDebug() << "area is " << area << "actual area" << (int) b.getSize() << "point size" << areapoints.size();
			//qDebug() << "id is: " << track.getId();
				
			//check if the expected amount of pixels are above the threhold and save this measurement
			//TODO make a changeble amount of lighten LED pixels instead
			//for test blobproducer about 291

			//some of the LEDs are either not on or are obscured, so measurement false will be added
			//this seems to be an incorrect assumption
			if(areapoints_t.size() < (getMinLedSize()))
			{
				//averaged bitset inside the track per measurement slot;
				if (elapsed < getBlinkTime() )
					track.setBits(false, 0);
				else if (elapsed <getBlinkTime()*2 )
					track.setBits(false, 1);
				else if (elapsed <getBlinkTime()*3)
					track.setBits(false, 2);
				else if (elapsed <getBlinkTime()*4)
					track.setBits(false, 3);
				else if (elapsed <getBlinkTime()*5)
					track.setBits(false, 4);
				else if (elapsed <getBlinkTime()*6)
					track.setBits(false, 5);
				else if (elapsed <getBlinkTime()*7)
				{	track.setBits(false, 6);}

				//don't set id when lights are off 
				//setTrackID(track);
			}

			//at least one LEDlight should  be detected to go further and assign direction bit code etc.
			//&& areapoints_t.size() < (getMaxLedSize() *4
			if(areapoints_t.size() > (getMinLedSize()) )
			{
				////save that the LED was on
				//if (elapsed <500 )
				//	track.setBits(true, 0);
				//else if (elapsed <1000)
				//	track.setBits(true, 1);
				//else if (elapsed <1500)
				//	track.setBits(true, 2);
				//else if (elapsed <2000)
				//	track.setBits(true, 3);
				//else if (elapsed <2500)
				//	track.setBits(true, 4);
				//else if (elapsed <3000)
				//	track.setBits(true, 5);
				//else if (elapsed <3500)
				//{	track.setBits(true, 6);}
				//
				////assign the ID according to the code of blinking LEdS it has measured.
				//setTrackID(track);
								
				//in case the blobtracker is less thrustworthy with its timing than the hungarian algorithm than only update it after each entire cycle
				//	if (elapsed>3500)
				//	{
					
						
				//qDebug() << "elapsed" << elapsed << "ID: " << (int) track.getId() << " #0 " << (int) track.getBit(0) << " #1 " << (int) track.getBit(1) << "#2: " << (int) track.getBit(2)<< " #3: " << (int) track.getBit(3) << " #4: " << (int) track.getBit(4) << " #5: " << (int) track.getBit(5) << "#6" << (int) track.getBit(6);
				//	}

				//draw points above threshold on dst3 (non output)
				for( unsigned int l=0; l < areapoints_t.size(); ++l )
				{		
					//Circle(img, center, radius, color, thickness=1, lineType=8, shift=0)
					cv::Point puntje = cv::Point(areapoints_t.at(l).x,areapoints_t.at(l).y);	
					cv::circle(dst3, puntje, 0, 255, 0, 8,0 );
					cv::circle(dst5, puntje, 0, 255, 0, 8,0 );
				}

				//find contours
				int mode = CV_RETR_LIST;
				int method = CV_CHAIN_APPROX_NONE;
				std::vector< std::vector< cv::Point > > contours;
				std::vector< cv::Vec4i > hierarchy;
				cv::findContours( dst3, contours, hierarchy, mode, method, cv::Point() );
				
				cv::Scalar red   = CV_RGB( 255, 0, 0 );
				cv::Scalar green = CV_RGB( 0, 255, 0 );
				cv::Scalar blue  = CV_RGB( 0, 0, 255 );
				cv::Scalar white = CV_RGB( 255, 255, 255 );

				std::vector< cv::Point > cogs;

				//first filter out the three most likely spots of the LEDs
				if (contours.size() > 0) //>3
				{
					for( unsigned int l=0; l<contours.size(); ++l)
					{
						const std::vector< cv::Point >& c = contours[l];
						Blob r(m_iterations,c);	
						if( r.getSize() >= getMinLedSize() &&  r.getSize() <= getMaxLedSize() )
						{
							//draw the blob on image 4 if it is the selected one might give more blobs on the pic than used
							if ( track.getId()==getBlobSelector() )
							{
								r.drawContour(dst4, blue, true);
								r.drawBoundingRect(dst4, blue); //even if not selected still draws this
							}
							cogs.push_back(cv::Point(r.getCenterOfGravity().x, r.getCenterOfGravity().y));
						}
						else
						{
							//unsuitable size
							if ( track.getId()==getBlobSelector() )
							{
								r.drawContour(dst4, red, true);
							}
						}

					}

					//TODO create a solution for 6 LED when two objects are connected somehow and the leds will be in one blob.
					//TODO come up with a more efficient algorithm
					//if more than 9 points it will become too time consuming
					if (cogs.size()>3 && cogs.size()<9) 
					{
						// make a score based on the expected 2:1 base:height shape of the triangle and select the closest
						int mintrianglescore = INT_MAX;
						int triangleids[3] = {0,0,0};

						int onex,oney,twox,twoy,threex,threey;
						double qone,qtwo,qthree,singletrianglescore;
						//unsigned int shortside;

						for(unsigned int j=0;j<cogs.size();++j)
						{
							for(unsigned int k=j+1;k<cogs.size();++k)
							{
								for(unsigned int l=k+1;l<cogs.size();++l)
								{
									//width*width aprox 640*640>int maxval
									//abs don't know if neccesary as it is ^2 anyway
									onex = abs(cogs.at(j).x - cogs.at(k).x); 
									oney = abs(cogs.at(j).y - cogs.at(k).y);
									qone = onex*onex+oney*oney; //a
									
									twox = abs(cogs.at(j).x - cogs.at(l).x);
									twoy = abs(cogs.at(j).y - cogs.at(l).y);
									qtwo = twox*twox+twoy*twoy;	//b	

									threex = abs(cogs.at(k).x - cogs.at(l).x);
									threey = abs(cogs.at(k).y - cogs.at(l).y);
									qthree = threex*threex+threey*threey; //c

									//choose the short side:
									if (qone<=qtwo && qone<=qthree )
									{
										if (qtwo>0)
										{
											//shortside = 1;
											//score: difference between the two long sides and the difference between expected base to height 
											//two long sides should equally long so q3/q2=1 , base = .5 height: half of short side height long side triangle: should be a^2+b^2=c^2 --> longside = root(17)*half shortside --> 20,62
											singletrianglescore = abs((qthree/qtwo)*10-10)+abs(((qone*42.5)/((qthree+qtwo)/2))-10) ;
											if (singletrianglescore < mintrianglescore)
											{
												mintrianglescore=singletrianglescore;
												triangleids[0] = k;
												triangleids[1] = j;
												triangleids[2] = l;
											}
										}
									}
									else if (qtwo<=qone && qtwo<=qthree )
									{
										//shortside = 2;
										
										//= 2:1 -->longside = 2,06 shortside
										if (qone>0)
										{
											singletrianglescore = abs((qthree/qone)*10-10)+abs(((qtwo*42.5)/((qthree+qone)/2))-10) ;
											if (singletrianglescore < mintrianglescore)
											{
												mintrianglescore=singletrianglescore;
												triangleids[0] = l;
												triangleids[1] = j;
												triangleids[2] = k;
											}
										}
									}
									else //if (qthree<=qone && qthree <= qtwo)
									{
										if (qone>0)
										{
											singletrianglescore = abs((qone/qtwo)*10-10)+abs(((qthree*42.5)/((qone+qtwo)/2))-10);
											if (singletrianglescore < mintrianglescore)
											{
												mintrianglescore=singletrianglescore;
												triangleids[0] = j;
												triangleids[1] = k;
												triangleids[2] = l;
											}
										} 
									}
									//((qone+qtwo)/2)>
								}
							}
						}
						std::vector< cv::Point > temptriangle; 
						temptriangle.push_back(cogs.at(triangleids[0]));	
						temptriangle.push_back(cogs.at(triangleids[1]));
						temptriangle.push_back(cogs.at(triangleids[2]));
						////cv::circle(img, pos, radius, cv::Scalar(m_colorp,m_colorp,m_colorp), CV_FILLED, CV_AA );
						cv::circle(dst4, temptriangle.at(0), 5, green, CV_FILLED, CV_AA );
						cv::circle(dst4, temptriangle.at(1), 5, green, CV_FILLED, CV_AA );
						cv::circle(dst4, temptriangle.at(2), 5, green, CV_FILLED, CV_AA );
						
						
						//always assume shortest side to be qthree so without connection to top point.
						
						//qDebug() << "q1, q2, q3" << qone << ", "<< qtwo << ", " <<  qthree << "score pt1 " << (int) abs((qone/qtwo)*10-10) << "pt 2: " << (int) abs(((qthree*42.5)/((qone+qtwo)/2))-10);
						//qDebug() << "points" << temptriangle.at(0).x << ", " <<  temptriangle.at(0).y  << "  " << temptriangle.at(1).x  << ", " << temptriangle.at(1).y << " " << temptriangle.at(2).x << ", " << temptriangle.at(2).y ;

						track.setDirection(temptriangle);
						track.setCentroidLed(temptriangle);
					}
					else if (cogs.size()==3) //only three suitable LEDS so we can simply use the found values
					{
						track.setDirection(cogs);
						track.setCentroidLed(cogs);
					}
					
					//only set on true if one above subblob was above threshold
					if (cogs.size()>0)
					{
						//save that the LED was on
						if (elapsed < getBlinkTime() )
							track.setBits(true, 0);
						else if (elapsed <getBlinkTime()*2 )
							track.setBits(true, 1);
						else if (elapsed <getBlinkTime()*3)
							track.setBits(true, 2);
						else if (elapsed <getBlinkTime()*4)
							track.setBits(true, 3);
						else if (elapsed <getBlinkTime()*5)
							track.setBits(true, 4);
						else if (elapsed <getBlinkTime()*6)
							track.setBits(true, 5);
						else if (elapsed <getBlinkTime()*7)
						{	track.setBits(true, 6);}
				
						//assign the ID according to the code of blinking LEdS it has measured.
						//setTrackID(track);
					}
					else //not enough contours of the right size
					{
						if (elapsed < getBlinkTime() )
							track.setBits(false, 0);
						else if (elapsed < getBlinkTime()*2 )
							track.setBits(false, 1);
						else if (elapsed < getBlinkTime()*3 )
							track.setBits(false, 2);
						else if (elapsed < getBlinkTime()*4 )
							track.setBits(false, 3);
						else if (elapsed < getBlinkTime()*5 )
							track.setBits(false, 4);
						else if (elapsed < getBlinkTime()*6 )
							track.setBits(false, 5);
						else if (elapsed < getBlinkTime()*7 )
							track.setBits(false, 6);
						//setTrackID(track); //only set the tracker when the lights are on
					}
				} 
				else //not enough contours should actually not happen
				{
					if (elapsed < getBlinkTime() )
						track.setBits(false, 0);
					else if (elapsed < getBlinkTime()*2 )
						track.setBits(false, 1);
					else if (elapsed < getBlinkTime()*3 )
						track.setBits(false, 2);
					else if (elapsed < getBlinkTime()*4 )
						track.setBits(false, 3);
					else if (elapsed < getBlinkTime()*5 )
						track.setBits(false, 4);
					else if (elapsed < getBlinkTime()*6 )
						track.setBits(false, 5);
					else if (elapsed < getBlinkTime()*7 )
						track.setBits(false, 6);

					//setTrackID(track);
				}
				//used to be wrong asssumption the amount of cogs (subblobcontours with size of a LED) count not the amount of contours
				/*else if (contours.size() < 3)
				{
					
				}*/
				//else //contour size ==3
				//{
				//	//for the number of different found contours create blobs, create blobs and save direction based on cog
				//	//this is most likely an inefficent to find contours after drawing the points and selecting the ones with a certain size
				//	//a kmeans is probably more desirable as we only need to know the centers, but I had some problems with implementation
				//	for( unsigned int l=0; l<contours.size(); ++l)
				//	{
				//		const std::vector< cv::Point >& c = contours[l];
				//		Blob p(m_iterations,c);
				//		//we only got three so make the best of it, 
				//		//TODO maybe add some error message to the track stating whether it is likely to be correct or not
				//		//if( b.getSize() >= getMinLedSize() &&  b.getSize() <= getMaxLedSize() )
				//		//{
				//			//draw the blob on image 4 if it is the selected one
				//			if ( track.getId()==getBlobSelector() )
				//			{
				//				p.drawContour(dst4, green, true);
				//				p.drawBoundingRect(dst4, blue);
				//			}
				//			cogs.push_back(cv::Point(p.getCenterOfGravity().x, p.getCenterOfGravity().y));

				//			//QString info = QString("pos:%1,%2 size:%3")
				//			//		.arg(b.getCenterOfGravity().x)
				//			//		.arg(b.getCenterOfGravity().y)
				//			//		.arg(b.getSize());
				//			//b.drawString(dst4, info, white);
				//		//}
				//		//else if ( track.getId()==getBlobSelector() )
				//		//{
				//			
				//		//}
				//	} //end of loop through the three contours

				//	track.setDirection(cogs);
				//	track.setCentroidLed(cogs);

				//}//end of if contoursize ==3.
				
			}//end of large enough areasize

		} //end of if track.getState() == BlobTrackBirth
		
		//loop through all the ids alive or not for seeing its last measured values
		if( (elapsed % getBlinkTime() ) < 50 ) 
		{
			qDebug() << "at " << elapsed << "the" << i << "th track:" << "has assigned ID" << track.getId() << "centroid LED: x=" << track.getCentroidLed().x << "y=" << track.getCentroidLed().y << "direction" << track.getDirection() << "blob cog x,y" << b.getCenterOfGravity().x << "," <<  b.getCenterOfGravity().y;
		}
		
		if(elapsed > getBlinkTime()*7 )
		{

			//better place would be inside the blob when on a set bet to true 
			setTrackID(track);
		}

	}//end of loop through blobtracks

	

	//get the FPS and reset the elapsed so we have a XXX (bit) second code.
	//TODO alter to specific LED blinking setting
	if(elapsed > getBlinkTime()*7 ) //10000
	{
		
		// add one so elapsed is never 0 and
		// we do not get div by 0
		m_fps = (m_numFramesSinceLastFPSCalculation * 1000) / elapsed;
		//m_fps = m_fps == -1.0f ? fps : m_fps * 0.9f + 0.1f * fps;
		qDebug() << "FPS in BlobTracker: " << (int)m_fps << "elapsed time" << elapsed;
		m_timeSinceLastFPSCalculation.restart();
		m_numFramesSinceLastFPSCalculation = 0;
		
		//loop through all the ids for test:
		for( int i=0; i < m_blobTracks.size(); ++i )
		{		
			//const BlobTrack& track = m_blobTracks.at(i);
			LedBlobTrack& track = m_blobTracks[i];
			//const BlobTrack& track = tracks.at(i);
		//	const Blob& b = track.getLastMeasurement();
		
			// a pre-equisite per se, not necceraly but doesn't improve it much
			if (track.getState() == LedBlobTrackDead)
			{
				//at least get it out of our list
				//delete(&track);
				m_blobTracks.removeAt(i);

			}
			//emit framesPerSecond(m_fps);
		}
	}

	//show image of leds in one selected blob and show all detected drawn blobs
	m_outputImage2->put(out2);
	m_outputImage4->put(out4);
	m_outputImage5->put(out5);
	
    return true;
}

//
int LedBlobTracker::getIDBySum(int sum)
{
	//is actually static so could also use a constant set on init instead of assigning it multiple times per cycle.
	////non cyclic code set for seven bits, used a simple excel function to create this counting non-zero not yet used cyclic bitshifted codes 1-255
	int idnumberset[19] = {1,3,5,7,9,11,13,15,19,21,23,27,29,31,43,47,55,63,127};
	
	//the short modulo version comparison does not apply for the maximum vlaue (all ones) as it will return zero instead
	if (sum==127)
	{
		return 18;
	}
	else {
		//using a for loop combined with a power function is quite inefficent so make a lookup 2^x array
		int lookup[7] = {1,2,4,8,16,32,64};
		for ( int tryID=0; tryID <18; ++tryID)
		{
			//limitedpower = 1; //bitshift should work??
			for( int i=0; i < 7; ++i )
			{
				//if (sum== (idnumberset.at(tryID)*limitedpower % 127))
				if (sum== (idnumberset[tryID]*lookup[i] % 127))
				{
					return tryID;
					//automatically breaks as well
				}
				//else
				//{
				//	limitedpower = limitedpower*2; //1...2^i = 2^7 in this case
				//}
			}
		}//end of for tryID

		//if it is not a proper code which is theoretical impossible if all ids are used and when programmed right than return: 
		return 99;
	}
}



//assign a id according to the bitcode combination
//TOD make a more efficient way of checking this using the bits themselves
void LedBlobTracker::setTrackID(LedBlobTrack& trackunit)
{
	//TODO make a case like 
	int summed =0;
	//summed = trackunit.getBit(0)*1+trackunit.getBit(1)*2+trackunit.getBit(2)*4+trackunit.getBit(3)*8+trackunit.getBit(4)*16+trackunit.getBit(5)*32+trackunit.getBit(6)*64;
	summed = trackunit.getBit(0)*1+trackunit.getBit(1)*2+trackunit.getBit(2)*4+trackunit.getBit(3)*8+trackunit.getBit(4)*16+trackunit.getBit(5)*32+trackunit.getBit(6)*64;
	
	int number = getIDBySum(summed);
	
	//if (number < 99)
	//{
			trackunit.setID(number);
//	}
	
}


// match the old blob to a new detected blob using
// maximum area converage
// also called the hungarian algorithm
void LedBlobTracker::matchBlobs(QList<Blob>& blobs, QList<LedBlobTrack>& tracks)
{
    //const int sizeDiffThreshold = 1000;
    //const int minBlobThreshold = 0;

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

    QVector< QList<int> > candidatesForBlobs( blobs.size() );
    QVector< QList<int> > candidatesForTracks( tracks.size() );

    double maxScore = std::numeric_limits<double>::min();
    for(int i=0; i < tracks.size(); ++i )
    {
		const LedBlobTrack& track = tracks.at(i);
        QList<int>& candidatesForTrack = candidatesForTracks[i];

        for( int j=0; j < blobs.size(); ++j )
        {
            const Blob& blob = blobs.at(j);
            QList<int>& candidatesForBlob = candidatesForBlobs[j];

            double score = (double)track.matches(blob);
            if( score > 0 )
            {
                candidatesForBlob.append(i);
                candidatesForTrack.append(j);
            }
            if( score > maxScore ) maxScore = score;
        }
    }
	//WHY this ????
    maxScore = maxScore - 1.0;

    // grouping

    // two tracks match a single blob and do not match any other blobs

    // two blobs match a single track

    // degrouping
    int blobsSize = blobs.size();
#if 0
    //QVector<int> dummies;
    for( int i=0; i < blobsSize; ++i )
    {
        const QList<int>& candidates = candidatesForBlobs.at(i);
        if( candidates.size() > 1 )
        {
            bool tracksHaveOneBlob = true;
            foreach( int trackId, candidates )
            {
                bool trackHasOneBlob = candidatesForTracks.at(trackId).size() == 1;
                tracksHaveOneBlob = tracksHaveOneBlob && trackHasOneBlob;
            }

            if( tracksHaveOneBlob )
            {
                // at least 2 blobs point to the same track
                QString out = QString("Blob %1 matches tracks ").arg(i);
                for( int j=0; j<candidates.size(); ++j )
                    out.append( QString("%1 ").arg( candidates.at(j) ) );

                qDebug() << out;

                // divide blob in two parts
                const Blob& b = blobs.at(i);

                //blobs.append(b);
                //dummies.append(blobs.size());
                //candidatesForBlobs.append(candidates);
            }
        }
    }
#endif

    // matrix (track,blob);
    int matrixSize = tracks.size() > blobs.size() ? tracks.size() : blobs.size();
    const double infinity =  std::numeric_limits<double>::infinity();
    Matrix<double> m(matrixSize,matrixSize);
    for( int i = 0 ; i < m.rows() ; i++ )
    {//temp added { for clarity, i++ is slightly different as it will return ++i will returnvalue i+1, i++ wil return i before plus was added,
		for( int j = 0 ; j < m.columns(); j++ )
        {
			m(i,j) =  infinity;
		}
	}

    for( int i=0; i < tracks.size(); ++i )
    {
        const LedBlobTrack& track = tracks.at(i);
        for( int j=0; j < blobs.size(); ++j )
        {
            const Blob& blob = blobs.at(j);
            double score = (double)track.matches(blob);
            if( score > 0 )
            {
                // convert matching score to cost 
				// don't understand

				//otherwise m remains infinity
				// maxScore is allready set earlier over existing tracks & incoming blobs, 
				// the same score values is used and maxscor of that is subtrtacted by 1 thus could be zero (or several) values resulting in score -1.
                m(i,j) = maxScore - score;
            }
        }
    }

    // run munkres matching algorithm
    Munkres munk;
    munk.solve(m);

    QVector<bool> matchedTracks( tracks.size(), 0 );
    QVector<bool> matchedBlobs( blobs.size(), 0);
    for( int i=0; i < tracks.size(); ++i )
    {
        LedBlobTrack& track = tracks[i];
        int match = -1;
        for( int j=0; j < blobs.size(); ++j )
        {
            if( m(i,j) == 0 )
                match = j;
        }
        if( match != -1 )
        {
            const Blob& blob = blobs.at(match);
            track.addMeasurement(blob);
            matchedBlobs[match] = true;
            matchedTracks[i]    = true;
        }
        else
        {
            track.notMatched( this->getProcessingSerial() );
        }
    }

//    QSet<int> usedBlobs;
//    for( int i=0; i < cadidatesForTracks.size(); ++i )
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


    // unmatched newblobs, add them to the collection
    // if they are large enough
    for( int j=0; j < blobsSize; ++j )
    {
        if( !matchedBlobs[j] )
        {
            Blob blob = blobs.at(j);
			//TODO catch >65535 assigned IDS will give a crash
            LedBlobTrack track( getNewId(), blob );
            tracks.append(track);
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
int LedBlobTracker::getBlinkTime() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_blinkTime;
}

void LedBlobTracker::setBlinkTime (int num)
{
    QMutexLocker lock(m_propertyMutex);
    if( num >= 0 )
    {
        m_blinkTime = num;
    }
    emit blinkTimeChanged(m_blinkTime);
}

int LedBlobTracker::getBlobSelector() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_blobSelector;
}

void LedBlobTracker::setBlobSelector(int num)
{
    QMutexLocker lock(m_propertyMutex);
    if( num >= 0 )
    {
        m_blobSelector = num;
    }
    emit blobSelectorChanged(m_blobSelector);
}

int LedBlobTracker::getThreshold() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_threshold;
}

void LedBlobTracker::setThreshold(int num)
{
    QMutexLocker lock(m_propertyMutex);
    if( num >= 0 )
    {
        m_threshold = num;
    }
    emit blobThresholdChanged(m_threshold);
}

void LedBlobTracker::setMinLedSize (int ledsize)
{
    QMutexLocker lock(m_propertyMutex);
    if( ledsize < getMaxLedSize() && ledsize >= 0)
    {
        m_minLedSize = ledsize;
    }
    emit minLedSizeChanged(m_minLedSize);
}

int LedBlobTracker::getMinLedSize () const
{
    QMutexLocker lock(m_propertyMutex);
    return m_minLedSize;
}

void LedBlobTracker::setMaxLedSize (int ledsize)
{
    QMutexLocker lock(m_propertyMutex);
    if( ledsize > getMinLedSize() )
    {
        m_maxLedSize = ledsize;
    }
    emit maxLedSizeChanged(m_maxLedSize);
}

int LedBlobTracker::getMaxLedSize () const
{
    QMutexLocker lock(m_propertyMutex);
    return m_maxLedSize;
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
//		//cv::circle(dst3, p, 0, cv::Scalar(0,0,255), 0, 8,0 );
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