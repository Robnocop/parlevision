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
        m_showPopup(true),
        m_someString("hello"),
		m_playTrigger(true),
		m_overrideString(false),
		m_overruledByMouse(false),
		m_faceDetected(false), 
		m_overrideFaceDetected(false),
		m_timeWithoutHeadThreshold(3000), //in milliseconds
		m_framesAveragingPosition(15),
		m_timeIntervalToRemindAgain(30000),
		m_minPosCorrection(0.5),
		m_pixelsDiffPersonBall(50),
		m_factorMetersMovementToPixels(640),
		m_pathStartSound("mysounds/hallo.wav"),
		m_pathLookAwaySound("mysounds/he.wav"),
		m_pathReminderSound("mysounds/Star_Wars_R2-D2_Determined.wav")
{
    m_inputPin = createCvMatDataInputPin("input", this);
    m_inputPinS2 = plv::createInputPin<QString>("input head rotation", this);
	m_inputPinS = plv::createInputPin<QString>("input head position", this);
	plv::InputPin< QList<plvblobtracker::BlobTrack> >* m_inputBlobs;

	//m_inputPinSB = plv::createInputPin<QString>("input ball position", this);
	m_inputBlobs = createInputPin< QList<plvblobtracker::BlobTrack> >( "input", this );

	m_outputPin = createCvMatDataOutputPin("output", this);
	m_outputPin2 = createOutputPin<QString>( "text", this);
	
	m_inputPin->addSupportedChannels(1);
	m_inputPin->addSupportedChannels(3);

    m_inputPin->addSupportedDepth(CV_8U);
    m_inputPin->addSupportedDepth(CV_16U);


}

BloxProcessor::~BloxProcessor()
{
}



QString BloxCommand::getString(){
	QString commandoBal = "";
	if(!grabAttention)
	{
		if (direction == 1)
		{
			commandoBal = QString("F1,%1,%2").arg(speed).arg(duration);
		}
		else if (direction == -1)
		{
			commandoBal = QString("B1,%1,%2").arg(speed).arg(duration);
		}
		else 
		{
			commandoBal = "S";
		}
	}
	else
	{
		commandoBal = QString("W,%1").arg(durationWiggle);
	}
	return commandoBal;
}

bool BloxProcessor::init()
{
	//attempt to play an empty pointer is quite bad, so init first:
	//set default sounds according to the accompanying fields
	//p_startsound = &andergeluid;
	/*p_startsound = new QSound("mysounds/he.wav",this);
	delete p_startsound;
	p_startsound = new QSound("mysounds/hallo.wav",this);*/

	p_startsound = new QSound(getPathStartSound(),this);
	p_lookawaysound = new QSound(getPathLookAwaySound(),this);
	p_remindersound = new QSound(getPathReminderSound(),this);

	/*m_sounds.clear();
	m_sounds.push_back(QSound(getPathStartSound(),this));
	m_sounds.push_back(QSound(getPathLookAwaySound(),this));
	m_sounds.push_back(QSound(getPathReminderSound(),this));*/
	
	
	m_balPosition.x= 0;
	m_balPosition.y = 0;
	m_personPosition.x=0;
	m_personPosition.y=0;

	//bloxcommandos:
	m_bloxCommandos.direction = 0;
	m_bloxCommandos.duration = 750;
	m_bloxCommandos.speed = 60;
	m_bloxCommandos.grabAttention = false;
	//TODO set in gui?
	m_bloxCommandos.durationWiggle = 1200;
	
	//assume center ==0 and maximum -0.5 so correction to all positive will be around 0.5meters
	//max negative pos... now retrievable in gui with getMinPosCorrection
	//m_middlecorrection = 0.5; 
	
	//assume -.5m to .5m --> 640 pixels , now retrievable in gui with getFactorMetersMovementToPixels()
	//m_factor = 640; 
	
	//threshold to send a command to the ball in pixels, now retrievable from gui with getPixelsDiffPersonBall()
	//m_threshold_posdiff = 50;
	
	//set pop up after initialisation makes more sense than at init 
	//setShowPopup(false);
	
	//setOverrideFaceDetected(false);
	
	//popup creation.
	m_plvannotationwidget.init();
	setShowPopup(getShowPopup());
	//some temporary parameters for testing
	//m_averagesizelist = 15;
    
	//m_lastmovement.start();

	//see what happends if elapsed is asked without starting the timer, does a null timer return 0/null or an error?
	//qDebug() << "null timer == " << m_lastmovement.elapsed();
	//not null...
	
	//initialise sounds is weird,
	//I want to be able to assign them with several string fields
	//now using a pointer 
	/*m_startsound = QSound("mysounds/hallo.wav");
	m_lookawaysound = QSound("mysounds/he.wav");	
	m_remindersound = QSound("mysounds/Star_Wars_R2-D2_Determined.wav");*/

	

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
	
	//**ugly solution, perhaps qelapsedtimer is more appropriate 
	//we want to be sure the timer is started and we need to restart it, 
	//in case start has been called twice it is uncertain if it will reset 
	//we need to restart in case the pipeline has been started before and is now stopped for instance
	m_lastheadfocus.start();
	m_lastheadfocus.restart();
	setFaceDetected(false);

	m_firstrun = true;
    return true;
}

bool BloxProcessor::stop()
{
	//TODO destory the timers? m_lastheadfocus.
	//m_lastmovement = NULL;
    return true;
}

void BloxProcessor::setShowPopup(bool b)
{
	m_showPopup = b;
	if (b)
	{
		m_plvannotationwidget.createPopUp();	
	}
	else
	{
		m_plvannotationwidget.deinit();
	}
	m_plvannotationwidget.m_popUpExists = b;
	emit(showPopupChanged(b));
}

void BloxProcessor::setPathStartSound(QString s) //{m_pathStartSound = s; emit(pathStartSoundChanged(s));}
{
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

void BloxProcessor::setPathLookAwaySound(QString s)// {m_pathLookAwaySound = s; emit(pathLookAwaySoundChanged(s));}
{
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
	if(b) 
	{
		m_overrideString = true;

	}
	else
	{
		m_overrideString = false;
		setSomeString("");
	}

	emit(overrideStringChanged(b));
}


void BloxProcessor::setOverruledByMouse(bool b)
{
	if(b) 
	{
		qDebug() << "overuledbymouse";
		m_overruledByMouse = true;
		//folder libs as parrent so ; /parlevision/libs/mysounds/x.wav --> "mysounds/bells.wav"
		//QSound::play("mysounds/bells.wav");
	}
	else
	{
		m_overruledByMouse = false;
		m_plvannotationwidget.m_mouseclicked = false;
	}

	emit(overruledByMouseChanged(b));
}


void BloxProcessor::setPlayTrigger(bool b)
{
	// for manual trigger we use this boolean checkbox, 
	//we can either use a timer to decide on what trigger to play after init
	//or a manual way so from off to on is slight grabbing and from on to off is impressive, however this would require both to happen
	//the init is always automatically at startup for now

	//the interval timer is also started here, if nothing is to be done withit you can always set the threshold to a very low interval
	//so this is setting the timeIntervalToRemindAgain to -1 or 0 or so;
	if(!m_firstrun)
		{
		if(b) 
		{
			if (!m_intervaltimer.elapsed() >0)
				m_intervaltimer.start();
			
			m_playTrigger = true;
		
			//which trigger will be played can be set with the intervaltime, if a short interval e.g. 1ms it will always play the case 2 perhaps 0ms is so low it will even play on the first try, if set very high it could always play the case 1 sound,
			//1 is the least invasive, normally when someone was watching but not in the last few seconds,
			//2 is more invasive, normally played after a long time in which someone is not looking.
			if (!m_firstrun && m_intervaltimer.elapsed()>getTimeIntervalToRemindAgain())
				playBloxSound(2);
			else
				playBloxSound(1);
			//restart timer;
			m_intervaltimer.restart();
		}
		else
		{
			m_playTrigger = false;
			playBloxSound(2);
		}
	}
	
	emit(playTriggerChanged(b));
}

void BloxProcessor::attentionGrabbingBehavior(int behaviortype)
{
	//etPlayTrigger(true);
	playBloxSound(behaviortype);
	//TODO send some command to the ball

	m_bloxCommandos.grabAttention = false;
}

void BloxProcessor::playBloxSound(int songnr)
{
	//folder libs as parrent so ; /parlevision/libs/mysounds/x.wav --> "mysounds/bells.wav"
	switch (songnr)
	{
			//playing sounds is not that straighforward, only one can be played at a time it seems, one starts ands top after the second starts
			//perhaps TODO implement the Phonon Module http://qt-project.org/doc/qt-4.8/phonon-module.html
			//but this requires a full fledged rebuild!  QT += phonon
			//"Applications that use Phonon's classes need to be configured to be built against the Phonon module. The following declaration in a qmake project file ensures that an application is compiled and linked appropriately:"
			//perhaps threading would help? as we did gfor the kinect facetracker?
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
			//m_startsound.setLoops(2);
			//old: other options to play a sound:
			//QSound::play("mysounds/hallo.wav");
			//m_startsound.play();
			
			if (p_startsound != NULL)
			{		
			   p_startsound->play();
			}
			else
				qDebug() << "NULL POINTER start";
			break;

			case 1:
			
			if (p_lookawaysound != NULL)
			{	
				//qDebug() <<  "looked away: play lookaway sound , " << p_lookawaysound->loopsRemaining();
				p_lookawaysound->play();
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

//perhaps needed later on for fps correction
//get the current processingserial();
	//unsigned int serial = this->getProcessingSerial();
	//unsigned int serialInInt = this->getProcessingSerial();
	//QString serialIntInString = QString("%1").arg(serialInInt);
	
    // update our "frame counter"
    //this->setSomeInt(this->getSomeInt()+1);

bool BloxProcessor::process()
{
	bool headinsceneinframe =false;
	bool balistracked = false;

	//deal with mouseclicks in the gui
	if (m_plvannotationwidget.m_mouseclicked)
	{
		if(!getOverruledByMouse())
			setOverruledByMouse(true);
	}

	//set boolean of showpopup according to whether the window is closed or not in the gui windows
	if (!m_firstrun && m_plvannotationwidget.m_popUpExists != getShowPopup())
	{
		setShowPopup(m_plvannotationwidget.m_popUpExists);
	}

	//get the position of the ball from a normal camera:

	//if via Strings.....
	//QString srcSB = m_inputPinSB->get();
	////get a stringlist containing all the tracks
	//QStringList tracks = srcSB.split("\n");
	////get a stringlist with the info of the tracks
	////a vector of unsigned ints would have been neater
	//QList<QStringList> trackinformation;
	//for( int i=0; i<tracks.count(); ++i )
	//{ 
	//	//the data we get from blobtrack:
	//	/*inline unsigned int getId() const { return d->id; }
	//	inline unsigned int getDirection() const { return d->direction; }
	//	inline unsigned int getVelocity() const { return d->velocity; }
	//	inline unsigned int getAvgVelocity() const { return d->avgvelocity; }
	//	inline unsigned int getAvgDirection() const { return d->avgdirection; }
	//	inline unsigned int getLastUpdate() const { return d->lastUpdate; }
	//	inline unsigned int getTimeSinceLastMeasurement() const {return d->timesincelastmeasurement; }
	//	inline unsigned int getAveragePixel() const {return d->averagepixel; }
	//	inline unsigned int getBlobSize() const {return d->blobsize;}*/

	//	//in the format specified in tracktostringconverter:
	//	//QString blobTabString = QString("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t %7 \%8").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel()).arg(t.getAge()).arg(t.getBlobSize()); 
	//	
	//	QStringList track = tracks.at(i).split("\t");
	//	if (track.count()==8)
	//		trackinformation.push_back(track);
	//	else
	//		qDebug()<< "track information is incorrect" << track.count();
	//	// process items in numerical order by index
	//	// do something with "list[i]";
	//}

	QList<plvblobtracker::BlobTrack> tracks = m_inputBlobs->get();
	
	//int ballpos = srcSB.toInt();

	//use the input of the kinect position of the head
	//input is a tabdelimited xyz pos
	QString srcS = m_inputPinS->get();
	QString srcRotationPin = m_inputPinS2->get();
	//split with tabs
	QStringList xyzpos = srcS.split("\t");
	float xposframe = 0.0f;
	//for now we only check if a head is detected, so a boolean, future version might need actual rotation in x or y.
	QStringList srcRotation = srcRotationPin.split("\t");
	bool rotationHead = false;
	
	//dont grab attention unless we just start, didn't have a face for the first time (looked away), or if we want to grabattention again, so set to false unless we actually want to grab attention for these reasons
	// we need this to distinguish from the other type of commands we can send to the ball etcetra.
	m_bloxCommandos.grabAttention = false;

	//check for correct input format of the kinect data in string format
	if (srcRotation.length()==3)
	{
		//have different behavior in first run independt whether a face is recognised or not.
		if (m_firstrun)
		{
			m_firstrun =false;
			attentionGrabbingBehavior(0);
		}
		else if (srcRotation.at(0).toFloat() != 0.0)
		{
			headinsceneinframe = true;
			if (m_lastheadfocus.elapsed() >0)
			{
				//ugly solution, just reset the time since a head is seen to 0,  this will work for now
				m_lastheadfocus.restart();
				//reset that a head is seen if this was not yet set so and if the manual annotation does not prohibit to do so.
				if (!getOverrideFaceDetected() && !getFaceDetected())
				{
					setFaceDetected(true);
					//we also need to set the timer of the last send command to move the ball to position of the player as it might have been neglected due to the focus of the player
					m_lastmovement.restart();
				}
			}
			else
			{
				//always have the timer running will make behavior more predictable
				m_lastheadfocus.start();
			}
		}
		//if the head is not seen, it is not directed at the ball after initial turning for longer than the threshold time: grab their attention
		//or if that was alrready the case, then grab their attention every interval again .
		else
		{
			//TODO check if this is needed as the elapsed will be bigger than 0 even if it has not been started it seems.
			//every single time it starts with facedetected being false
			if (m_lastheadfocus.elapsed() >0)
			{
				
				//respond to that the head is not there, and grab attention first time and after every interval afterwardsallready started and counting the seconds when the head was seen
				if (m_lastheadfocus.elapsed() > getTimeWithoutHeadThreshold())
				{
					//if a face was looking at the kinect but now turning away. So if a face was seen before and not now (for more than the look away threshold)
					//slightly grab attention and behave accordingly and start the interval timer
					//at least if not manual override of the behavior, otherwhise
					if (getFaceDetected()&&!getOverrideFaceDetected())
					{
						setFaceDetected(false);
						//do the interactive stuff method:
						//a subtle method as it is triggered quite often
						attentionGrabbingBehavior(1);
						if (m_intervaltimer.elapsed() >0)
							m_intervaltimer.restart();
						else
							m_intervaltimer.start();
					}
					else if (!getFaceDetected() && getOverrideFaceDetected())
					{

					}
					//only perform the more immersive attention grabbing behavior with a certain interval
					else if(!getFaceDetected() && m_intervaltimer.elapsed()>getTimeIntervalToRemindAgain())
					{
						attentionGrabbingBehavior(2);
						if (m_intervaltimer.elapsed() !=0)
							m_intervaltimer.restart();
						else
							m_intervaltimer.start();
					}
				}
				
			}
			else
			{
				m_lastheadfocus.start();
			}
			//qDebug() << m_lastmovement.
		}
	}

	//if the head indeed has been transmitted correctly with tabs, otherwise a crash would occur later on 
	if (xyzpos.length()==3)
	{
		xposframe = xyzpos.at(0).toFloat();
		//show the values
		//qDebug() << "split" << xyzpos.at(0) << "2nd val " << xyzpos.at(1) << "3rdval" << xyzpos.at(2);
		
		//TODO takeinto account momentary values of 0 are kind of useless better use the last known positions only.
		//average over size of the list of positions so delete the if there are more coming in
		if (xposframe!= 0.0 && m_possitionsarray.size()>getFramesAveragingPosition() && m_possitionsarray.size()>0)
		{
			m_possitionsarray.removeLast();
		}
		if (xposframe!= 0.0) 
		{
			m_possitionsarray.push_front(xposframe);
		}
		
		//float foraverage = m_personPosition.x;
		float foraverage = 0;
		for(int i=0; i<m_possitionsarray.count(); ++i)
		{
			foraverage = foraverage+m_possitionsarray[i];
		}
		
		//calculate the avarage value
		//check for average of 0: this allows for manual steering of the ball when no face was detected in the last 15 frames;
		if (foraverage!=0.0)
		{
			m_personPosition.x = ((foraverage/m_possitionsarray.count())+getMinPosCorrection()) * getFactorMetersMovementToPixels() ;
			//set the to be drawn ball to this value:
			//TODO take into account that the mousclick should be able to override
			if (!getOverruledByMouse())
			{
				m_plvannotationwidget.m_point_wanted.x = (int) m_personPosition.x;
				//qDebug() << "set pos to" << (int) m_personPosition.x;
			}
		}
		//qDebug() << "avg person positon" <<m_personPosition.x;
	}
	else
	{
		//TODO check if warning behaves similar to a qDebug()
		qWarning() << "no split, length, check if the right input of head tracking in tabdelimited format is used " << xyzpos.length();
	}



    assert(m_inputPin != 0);
    assert(m_outputPin != 0);
	//get the image
    CvMatData src = m_inputPin->get();
	//set the current pos according to the widget
	m_balPosition.x = m_plvannotationwidget.m_point_assumed.x;
	
	int lastdirection = m_bloxCommandos.direction;
	//if difference between wanted and current pos is big enough move the ball to that direction
	if ( m_plvannotationwidget.m_point_wanted.x-m_plvannotationwidget.m_point_assumed.x > getPixelsDiffPersonBall())
	{
		m_bloxCommandos.direction = 1;
	}
	else if (m_plvannotationwidget.m_point_assumed.x-m_plvannotationwidget.m_point_wanted.x > getPixelsDiffPersonBall())
	{
		m_bloxCommandos.direction = -1;
	}
	else
	{
		m_bloxCommandos.direction = 0;
	}
	
		//get the ball protocol correct String
	QString commandString = "-" ;
	
	//only send a command if it is in an opposite direction or if the ball has not moved for some time
	if (lastdirection!=m_bloxCommandos.direction)
	{
		if (m_lastmovement.elapsed() >0)
			m_lastmovement.restart();
		else
			m_lastmovement.start();
		//qDebug() << m_lastmovement.
	}

	//only actually move if the player is looking at the screen
	//if (m_lastmovement.elapsed()>600 && !m_bloxCommandos.grabAttention)
	if (m_lastmovement.elapsed()>600 && getFaceDetected())
	{
		commandString = m_bloxCommandos.getString();
		if (m_lastmovement.elapsed() >0)
			m_lastmovement.restart();
		else
			m_lastmovement.start();
	}

	//simply override this in case we need an attentiongrabbing behavior:
	//if ()
	//{
		//do something with the commandstring according to attentiongrabbing behavior?
	//}

	//output the text with message to the ball
	//override text with text from string input after boolean is clicked
	//for now get the current text by showing the frame number
	//QString texttoshow = QString::number(getSomeInt());
	if (getOverrideString())
	{
		commandString = getSomeString();
		setOverrideString(false);
	}
	m_outputPin2->put(commandString);
		
    //point to data in src
    cv::Mat& in = src;
	int extraWidth  = 0;
    int extraHeight = 200;
	
	//allocate a target buffer
	//inlcude a bottom screen to show the assumed position of the ball and be able to rectify this by clicking on it
	CvMatData target;
    target.create( src.width()+extraWidth, src.height()+extraHeight, src.type() );
	cv::Mat& out2 = target;
	//for now just copy the input to the output
	in.copyTo(out2);

	//old way create the actual size mat and for oop values int o it
	//plv::CvMatData outImg = CvMatData::create( src.width()+extraWidth, src.height()+extraHeight, src.depth(), src.channels() );
	//make pointer to data from cvmat
	//cv::Mat& out3 = outImg;
    // set output image to black
    //out3.setTo(0);

	//instead vconcat the mats so create the empty stuff:
	cv::Mat ballArea(cv::Size(src.width()+extraWidth,extraHeight),CV_8UC3);
	ballArea.setTo(0);
	cv::vconcat(in, ballArea, out2);
    
	//copyImgInto(in, outImg, 0,0);
	//copyImgInto(in, out3, 0,0);
	
	int radius = 50;
	//??
	
	//draw the points of the assumed and the wanted position. 
	//qDebug() << "point x " << m_plvannotationwidget.m_point.x;
	//perhaps set all the values in the plvmouseclick for proper handling of mouseclick areas:
	m_plvannotationwidget.m_minwantedy=src.height();
	m_plvannotationwidget.m_maxwantedy=src.height()+extraHeight/2;
	m_plvannotationwidget.m_maxassumedposy = src.height()+extraHeight;

	cv::rectangle(out2, cvPoint(0,src.height()), cvPoint(src.width(),m_plvannotationwidget.m_maxwantedy), CV_RGB(150,150,150), -1, 8, 0);
	//longer than threshold:
	//if (getFaceDetected())
	if(headinsceneinframe)
		cv::circle(out2, cvPoint(m_plvannotationwidget.m_point_wanted.x,src.height()+radius), radius, CV_RGB(200,0,0), -1);
	else if (getFaceDetected())
		cv::circle(out2, cvPoint(m_plvannotationwidget.m_point_wanted.x,src.height()+radius), radius, CV_RGB(150,50,50), -1);
	else
		cv::circle(out2, cvPoint(m_plvannotationwidget.m_point_wanted.x,src.height()+radius), radius, CV_RGB(0,0,200), -1);
	
	cv::circle(out2, cvPoint(m_balPosition.x,src.height()+extraHeight/2+radius), radius, CV_RGB(200,0,0), -1);

	// publish the new image
	m_outputPin->put( out2 );

	//popup
	//evt m_plvannotationwidget.setWindowTitle(windowname);
	//paint the popup if there is one
	if (getShowPopup())
	{
		m_plvannotationwidget.processInit();
		//m_plvannotationwidget.toPaint(target);
		m_plvannotationwidget.toPaint(target);
	}
    return true;
}


