#include "TrackAnnotation.h"

using namespace plv;
using namespace plvblobtracker;

#include <plvcore/CvMatData.h>
#include <QFile>
#include <QDebug>
#include <QtGui>

#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QHBoxLayout>
#include <QScopedPointer>

#include "PlvMouseclick.h"

TrackAnnotation::TrackAnnotation() :
	m_saveToFile(false),
	//TODO VERY DANGEROUS!!! set to a new file name automatically or make the user set the filename
	//m_filename("annotationlog.txt"),
	m_filenameLog("annotationlog.txt"),
	m_filenameFrameNr("framenr.txt"), //sets the framenumber in the imagedirectoryproducer and loads the last values in order to be able to go back in a properly tracked file
	m_filenameBCD("blobtrackchange.txt"), //will set the blobtrack changed data, switches etc. , to be loaded into the tracker to swithc the ids etc for once and all to prevent restoring same mistake over and over
	m_popUpExists(false),
	m_legacyFormat(true),
	m_interesting(true),
	debugging(false)
{	
	//CHANGED
	m_fileNameInputPin = createInputPin<QString> ("filename of frame", this);
	m_fileNameNrInputPin = createInputPin<int> ("filenameNr of frame", this);
	m_correctimagedirectoryboolInputPin = createInputPin<bool>("signal of imagedir readtxtfile", this);
	m_inputImage = createCvMatDataInputPin( "blob track image", this );
	m_inputAnnotationNeeded = createInputPin<bool>("annotation needed", this);
	//m_inputSerialSeen = createInputPin<unsigned int> ("serial check pincueflush", this);
	m_inputBlobTrackState = createInputPin<QList<plvblobtracker::PlvBlobTrackState>>("blobtracker state", this);
	//< QList<plvblobtracker::PlvBlobTrackState>>* m_outputAnnotationSituation;
	m_inputBlobs = createInputPin< QList<BlobTrack> >( "input tracks", this );
	m_inputImage->addAllChannels();
    m_inputImage->addAllDepths();

    m_outputPin = createOutputPin<QString>( "output", this );
	
	//doesnt work due to cycle check:
	//m_changeFramePin = createOutputPin<int>( "frame changer", this );
}

TrackAnnotation::~TrackAnnotation()
{
	if(m_popUpExists)
	{
		m_popupWidget->close ();
	}
	m_stopwhile = true;
	qDebug() << "~ in trackannotation";
}


bool TrackAnnotation::deinit() throw ()
{
	m_stopwhile = true;
	m_plvannotationwidget.deinit();
	qDebug() << "deinit in trackannotation";
    return true;
}

bool TrackAnnotation::stop()
{
    m_stopwhile = true;
	qDebug() << "stop the pipeline qdebug";
	return true;
}


bool TrackAnnotation::init()
{
	m_waitserial = 0;
	m_skipprocessloop = false;
	m_skipprocessloopback = false;
	m_skipprocesslooptrack = false;
	m_closedwidget = false;
	m_back_prev = 1;  //assume to start with +
	m_stopwhile = false;
	m_plvannotationwidget.init();
	createPopup();
	m_popUpExists = m_plvannotationwidget.m_popUpExists; 
	
	return true;
}


//probably need to create a seperate popup-widget class myself in order to deal with the mouse events
bool TrackAnnotation::createPopup()
{
	m_plvannotationwidget.createPopUp();
	m_popUpExists = m_plvannotationwidget.m_popUpExists; 
	return true;
}

//TODO likely no longer needed
//read frame forward,back, current
int TrackAnnotation::readFile(QString filename, bool previous) 
{
	int back = -3; //-1,0,1
	int framefilenrset = -1; //0-650000 or so.
	bool signalToSet = 0; //should be true if actually set
	QFile inFile(filename);
	if(inFile.exists())
	{
		if ( inFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) 
		{
			QString processingserial,framechange;
			//double processingserialDouble;
			int previousvalue;

			QTextStream stream( &inFile );
			QString line;

			for (int counter = 1; counter < 2; counter++) {
				line = stream.readLine(); 
				// line of text excluding '\n'
			}

			//QString toimageproducer = QString("%1 \t %2 \t %3").arg(back).arg(back+filenamenr).arg(true);
		
			back = line.section('\t', 0,0).toInt();
			framefilenrset = line.section('\t', 1,1).toInt();
			//signal is not needed here
			qDebug() << "debug readfile trackannotation " << "first val giving dir " << back << "2nd value to set frame " << framefilenrset;
			//do stuff with the temp strings
			
			//if (processingserialDouble < (this->getProcessingSerial()) && processingserialDouble > (this->getProcessingSerial()-3) )
		}
	}
	inFile.close();
	if (previous)
		return back;
	return framefilenrset;
}

//?does this do anything? I dont think so:
void TrackAnnotation::mouseaction()
{
	qDebug() << "mouseclick";
}

//?does this do anything? I dont think so:
void TrackAnnotation::mousePressEventCopy(QMouseEvent *event)
{
	event->accept();
}

bool TrackAnnotation::getStopState()
{
	if (m_plvannotationwidget.framestate != NotSet)
	{
		m_stopwhile = true;
		//this function, specifically this state is set twice after an arrow button
		//qDebug() << "no longer not notset" << m_plvannotationwidget.framestate;
	}
	else if(m_plvannotationwidget.m_error)
	{
		m_stopwhile = true;
		m_closedwidget = true;
		qDebug() << "m_error";
	}
	
	return m_stopwhile;
}

bool TrackAnnotation::process()
{
	//there is a cue of images out there which need be diregarded. They have to be filled in again
	//lets experiment 

	//Flushing them directly wont work, will result in asyncronous filename and pictures, we need to manually skip the frame and flush entire process loops, 
	//need to set the right framenr and skip all the stuff in the process loop, once a annotation has been done, or if one wishes to go back. 
	//to save corrected tracks data:
	QString out;
	QString filename = m_fileNameInputPin->get();
	int filenamenr = m_fileNameNrInputPin->get();

	//returns a lot of unnecesary empty frames, but we need to set an out probably satisfies parlevision restrictions of output in outputpin and gives direct feedback
	out = QString("PROCESSING SERIAL %1 FRAME:%2#").arg( this->getProcessingSerial()).arg(filename);
	
	//if(m_waitserial == m_inputSerialSeen->get())
	m_correctimagedirectorybool = m_correctimagedirectoryboolInputPin->get();

	if (m_skipprocessloop)
	{
		if(m_correctimagedirectorybool)
		{
			m_skipprocessloop = false;
			qDebug() << "set to non-skip by signal in";
		}
	}
	/*if (m_skipprocessloopback)
		if(m_correctedtrack)
			m_skipprocesslooptrack = false;*/
	
	//all gets are neccesary to do
	m_stopwhile = !m_inputAnnotationNeeded->get();
	QList<plvblobtracker::PlvBlobTrackState> blobtrackstate = m_inputBlobTrackState->get();
	QList<plvblobtracker::BlobTrack> tracks = m_inputBlobs->get();
	//get image of tracks and put it
	CvMatData image = m_inputImage->get(); 

	if (!m_skipprocessloop)
	{
		qDebug()<< "enter boolshit";
		//m_stopwhile = false; //false would block, false would meen no annotation needed
		//if annotation needed is true then stop(the-while loop) should be false

		//TODO add a pin an int that transfers the reason why the pipeline is stopped (merged,split etc according to the tracker, this should be represented somewhere as well)
		
		foreach ( PlvBlobTrackState bts, blobtrackstate)
		{
			if (debugging)
				qDebug() << "state(s) from blobtracker " << bts;
		}

		//if going back in frames also block until proceeding again
		if (m_stopwhile == true)
		{
			//TODO this was always true, is it working now?
			//if the previousvalue was -1 (gotocurrent) or -2(goto previous) than block the process until next is pressed.
			//qDebug() << "the stopwhile is true and i read file in trackanno " <<  readFile(m_filenameFrameNr,true);
			qDebug() << "the stopwhile is true" << m_back_prev;
			//if(readFile(m_filenameFrameNr,true)<0)
			if(m_back_prev<0)
			{	
				m_stopwhile = false;
			}
		}

		m_plvannotationwidget.processInit();
	
		//for clarity:
		QString windowname = QString("annotationwidget, nr %1").arg(filenamenr);
		m_plvannotationwidget.setWindowTitle(windowname);
		
		m_plvannotationwidget.toPaint(image);
		
		int back = 1;	
		//wait for a arrow key before proceeding
		//will result in a crash when parlevision is stopped without exiting the popupscreen
		//can't be tested in debug because of the use of different threads according to debug error.
		
		//wait for the popup to give input (when needed)
		while (m_stopwhile==false) 
		{
		  if (getStopState())
		  {
			break;
		  }
		}

	
		saveBlobChangeDataToFile(filename);

		//the popup changes frame in following code

		//TO ALTER THE FRAME/ TO REWIND and set this to the imagedirectoryproducer
		//there used to be a second getstopstate check
		//default next frame, in imagedirectory -1 and +1 are added 
		back = 1;
		if (m_plvannotationwidget.framestate == GotoPreviousFrame)
			back = -1;
		if (m_plvannotationwidget.framestate == GotoCurrentFrame)
			back = 0;
		if (m_plvannotationwidget.m_error)
		{
			//keep same image
			back = 0;
			//back = 999;
		}
		
		qDebug() << "getstopstate back: " << back;
		
	
		//if widget is closed and turns into a nonblocking state it wil not save to the annotation files
		foreach( BlobTrack t , tracks )
		{
			cv::Point p = t.getLastMeasurement().getCenterOfGravity();
			cv::Point pbackup = p; 
			QString blobString;
		
			//m_plvannotationwidget.m_blobchanges
			if (getSaveToFile())
			{
				//save to annotationfile even if there is no changed blob, but don't save to annotation file again if allready found a corresponding changed one.
				bool changedblob = false;
				foreach( BlobChangeData bcd , m_plvannotationwidget.m_blobchanges)
				{
					//there can be multiple changed blob data with the same oldid, eg after merging. 
					if (bcd.oldid == t.getId())
					{
						//independent of state if cogs are set with a mouseclick override the position of the blob
						if (bcd.cogs.x != 0 || bcd.cogs.y != 0)
						{
							qDebug() << "cogs improvement clicked for " << bcd.oldid;
							p = bcd.cogs;
						}
						else
						{
							p = pbackup;
						}

						if (bcd.changetype == 'N')
						{
							//a neutral unnecassary state with no changed blobs basiccaly. 
							changedblob = true;
							//out append is used to directly have a way to view the info in the log file;
							out.append(saveTrackToAnnotation(filename,t, p,'N', t.getId()));
						}
						else if (bcd.changetype == 'E')
						//exchanged, allready interpreted blobchangedata as two assigns
						{
							changedblob = true;
							//basically a double Assign thus this should not be reachable, save anyway
							out.append(saveTrackToAnnotation(filename,t, p,'E',t.getId()));
							qDebug() << "a strange thing happend on exchange, this state is supposed to be unreachable";
						}
						//assigned
						else if (bcd.changetype == 'A')
						{
							changedblob = true;
							//t.setID(bcd.newid);
							out.append(saveTrackToAnnotation(filename,t, p,'A',bcd.newid));
						}
						//merged
						else if (bcd.changetype == 'M')
						{
							changedblob = true;
							out.append(saveTrackToAnnotation(filename,t, p,'M',bcd.newid));
						}
						//divided
						else if (bcd.changetype == 'D')
						{
							changedblob = true;
							//maybe make clearer in the text file that one has to be removed?
							out.append(saveTrackToAnnotation(filename,t, p,'D',bcd.newid));
						}
						//boring, similar to neutral but also removes the block to proceed in the direction one was going, no clue why I choose that over using arrows, maybe to prevent overwriting track stuff?
						//else if (bcd.changetype == 'B')
						//{
						//	changedblob = true;
						//}
						else
						{
							//supposed to be unreachable:
							changedblob = true;
							out.append(saveTrackToAnnotation(filename,t, p,' ', t.getId()));
						}
					}
				}
				if (!changedblob)
				{
					//TODO seems to be independent of error state of the widget and will thus also be saved in error state (once programs is actually stopped)
					out.append(saveTrackToAnnotation(filename,t, p,' ',t.getId()));
				}

				//independent of the annotationstate
				out.append("\n");
			}	
		}

		//the frames in the cue allready processed by the imageproducer might not be the right ones, if so skip them, loop but dont reallt process
		//the same goes for the blobtracker, it should only process the right frames and the changes have to be loaded into the blobtracker
		//annotation should also skip the not updated blobtracker stuff
		//the current solution is to always skip frames and set the framenr in trackannotations file to imagedir
		//if something is changed and not just proceeding with loop, one needs to trash a lot of frames.
		
		//as long we keep on going in the same forward direction we might not need to skip the frames, for backward it is easier to do so.
		if (back!=m_back_prev || back!=1 || m_plvannotationwidget.m_blobchanges.size()>0 )
		{
			m_skipprocessloop = true;

			//for practical reasons also saved the previous state of framechanging, previous, current or next indicated with the keyboards arrows, however this might no longer be needed
			//send the framedirection int and send the wanted image to the txt file which than should be reset in the imagedirectoryproducer, with some delay however depending on the pipeline
			//either add the 3rd value with processing serial, to see if the correct frame will be loaded this->getProcessingSerial()
			//OR TRY to send a boolean trigger that will be loadded by imae directory and makes processes not really process everything until a frame with a trigger is hit.
			//need to set the current wanted framenumber instead of + or - as the delay can otherwise not be directed;
			//also send the true signal 
			QString toimageproducer = QString("%1 \t %2 \t %3").arg(back).arg(filenamenr+back).arg(true);
			qDebug() << "back changed in track anno" << toimageproducer;
			QFile file2(m_filenameFrameNr);
			bool ret2 = file2.open(QIODevice::WriteOnly | QIODevice::Truncate);
			Q_ASSERT(ret2);
			QTextStream s(&file2);
			s << toimageproducer;
			ret2 = file2.flush();
			Q_ASSERT(ret2);
			file2.close();
		}
		else
		{
			//this will only be reached after the signal or a in other ways appropriate frame
			//the thing might have been unnecassary to comment or something
			m_skipprocessloop = false;
		}
		
		//send the skipping process boolean to the blobtracker to also let that process skip
		QString toBlobTrackerSignal = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7").arg(filename).arg(-1).arg(this->getProcessingSerial()).arg(0).arg(0).arg(m_skipprocessloop).arg(' ');
		QFile file(m_filenameBCD);
		//append instead of overwrite
		bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
		Q_ASSERT(ret);
		QTextStream s(&file);
		for (int i = 0; i < toBlobTrackerSignal.size(); ++i)
			s << toBlobTrackerSignal.at(i);
		file.write("\n");	
		ret = file.flush();
		Q_ASSERT(ret);
		file.close();
		
		m_back_prev = back; //save this latest back value
	
	}
	else
	{
		qDebug()<<"SKIP";
	}

	m_outputPin->put(out);
	
	return true;
}


void TrackAnnotation::saveBlobChangeDataToFile(QString filename)
{
	//inpractical for debugging to remove file here, it will never be seen in the folder.
	QFile filerem(m_filenameBCD);
	filerem.remove();

	foreach( BlobChangeData bcd , m_plvannotationwidget.m_blobchanges)
	{
		QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7").arg(filename).arg(bcd.oldid).arg(bcd.newid).arg(bcd.cogs.x).arg(bcd.cogs.y).arg(m_interesting).arg(bcd.changetype);
		QFile file(m_filenameBCD);
		//append instead of overwrite
		bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
		Q_ASSERT(ret);
		QTextStream s(&file);
		for (int i = 0; i < blobTabString.size(); ++i)
			s << blobTabString.at(i);
		file.write("\n");	
		ret = file.flush();
		Q_ASSERT(ret);
		file.close();
	}
}

QString TrackAnnotation::saveTrackToAnnotation(QString filename, BlobTrack t, cv::Point p, char annotationstate, int newid)
{
	//prevent logging after closing the widget, if the widget is closed once it enters a error stage that will prevent blocking the process.
	if (!m_closedwidget)
	{
	//TODO check when later read into matlab, the divided will end up using only the last blob's value, so compare d-state and processingserial?
	//add processingserial only save the ones with a higher processing serial, i guess both will have the same processingserial?
		//maybe better to save the values instead of calling them twice. although none of these values seem to require much proccesing power, they only return a value. 
		//TODO also save track id
		QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \t %8 \t %9 \t %10 \t %11 \t %12").arg(filename).arg(t.getId()).arg(newid).arg(p.x).arg(p.y).arg(t.getAveragePixel()).arg(t.getDirection()).arg(t.getVelocity2()).arg(t.getAge()).arg(m_skipprocessloop).arg(annotationstate).arg(this->getProcessingSerial());
		
		QFile file(m_filenameLog);
		bool ret = file.open(QIODevice::WriteOnly | QIODevice::Append);
		Q_ASSERT(ret);
		QTextStream s(&file);
		for (int i = 0; i < blobTabString.size(); ++i)
			s << blobTabString.at(i);
		file.write("\n");	
		ret = file.flush();
		Q_ASSERT(ret);
		file.close();
		return blobTabString;
	}
	QString blobTabString2 = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \t %8 \t %9 \t %10 \t %11").arg(filename).arg(0).arg(0).arg(0).arg(0).arg(0).arg(0).arg(0).arg(0).arg(annotationstate).arg(this->getProcessingSerial());
	return blobTabString2;
}

QString TrackAnnotation::getFilenameLog()
{
    QMutexLocker lock( m_propertyMutex );
    return m_filenameLog;
}

 