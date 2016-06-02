#include "bloxprocessor.h"

#include <QDebug>

#include "BloxProcessor.h"
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>

//we need the info from blobtrack now.. included via the header file
//requires plv_blobtracker_plugin.dll;
//#include "../plvblobtracker/Blob.h"
//#include "../plvblobtracker/BlobTrack.h"
//#include <QtMultimedia/QAudioOutput>

using namespace plv;

BloxProcessor::BloxProcessor() :
        //m_showPopup(true),
        m_someString("I"),
		m_playTrigger(true),
		m_overrideString(false),
		m_personOverruledByMouse(false),
		m_ballOverruledByMouse(false),
		m_faceDetected(false), 
		m_overrideFaceDetected(false),
		m_timeWithoutHeadThreshold(3000), //in milliseconds
		m_framesAveragingPosition(15),
		m_ballRollSpeed(60),
		m_popupSkipRate(0),
		m_timeIntervalToRemindAgain(30000),
		m_minPosCorrection(0.5),
		m_pixelsDiffPersonBall(50),
		m_factorMetersMovementToPixels(640),
		m_pathStartSound(""),
		m_pathLookAwaySound(""),
		m_pathReminderSound(""),
		m_factorMaxball(1.28),
		m_messageDuration(750),
		m_sendMessageInterval(600),
		m_virtualBallCommands(false)
{
    m_inputPin = createCvMatDataInputPin("input", this);
    m_inputPinHR = plv::createInputPin<QString>("input head rotation", this);
	m_inputPinHP = plv::createInputPin<QString>("input head position", this);
	//plv::InputPin< QList<plvblobtracker::BlobTrack> >* m_inputBlobs;
	
	m_inputPinSB = plv::createInputPin<QString>("input ball position", this);
	//m_inputBlobs = createInputPin< QList<plvblobtracker::BlobTrack> >( "input", this );

	m_outputPin = createCvMatDataOutputPin("output", this);
	m_outputPin2 = createOutputPin<QString>( "text", this);
	m_outputPin3 = createOutputPin<QString>( "soundplayed", this);
	
	m_inputPin->addSupportedChannels(1);
	m_inputPin->addSupportedChannels(3);

    m_inputPin->addSupportedDepth(CV_8U);
    m_inputPin->addSupportedDepth(CV_16U);

	m_headinframe = 0; 
	m_initialised = false;

}

BloxProcessor::~BloxProcessor()
{
}

//return the current situation into a command for the Ball
QString BloxCommand::getString(){
	QString commandoBal = "-";
	if(!getGrabAttention())
	{
		if (getDirection() == 1)
		{
			commandoBal = QString("F1,%1,%2").arg(getSpeed()).arg(getDuration());
		}
		else if (getDirection() == -1)
		{
			commandoBal = QString("B1,%1,%2").arg(getSpeed()).arg(getDuration());
		}
		else 
		{
			commandoBal = "S";
		}
	}
	else
	{
		//UNSAVE random movement in the ball if stopped, it will keep moving even when stopped it seemed,
		//commandoBal = QString("F1,40,%1").arg(durationWiggle);
	}
	return commandoBal;
}

bool BloxProcessor::init()
{
	m_ballPosition.x= 0;
	m_ballPosition.y= 0;
	m_personPosition.x=0;
	m_personPosition.y=0;

	//bloxcommandos:
	m_bloxCommandos.setDirection(0);
	m_bloxCommandos.setDuration(getMessageDuration());
	m_bloxCommandos.setSpeed(getBallRollSpeed());
	m_bloxCommandos.setGrabAttention(false);
	m_bloxCommandos.setDurationWiggle(1000);
	
	setBeforeStart(true);
	setInitialised(false);

	return true;
}

bool BloxProcessor::deinit() throw ()
{
	//popup creation
	m_plvannotationwidget.deinit();
	
    return true;
}

bool BloxProcessor::start()
{
	//only perform these actions once and not every time it is started, as they will lead to memory usage and probably leaks
	if (!getInitialised())
	{
		initialiseSoundAndWidget();
	}
	
	setBeforeStart(false);


	//is not a real problem if this is init several times as is it is for the sounds
	//m_plvannotationwidget.init();

	//**ugly solution, perhaps qelapsedtimer is more appropriate 
	//we want to be sure the timer is started and we need to restart it, 
	//in case start has been called twice it is uncertain if it will reset 
	//we need to restart in case the pipeline has been started before and is now stopped for instance
	m_lastheadfocus.start();
	//m_lastheadfocus.restart();
	m_intervaltimer.start();
	//m_intervaltimer.restart();

	m_bloxCommandos.setDuration(getMessageDuration());

	setFaceDetected(false);
	m_woozfacefirstremindergiven = false;
	//setShowPopup(getShowPopup());
	m_skipcounter = 0;

	m_firstrun = true;
    return true;
}

bool BloxProcessor::stop()
{
	//TODO destory the timers? m_lastheadfocus.
	//m_lastmovement = NULL;
    return true;
}

void BloxProcessor::attentionGrabbingBehavior(int behaviortype)
{
	playBloxSound(behaviortype);
	//TODO send some command to the ball to let it wiggle
	m_bloxCommandos.setGrabAttention(true);
	//so have a descirption of the wiggle in bloxcommands
}

void BloxProcessor::playBloxSound(int songnr)
{
	//folder libs as parrent so ; /parlevision/libs/mysounds/x.wav --> "mysounds/bells.wav"
	switch (songnr)
	{
			//playing sounds is not that straighforward, only one can be played at a time it seems, one starts ands stops after the second starts
			//perhaps TODO implement the Phonon Module http://qt-project.org/doc/qt-4.8/phonon-module.html
			//but this requires a full fledged rebuild!  QT += phonon
			//"Applications that use Phonon's classes need to be configured to be built against the Phonon module. The following declaration in a qmake project file ensures that an application is compiled and linked appropriately:"
			//perhaps threading would help? as we did gfor the kinect facetracker?
			//the play function from qt sound returns immediatly thus not threading needed for that for now.
		////move the connection to its own thread. 
		//QThreadEx* m_ftThread = new QThreadEx();
		//facetrack->moveToThread(m_ftThread);
		//// when the tracker is done it is scheduled for deletion
		//connect( facetrack, SIGNAL(finished()),
		//         facetrack, SLOT(deleteLater()));
		////doesnt make sense
		//connect( facetrack, SIGNAL(finished()),
		//        facetrack, SLOT(stop()));
			
			//as defined for attenion grabbing behavior, chronological at 0=startup, 1=looked away, 2=gaining interest with an interval
			case 0:
			//old: other options to play a sound:
			//QSound::play("mysounds/hallo.wav");
			//m_startsound.play();
			
				if (p_startsound != NULL)
				{		
				   p_startsound->play();
				   m_outputPin3->put("0");
				}
				else
					qDebug() << "NULL POINTER start";
				break;

			case 1:
			
				if (p_lookawaysound != NULL)
				{	
					p_lookawaysound->play();
					m_outputPin3->put("1");
				}
				else
					qDebug() << "NULL POINTER lookaway";
				//QSound::play("mysounds/he.wav");	
				break;

			case 2:
			
				if (p_remindersound != NULL)
				{	
					qDebug() <<  "reminder, you looked away for some time: play reminder sound";
					p_remindersound->play();	
					m_outputPin3->put("2");
				}
				else
					qDebug() << "NULL POINTER reminder";
				//QSound::play("mysounds/aandacht.wav");
			
			break;

			default:
			qDebug() <<   "state for play sound not set" << songnr;
			break;
	}		
}

bool BloxProcessor::process()
{
	//TODO split in submethods this is bullshit
	//e.g. 
	//posInfo, rotInfo
	//wooz ball / wooz person
	//command string 
	//and drawing

	bool debugging = false;
	if (debugging)
		qDebug() << "enter bloxprocessor" << this->getProcessingSerial();

	//DEAL WITH THE CLICKS IN THE WIDGET

	//set gui boolean in plv for showpopup according to whether the window is closed or not in the gui windows
	//in the firstrun 
	if (debugging)
		qDebug() << "setshowpopup bloxprocessor" << this->getProcessingSerial();
	if (!getFirstRun() && (m_plvannotationwidget.getPopUpExists() != getShowPopup()))
	{
		setShowPopup(m_plvannotationwidget.getPopUpExists());
	}

	//use this information to skip tracking if it is not needed
	//check whether wooz is done
	//if wooz than in these methods the positions are also set
	if (debugging)
		qDebug() << "woozpos" << this->getProcessingSerial();
	//debugging:
	bool woozPerson = woozPersonPosition();
	if (debugging)
		qDebug() << "woozball" << this->getProcessingSerial();
	bool woozBall = woozBallPosition();
	
	//GET THE POSITION OF THE BALL FROM A NORMAL CAMERA: 
	if (!woozBall)
	{
		if (debugging)
			qDebug() << "trackball" << this->getProcessingSerial();
		getSetTrackInfoBall();
	}
	else
	{
		//to deal with pipeline processing of parlevision:
		//also get the information from the ball position pipeline even if we don't need it, just to prevent cueing in parlevision
		QString srcSB = m_inputPinSB->get();
	}

	//set the current pos according to the widget 
	//DEAL WITH HEAD FOCUS errors
	//dont grab attention unless we just start, didn't have a face for the first time (looked away), or if we want to grabattention again, 
	//so set to false unless we actually want to grab attention for these reasons
	//we need this to distinguish from the other type of commands we can send to the ball etcetra.
	m_bloxCommandos.setGrabAttention(false);
	//TODO I do not yet set grabbing attention triggers on manual overrideface detected it seems
	//do this in set face detected and int

	if (!woozPerson)
	{
		if(!getOverrideFaceDetected()) 
		{
			if (debugging)
				qDebug() << "getsetrot no woozpers no getoverride face" << this->getProcessingSerial();
			getSetHeadRotationTracking();
		}
		else
		{
			//also get the information from the head rotation pipeline even if we don't need it, just to prevent cueing in parlevision
			QString srcRotationPin = m_inputPinHR->get();
			woozFaceDetectionReminder();
			//added during pilot
			if (getFirstRun())
			{
				setFirstRun(false);
			}
		}
		//for steering the ball we actually only need to get the head position if the head was seen in the scene in the last period, in which the period is set with the gui.
		//however we do get the head position even when no face is detected or when face detection is overwritten
		if (debugging)
				qDebug() << "getsetpos no woozpers" << this->getProcessingSerial();
		getSetHeadTrackingPosition();
	}
	else
	{
		//if we are wooz the position of the person it does not mean we want to wooz the presence of the head in te image as well
		//there for we should check for a face in rotation if one does not use overridefacedetected here
		//TODO make overrule van facedetected working, not overruled by actual detection
		if(!getOverrideFaceDetected()) 
		{
			if (debugging)
				qDebug() << "wooz no overide face" << this->getProcessingSerial();
			getSetHeadRotationTracking();
		}
		else
		{
			
			//added during pilot
			if (getFirstRun())
			{
				setFirstRun(false);
			}
			setHeadInCurrentFrame(getFaceDetected());
			//also get the information from the head rotation pipeline even if we don't need it, just to prevent cueing in parlevision
			QString srcRotationPin = m_inputPinHR->get();
			woozFaceDetectionReminder();
		}
		
		//also get the information from the head position pipeline even if we don't need it, just to prevent cueing in parlevision
		QString srcPositionPin = m_inputPinHP->get();
	}

	
	//LOGIC and command for movement commmands, put in own method
	//output the string to be used for logging and or tcp server
	//will return - if it is not supposed to move
	if (debugging)
		qDebug() << "move or stop ball commands method" << this->getProcessingSerial();
	m_outputPin2->put(moveOrStopBall());
	
	//DRAW the positions and indicate facetracked 
	//get the image
	if (debugging)
		qDebug() << "get image stuuf before drawing" << this->getProcessingSerial();
	assert(m_inputPin != 0);
	CvMatData src = m_inputPin->get();
	//bugfix head out of reach
	//if not a proper src than stop processing stuff
	if (src.width() < 320 || src.width() > 640 )
		return true;

	if (debugging)
		qDebug() << "draw into the widget" << this->getProcessingSerial();
	//if anoher strange size or strange position, the position of the bal and person will also be reset in the draw the positions method to fitting maxima
	//method returns a cvmat reference
	CvMatData outputImage = drawThePositions(src);
    m_outputPin->put( outputImage );

	if (debugging)
		qDebug() << "exit bloxprocessor" << this->getProcessingSerial() ;;
    return true;
}


void BloxProcessor::woozFaceDetectionReminder()
{

	//METHOD WOOZ FACEDETECITON REMINDER:
	//also deal with attentiongrabbingbehavior as normally
	if(getFaceDetected())
	{
		//assume started: m_lastheadfocus.start();
		m_lastheadfocus.restart();
			
		//assume started: m_intervaltimer.start();
		m_intervaltimer.restart();

		m_woozfacefirstremindergiven = false;
	}
	else if(!m_woozfacefirstremindergiven && (m_lastheadfocus.elapsed() > getTimeWithoutHeadThreshold()))
	{
		attentionGrabbingBehavior(1);
		m_woozfacefirstremindergiven = true;
	}
	//only perform the more immersive attention grabbing behavior with a certain interval, the less immersive sound was played when clicking off face detected in the GUI
	//getTimeIntervalToRemindAgain()
	else if(m_intervaltimer.elapsed()>getTimeIntervalToRemindAgain())
	{
		attentionGrabbingBehavior(2);
		m_intervaltimer.restart();			
	}
}

bool BloxProcessor::woozPersonPosition() //{m_pathStartSound = s; emit(pathStartSoundChanged(s));}
{

	//check if the widget has been clicked
	if (m_plvannotationwidget.getMouseClickedPerson())
	{
		if(!getPersonOverruledByMouse())
			setPersonOverruledByMouse(true);
	}
	//if we use wooz in any case we need to set position of the ball according to the clicked positions

	//set the person position according to the wooz clicked position

	if (getPersonPosition())
		setPersonPosition(m_plvannotationwidget.getPointWanted().x);
	
	return getPersonOverruledByMouse();
}

bool BloxProcessor::woozBallPosition()//{m_pathStartSound = s; emit(pathStartSoundChanged(s));}
{
	if (m_plvannotationwidget.getMouseClickedBall())
	{
		if(!getBallOverruledByMouse())
			setBallOverruledByMouse(true);
	}

	//if we use wooz set position of the ball according to the last clicked positions
	if (getBallOverruledByMouse())
		setBallPosition(m_plvannotationwidget.getPointAssumed().x);
	

	return getBallOverruledByMouse();
}

void BloxProcessor::getSetTrackInfoBall()//{m_pathStartSound = s; emit(pathStartSoundChanged(s));}
{
	//via Strings..... which makes life easier as now import of the blobtrack class is needed, but makes the program slower.... 
	QString srcSB = m_inputPinSB->get();
	//get a stringlist containing all the tracks
	QStringList tracks = srcSB.split("\n");
	//get a stringlist with the info of the tracks
	QList<QStringList> trackinformation;
	int biggestblob = 0; 
	//check all incoming tracks to get the biggest one, assuming this will be the ball due to the cropping of the image that has occured. 
	for( int i=0; i<tracks.count(); i++ )
	{ 
		//in the format specified in tracktostringconverter:
		//QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \%8").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge()).arg(t.getBlobSize()); 
		//the tabdelimited version seems to have some issues within a qstringlist
		//QString blobTabString2 = QString("%1 \t #%2 \t #%3 \t #%4 \t #%5 \t #%6 \t #%7 \t #\%8").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge()).arg(t.getBlobSize()); 
		QStringList track = tracks.at(i).split("#");
		if (track.count()==8)
		{
			trackinformation.push_back(track);
			//again we want to take the biggest blob, as this will be the ball if setup correctly!
			//if no int can be made of this split string it will return zero
			if (tracks.count()>0 && (track.at(7).toInt() >= trackinformation.at(biggestblob).at(7).toInt()))
			{
				biggestblob= i;
			}
			//qDebug()<< "track information is correct " <<track.count() << i <<track.at(7).toInt();
		}
		else
		{
			//alway one additional line is added!
			//if(srcSB != "no data")
			//	qDebug()<< "track information is incorrect " <<track.count() << i << tracks.count() << srcSB;
			
		}
	}
	
	if (trackinformation.count()>0 && trackinformation.count()>=biggestblob) 
	{
		if (trackinformation.at(biggestblob).count()>1)
		//we want to keep track of the last sensible position of the ball
		{ 
			if (trackinformation.at(biggestblob).at(1).toInt()>0)
				setBallPosition(trackinformation.at(biggestblob).at(1).toInt()*getFactorMaxball());
		}
	}
}

void BloxProcessor::getSetHeadTrackingPosition()
{
	//use the input of the kinect position of the head
	//input is a tabdelimited xyz pos
	QString srcPositionPin = m_inputPinHP->get();
	//split the position with tabs according to the input, use these tabs to create an appropriate list that is easy to deal with : a stringlist
	QStringList xyzpos = srcPositionPin.split("\t");
	float xposframe = 0.0f;

	//if the head indeed has been transmitted correctly with tabs, otherwise a crash would occur later on 
	if (xyzpos.length()==3)
	{
		xposframe = -1* xyzpos.at(0).toFloat();

		//take into account that momentary values of 0 are kind of useless, it seems to be better to use the last known positions only.
		//average over size of the list of positions so delete if there are more coming in
		if (xposframe!= 0.0) 
		{
			manuallistmutex.lock();
			m_possitionsarray.push_front(xposframe);
			//delete the more than needed positions of the last frames
				//framesaveraging has to be bigger than 0
			while(xposframe!= 0.0 && m_possitionsarray.size()>getFramesAveragingPosition())
			{
				m_possitionsarray.removeLast();
			}
			manuallistmutex.unlock();
		}
		
		//average over these frames
		float foraverage = 0;
		
		//prevent from accesing the list while deleting values;
		manuallistmutex.lock();
		for(int i=0; i<m_possitionsarray.count(); ++i)
		{
			foraverage = foraverage+m_possitionsarray[i];
		}
		int nrofpositionssaved = m_possitionsarray.size();
		manuallistmutex.unlock();
		
		//calculate the avarage value
		//check for average of 0: this allows for manual steering of the ball when no face was detected in the last 15 frames;
		if (foraverage!=0.0)
		{
			if (nrofpositionssaved>0)
			{
				//cast to double as we have a point2d format;
				setPersonPosition( ( (double) (foraverage/nrofpositionssaved)+getMinPosCorrection() ) * getFactorMetersMovementToPixels()) ;
			}
			else
				qDebug() << "this should be impossible arraycount =0"; 
		}
	}
	else
	{
		//TODO check if warning behaves similar to a qDebug()
		qDebug() << "no split, length, check if the right input of head tracking in tabdelimited format is used " << xyzpos.length();
	}
}

//also responsibly for throwing the grabbingbehavior when a head is turned
//not reachable in the wooz override head detected mode
//only dealas with get and attentiongrabbing doesn't set position or anything else
void BloxProcessor::getSetHeadRotationTracking()
{
	
	QString srcRotationPin = m_inputPinHR->get();
	//for now we only check if a head is detected, so a boolean, future version might need actual rotation in x or y.
	QStringList srcRotation = srcRotationPin.split("\t");
	bool rotationHead = false;
	setHeadInCurrentFrame(false);

	if (srcRotation.length()==3)
	{
		//have different behavior in first run independent of whether a face is recognised or not.
		if (getFirstRun())
		{
			setFirstRun(false);
			attentionGrabbingBehavior(0);
		}
		//if a head was seen:
		//an angle of 0.0 is send if no head was detected
		//lastmin CHANGED 
		else if (srcRotation.at(0).toFloat() != 0.0 )
		{
			setHeadInCurrentFrame(true);
			
			//we also need to set the timer of the last send command to move the ball to position of the player if it might have been neglected due to loss of the focus of the player
			if(!getFaceDetected())
			{
				//assume started
				m_lastmovement.restart();
			}

			
			//ugly solution, just reset the time since a head is seen to 0,  this will work for now
			//assume started: m_lastheadfocus.start();
			m_lastheadfocus.restart();
			//set that a head is seen
			//always have the timer running will make behavior more predictable
			
			setFaceDetected(true);

		}
		//if the head is not seen, it is not directed at the ball after initial turning for longer than the threshold time: grab their attention
		//or if that was already the case, then grab their attention every interval again .
		else
		{
			//respond to that the head is not there, and grab attention first time and after every interval afterwardsallready started and counting the seconds when the head was seen
			if (m_lastheadfocus.elapsed() > getTimeWithoutHeadThreshold())
			{
				//if a face was looking at the kinect but now turning away. So if a face was seen before and not now (for more than the look away threshold)
				//slightly grab attention and behave accordingly and start the interval timer
				//at least if not manual override of the behavior, otherwhise
				if (getFaceDetected())
				{
					//do the interactive stuff method:
					//a subtle method as it is triggered quite often
					attentionGrabbingBehavior(1);
					//assume started
					m_intervaltimer.restart();
					
					//setFacedetected itself is a gui element as well, so the override check shouldnt be done there
					if (!getOverrideFaceDetected())
					{
						setFaceDetected(false);
					}
				}
				//only perform the more immersive attention grabbing behavior with a certain interval
				else if(!getFaceDetected() && m_intervaltimer.elapsed()>getTimeIntervalToRemindAgain())
				{
					attentionGrabbingBehavior(2);
					//assume started
					m_intervaltimer.restart();
				}
			}
			//else
			//{
			//	m_lastheadfocus.start();
			//}
		}
	}
}

//also takes into account if the ball is moved in another direction 
QString BloxProcessor::moveOrStopBall()
{
	int lastdirection = m_bloxCommandos.getDirection();
	bool needtomove = false;

	//if difference between wanted and current pos is big enough move the ball to that direction
	if ( getPersonPosition()-getBallPosition() > getPixelsDiffPersonBall())
	{
		m_bloxCommandos.setDirection(1);
	}
	else if (getBallPosition()-getPersonPosition() > getPixelsDiffPersonBall())
	{
		m_bloxCommandos.setDirection(-1);
	}
	else
	{
		m_bloxCommandos.setDirection(0);
	}
	
	//get the ball protocol correct String
	QString commandString = "-" ;
	
	//only send a command if it is in an opposite direction or if the ball has not moved for some time
	if(m_firstrun)
	{
		//start the sending info thing for virtual ball
		if (m_lastmovement.elapsed() >0)
			m_lastmovement.restart();
		else
			m_lastmovement.start();
	}
	else if (!getVirtualBallCommands())
	{	
		//when direction is changed directly incorporated it in the orders to the ball
		if (lastdirection!=m_bloxCommandos.getDirection() )
		{
			needtomove = true;
			//assume started	
			m_lastmovement.restart();
		}
	}
	
	
	//only actually move if the player was looking at the ball in the last x seconds (threshold to set in GUI)
	if (getFaceDetected() && !getOverrideString())
	{
		if (m_lastmovement.elapsed()>(getSendMessageInterval()) || needtomove)
		{
			if(!getVirtualBallCommands())
				commandString = m_bloxCommandos.getString();
			else
			{
				commandString = QString("%1").arg(getPersonPosition());
			}
			
			//assume started
			m_lastmovement.restart();
		}
	}
	else if (getOverrideString())
	{
		commandString = getSomeString();
		//only send such a command once/
		setOverrideString(false);
	}

	return commandString;
	
}

CvMatData  BloxProcessor::drawThePositions(CvMatData src)
{
	bool debugging = false;

	if (debugging)
		qDebug() << "start draw into the widget" << this->getProcessingSerial();

	//bugfix
	//it seemed to crash if the head was out of screen
	if (getBallPosition()>src.width())
	{
		setBallPosition(src.width());
	} 
	else if (getBallPosition()<0)
	{
		setBallPosition(0);
	}
	//do the same for the ball:
	if (getPersonPosition()>src.width())
	{
		setPersonPosition(src.width());
	} 
	else if (getPersonPosition()<0)
	{
		setPersonPosition(0);
	}

	
	if (debugging)
		qDebug() << "point to src" << this->getProcessingSerial();
    CvMatData target;

	//point to data in src
	if(!src.isEmpty())
	{
		if (debugging)
			qDebug() << "non empty src" << this->getProcessingSerial();
		cv::Mat& in = src;
		int extraWidth  = 0;
		int extraHeight = 200;
		int radius = 50;
		
		//allocate a target buffer
		//inlcude a bottom screen to show the assumed position of the ball and be able to rectify this by clicking on it
		
		target.create( src.width()+extraWidth, src.height()+extraHeight, src.type() );
		cv::Mat& out2 = target;
		//for now just copy the input to the output
		in.copyTo(out2);

		//instead vconcat the mats so create the empty stuff:
		cv::Mat ballArea(cv::Size(src.width()+extraWidth,extraHeight),CV_8UC3);
		ballArea.setTo(0);
		cv::vconcat(in, ballArea, out2);
    	
		if (getShowPopup())
		{
			if (m_plvannotationwidget.getMinWantedY() != src.height())
				m_plvannotationwidget.setMinWantedY(src.height());
	
			if (m_plvannotationwidget.getMaxWantedY() != src.height()+extraHeight/2)
				m_plvannotationwidget.setMaxWantedY(src.height()+extraHeight/2);
	
			if (m_plvannotationwidget.getMaxAssumedPosY() != src.height()+extraHeight)
				m_plvannotationwidget.setMaxAssumedPosY(src.height()+extraHeight);

		}
		
		if (debugging)
			qDebug() << "draw rectangle" << this->getProcessingSerial();
		cv::rectangle(out2, cvPoint(0,src.height()), cvPoint(src.width(),src.height()+extraHeight/2), CV_RGB(150,150,150), -1, 8, 0);
	
		if (debugging)
			qDebug() << "getHeadInCurrentFrame" << this->getProcessingSerial();

		if(getHeadInCurrentFrame())
			cv::circle(out2, cvPoint((int) getPersonPosition(),(int) src.height()+radius), radius, CV_RGB(200,0,0), -1);
		else if (getFaceDetected())
			cv::circle(out2, cvPoint((int) getPersonPosition(),(int)src.height()+radius), radius, CV_RGB(150,50,50), -1);
		else
			cv::circle(out2, cvPoint((int) getPersonPosition(),(int) src.height()+radius), radius, CV_RGB(0,0,200), -1);

		//draw the position of the ball:
		cv::circle(out2, cvPoint((int) getBallPosition(),(int) src.height()+extraHeight/2+radius), radius, CV_RGB(200,0,0), -1);
		
		//popup
		//evt m_plvannotationwidget.setWindowTitle(windowname);

		//paint the popup if there is one
		//skips 1 shows one
		if(m_skipcounter>getPopupSkipRate())
		{	
			if (getShowPopup())
			{
				//only send this image if it is in the correct format 
				//size is allready checked before this
				if (target.depth() == CV_8U && target.channels() ==3)
					m_plvannotationwidget.toPaint(target);

			}
			m_skipcounter = 0;
		}
		else
		{
			m_skipcounter++;
		}
	}
	//if src is empty:
	else
	{
		target.create( 640, 480, CV_8U, 3);
	}
	// publish the new image
	return target;
}

void BloxProcessor::initialiseSoundAndWidget()
{
	QMutexLocker lock(m_propertyMutex);
	p_startsound = new QSound(getPathStartSound(),this);
	p_lookawaysound = new QSound(getPathLookAwaySound(),this);
	p_remindersound = new QSound(getPathReminderSound(),this);
	//we also set the init of the widget at that point as it is needed to not crash and also only needs to be init only once
	m_plvannotationwidget.init();

	setInitialised(true);
}

//we only need the x pos in the drawn ball and person for now
void BloxProcessor::setBallPosition(double balpos)
{
	QMutexLocker lock(m_propertyMutex);
	m_ballPosition.x= balpos;
}

void BloxProcessor::setPersonPosition(double personpos)
{
	QMutexLocker lock(m_propertyMutex);
	m_personPosition.x= personpos;
}



////GUI RELATED:
void BloxProcessor::setBallRollSpeed(int speed) 
{
	QMutexLocker lock(m_propertyMutex);
	if (speed>20 && speed<=100)
	{
		m_ballRollSpeed= speed;
		m_bloxCommandos.setSpeed(speed);
		emit(ballRollSpeedChanged(speed));
	}
}

void BloxProcessor::setPopupSkipRate(int skiprate)
{
	QMutexLocker lock(m_propertyMutex);
	if (skiprate>=0)
	{
		m_popupSkipRate = skiprate;
		emit popupSkipRateChanged(skiprate);
	}
}

void BloxProcessor::setShowPopup(bool b)
{
	QMutexLocker lock(m_propertyMutex);
	if (!getInitialised())
	{
		initialiseSoundAndWidget();
	}

	m_showPopup = b;
	if (b)
	{
		m_plvannotationwidget.createPopUp();	
	}
	else
	{
		m_plvannotationwidget.deinit();
	}
	//is allready inherently set to true in the methods in plvmouselick related to createPopUp
	//if false probable also 
	m_plvannotationwidget.setPopUpExists(b);
	emit(showPopupChanged(b));
}


///DEAL WITH GUI VARIABLES IN PLV:
void BloxProcessor::setFaceDetected(bool b) 
{
	QMutexLocker lock(m_propertyMutex);
	
	//when manually clicking whether a head is seen or not also keep track of grabbing attention.
	//LASTMIN CHANGE, should be same signal rules as normally shouldn't it? So why not use the same timers etc.
	//if (getOverrideFaceDetected())
	//{
	//	if(!getBeforeStart())
	//	{
	//		//when head was seen before but no longer send a attentiongrabbing behavior cue....
	//		if (m_faceDetected && !b)
	//		{
	//			attentionGrabbingBehavior(1);
	//			if (m_intervaltimer.elapsed() >0)
	//				m_intervaltimer.restart();
	//			else
	//				m_intervaltimer.start();
	//		}
	//	}
	//}

	m_faceDetected  = b; 
	emit(faceDetectedChanged(b));
}


void BloxProcessor::setPathStartSound(QString s) //{m_pathStartSound = s; emit(pathStartSoundChanged(s));}
{
	QMutexLocker lock(m_propertyMutex);
	m_pathStartSound = s; 
	if (QFile(m_pathStartSound).exists())
	{
		//delete the old sound object to prevent memory leak
		if (p_startsound != NULL)
		{
			//TODO both delete and deleteLater fail
			//result in memory leaks
			p_startsound->deleteLater();
			//p_startsound = NULL;
			//p_startsound->deleteLater();
			//p_startsound->d_ptr();
		}
		p_startsound = new QSound(m_pathStartSound,this);
		/*if (m_sounds.size() >0)
			m_sounds.at(0) = */
	}
	emit(pathStartSoundChanged(s));
}

void BloxProcessor::setMessageDuration(int i)
{
	QMutexLocker lock(m_propertyMutex); 
	if (i>0) 
	{
		if(!m_firstrun)
		{
			//TO fix:  not thread safe, it can be set and accesed in gui and used at the same time in sending a string
			m_bloxCommandos.setDuration(i);
		}
		m_messageDuration = i;
	} 
	emit(messageDurationChanged(i));
}


void BloxProcessor::setPathLookAwaySound(QString s)// {m_pathLookAwaySound = s; emit(pathLookAwaySoundChanged(s));}
{
	QMutexLocker lock(m_propertyMutex);
	m_pathLookAwaySound = s; 
	if (QFile(m_pathLookAwaySound).exists())
	{
		//delete the old sound object to prevent memory leak
		if (p_lookawaysound!=NULL)
		{
			p_lookawaysound->deleteLater();
		}
		p_lookawaysound = new QSound(m_pathLookAwaySound,this);
		
	}
	emit(pathLookAwaySoundChanged(s));
}

void BloxProcessor::setPathReminderSound(QString s)// {m_pathReminderSound = s; emit(pathReminderSoundChanged(s));}
{
	QMutexLocker lock(m_propertyMutex);
	m_pathReminderSound = s; 
	if (QFile(m_pathReminderSound).exists())
	{
		//delete the old sound object to prevent memory leak
		if (p_remindersound != NULL)
		{
			p_remindersound->deleteLater();
		}
		p_remindersound = new QSound(m_pathReminderSound,this);	
	}
	emit(pathReminderSoundChanged(s));
}

void BloxProcessor::setOverrideString(bool b)
{
	QMutexLocker lock(m_propertyMutex);
	if(b) 
	{
		m_overrideString = true;
	}
	else
	{
		m_overrideString = false;
		//setSomeString("");
	}

	emit(overrideStringChanged(b));
}


void BloxProcessor::setPersonOverruledByMouse(bool b)
{
	QMutexLocker lock(m_propertyMutex);
	if(b) 
	{
		qDebug() << "overuledbymouse";
		m_personOverruledByMouse = true;
		//also set it in the widget
		m_plvannotationwidget.setMouseClickedPerson(true);
	}
	else
	{
		m_personOverruledByMouse = false;
		m_plvannotationwidget.setMouseClickedPerson(false);
	}

	emit(personOverruledByMouseChanged(b));
}

void BloxProcessor::setBallOverruledByMouse(bool b)
{
	QMutexLocker lock(m_propertyMutex);
	if(b) 
	{
		qDebug() << "overuledbymouse";
		m_ballOverruledByMouse = true;
		m_plvannotationwidget.setMouseClickedBall(true);
	}
	else
	{
		m_ballOverruledByMouse = false;
		m_plvannotationwidget.setMouseClickedBall(false);
	}

	emit(ballOverruledByMouseChanged(b));
}



void BloxProcessor::setPlayTrigger(bool b)
{
	//TODO this is nonsense, simply use another enum/or int to play sound 1,2,3 and play on trigger

	QMutexLocker lock(m_propertyMutex);
	// for manual trigger we use this boolean checkbox, 
	//we can either use a timer to decide on what trigger to play after init
	//or a manual way so from off to on is slight grabbing and from on to off is impressive, however this would require both to happen
	//the init is always automatically at startup for now

	//the interval timer is also started here, if nothing is to be done withit you can always set the threshold to a very low interval
	//so this is setting the timeIntervalToRemindAgain to -1 or 0 or so;
	if(!getBeforeStart() && !getFirstRun())
	{
		if(b) 
		{
			playBloxSound(1);
		}
		else
		{
			playBloxSound(2);
		}
	}
	else if (!getBeforeStart() && getFirstRun() && !getInitialised())
	{
		//not yet created the sounds in init
		initialiseSoundAndWidget();
		playBloxSound(0);
	}
	else if (getBeforeStart())
	{
		//playBloxSound(0);
	}

	m_playTrigger = b;
	
	emit(playTriggerChanged(b));
}
