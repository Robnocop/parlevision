#ifndef BLOXPROCESSOR_H
#define BLOXPROCESSOR_H

#include <QObject>
#include <plvcore/PipelineProcessor.h>
#include <plvcore/RefPtr.h>
#include <plvcore/Pin.h>
#include <plvcore/PlvMouseclick.h>
#include <QPointer>

//we need the info from blobtrack now..
//requires plv_blobtracker_plugin.dll;
//#include "../plvblobtracker/Blob.h"
//#include "../plvblobtracker/BlobTrack.h"
//using namespace plvblobtracker;

//#include <QtMultimedia/QAudioOutput>

using namespace plv;

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

//features from the last send command
class BloxCommand
{
	public:
		
		//in this case we dont need it to be recursive but it has most chance of working as it needs to unlock on return, which is hard to code otherwise
		/** mutex used for properties. Properties need a recursive mutex
          * sice the emit() they do to update their own value can return the
          * call to the set function resulting in a deadlock if we use a normal
          * mutex */
		//m_propertyMutex( new QMutex( QMutex::Recursive ) )
      	mutable QMutex m_propertyMutex2;
		
		//returns the last string in a format that is recognisable for the ball.
		QString getString();

		//represent the direction the ball has been ordered to move in -1 == left, 0 == stop, 1 ==right
		int getDirection() { m_propertyMutex2.lock(); int i = direction; m_propertyMutex2.unlock(); return i; };
		void setDirection(int i) {m_propertyMutex2.lock(); direction = i; m_propertyMutex2.unlock();};
		
		//the duration this movement will be keeping on
		//for bodymovement a top of 2 Hz for rocking behavior has been mentioned so around 30fps --> <15 fps in order to get a proper reading, however the lag in actuation prevents such a speed
		int getDuration() { m_propertyMutex2.lock(); int i = duration; m_propertyMutex2.unlock(); return i; }
		void setDuration(int i) {m_propertyMutex2.lock(); if (i>0) {duration = i;}; m_propertyMutex2.unlock();};
		
		int getDurationWiggle() { m_propertyMutex2.lock(); int i = durationWiggle; m_propertyMutex2.unlock(); return i; }
		void setDurationWiggle(int i) {m_propertyMutex2.lock(); if (i>0) {durationWiggle = i;}; m_propertyMutex2.unlock();};

		//the speed the motors have to move with ranging from 0-100, minimum around 20 in order to actually move
		int getSpeed() { m_propertyMutex2.lock(); int i = speed; m_propertyMutex2.unlock(); return i; }
		void setSpeed(int i) {m_propertyMutex2.lock(); if (i>0) {speed= i;}; m_propertyMutex2.unlock();};
		
		//indicates if attention grabbing behavior is wanted or normal movement
		int getGrabAttention() { m_propertyMutex2.lock(); bool b = grabAttention; m_propertyMutex2.unlock(); return b; }
		void setGrabAttention(bool b) {m_propertyMutex2.lock(); grabAttention = b; m_propertyMutex2.unlock();};
		
		//

	private:
		int duration;
		int direction;
		int speed;
		int durationWiggle;
		bool grabAttention;
		
		
};

class BloxProcessor : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( BloxProcessor )

    Q_CLASSINFO("author", "Robby van Delden")
    Q_CLASSINFO("name", "BloxProcessor")
	 Q_CLASSINFO("description", "A processor for the interactive Blox ball for Dichterbij by Kitt Engineering." 
								"\r\n factorMaxball transforms the biggestblob in pixels to center of circle position of the blob: ball.x=track.at(biggestblob).x*factorMaxball;"
								"\r\n factorMetersMovementToPixels transforms meters to pixels, so from 1m to 640p --> fmmtp=640, assuming a max of 1m"
								"\r\n minposcorrection is the minimum value, so for 1m symmetric movement set to 0.5 with fmmtp to 640"
								"\r\n pixelsdiffpersonball is the threshold in pixels in the drawn image to send the ball moving"
								"\r\n to manual send the ball click where the head (upper rectangle) and ball(rectangle underneath) should be, select overridefacedetected and indicate a face is detected, and/or play the trigger manually"
								)


    //TODO make time and thus a float instead of frames
	Q_PROPERTY( double timeWithoutHeadThreshold READ getTimeWithoutHeadThreshold WRITE setTimeWithoutHeadThreshold NOTIFY timeWithoutHeadThresholdChanged  )
	Q_PROPERTY( double timeIntervalToRemindAgain READ getTimeIntervalToRemindAgain WRITE setTimeIntervalToRemindAgain NOTIFY timeIntervalToRemindAgainChanged  )
	Q_PROPERTY( int messageDuration READ getMessageDuration WRITE setMessageDuration NOTIFY messageDurationChanged  ) 
	Q_PROPERTY( int sendMessageInterval READ getSendMessageInterval WRITE setSendMessageInterval NOTIFY sendMessageIntervalChanged  ) 
	
	Q_PROPERTY( QString someString READ getSomeString WRITE setSomeString NOTIFY someStringChanged )

	Q_PROPERTY( double minPosCorrection READ getMinPosCorrection WRITE setMinPosCorrection NOTIFY minPosCorrectionChanged  )
	Q_PROPERTY( double factorMetersMovementToPixels READ getFactorMetersMovementToPixels WRITE setFactorMetersMovementToPixels NOTIFY factorMetersMovementToPixelsChanged  )
	Q_PROPERTY( double factorMaxball READ getFactorMaxball WRITE setFactorMaxball  NOTIFY factorMaxballChanged  )
	Q_PROPERTY( int pixelsDiffPersonBall READ getPixelsDiffPersonBall WRITE setPixelsDiffPersonBall NOTIFY pixelsDiffPersonBallChanged  ) 
	
	Q_PROPERTY( int framesAveragingPosition READ getFramesAveragingPosition WRITE setFramesAveragingPosition NOTIFY framesAveragingPositionChanged  )
	Q_PROPERTY( int ballRollSpeed READ getBallRollSpeed WRITE setBallRollSpeed NOTIFY ballRollSpeedChanged )
	Q_PROPERTY( int popupSkipRate READ getPopupSkipRate WRITE setPopupSkipRate NOTIFY popupSkipRateChanged )
	Q_PROPERTY( bool showPopup READ getShowPopup WRITE setShowPopup NOTIFY showPopupChanged  )
   	Q_PROPERTY( bool playTrigger READ getPlayTrigger WRITE setPlayTrigger NOTIFY playTriggerChanged  )
	Q_PROPERTY( bool overrideString READ getOverrideString WRITE setOverrideString NOTIFY overrideStringChanged  )
	Q_PROPERTY( bool personOverruledByMouse READ getPersonOverruledByMouse WRITE setPersonOverruledByMouse NOTIFY personOverruledByMouseChanged  )
	Q_PROPERTY( bool ballOverruledByMouse READ getBallOverruledByMouse WRITE setBallOverruledByMouse NOTIFY ballOverruledByMouseChanged  )

	
	//create two popups for headorientation tracking, 
	//one showing the override 
	//and the other showing whether it currently is seeing a face of a person, perhaps this cna be combined with a regular viola jones tracker
	//will only change after it is bigger than the threshold time that is set, otherwise it will be flickering all the time
	Q_PROPERTY( bool faceDetected READ getFaceDetected WRITE setFaceDetected NOTIFY faceDetectedChanged  )
	Q_PROPERTY( bool overrideFaceDetected READ getOverrideFaceDetected WRITE setOverrideFaceDetected NOTIFY overrideFaceDetectedChanged  )

	Q_PROPERTY( bool virtualBallCommands READ getVirtualBallCommands WRITE setVirtualBallCommands NOTIFY virtualBallCommandsChanged  )
	//the paths for the sounds
	Q_PROPERTY( QString pathStartSound READ getPathStartSound WRITE setPathStartSound NOTIFY pathStartSoundChanged )
	Q_PROPERTY( QString pathLookAwaySound READ getPathLookAwaySound WRITE setPathLookAwaySound NOTIFY pathLookAwaySoundChanged )
	Q_PROPERTY( QString pathReminderSound READ getPathReminderSound WRITE setPathReminderSound NOTIFY pathReminderSoundChanged )
	
    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR

public:
    BloxProcessor();
    virtual ~BloxProcessor();

    /** these methods can be overridden if they are necessary for
        your processor */
    virtual bool init();
    virtual bool deinit() throw();
    virtual bool start();
    virtual bool stop();

    /** propery methods */
    double getTimeWithoutHeadThreshold() { QMutexLocker lock(m_propertyMutex); return m_timeWithoutHeadThreshold; }
	double getTimeIntervalToRemindAgain() { QMutexLocker lock(m_propertyMutex); return m_timeIntervalToRemindAgain; }
	int getSendMessageInterval() { QMutexLocker lock(m_propertyMutex); return m_sendMessageInterval; }
	int getMessageDuration() { QMutexLocker lock(m_propertyMutex); return m_messageDuration; }

	double getFactorMetersMovementToPixels() {QMutexLocker lock(m_propertyMutex); return m_factorMetersMovementToPixels; }
	double getFactorMaxball () { QMutexLocker lock(m_propertyMutex); return m_factorMaxball ; } 
	double getMinPosCorrection() {QMutexLocker lock(m_propertyMutex); return m_minPosCorrection;}
	int getPixelsDiffPersonBall() { QMutexLocker lock(m_propertyMutex); return m_pixelsDiffPersonBall; }
		
	int getBallRollSpeed() {QMutexLocker lock(m_propertyMutex); return m_ballRollSpeed; }
	int getPopupSkipRate() {QMutexLocker lock(m_propertyMutex); return m_popupSkipRate; }

	int getFramesAveragingPosition() { QMutexLocker lock(m_propertyMutex); return m_framesAveragingPosition; }
    bool getShowPopup() { QMutexLocker lock(m_propertyMutex); return m_showPopup; }
	
	bool getPlayTrigger() {QMutexLocker lock(m_propertyMutex); return m_playTrigger;}
	bool getPersonOverruledByMouse() {QMutexLocker lock(m_propertyMutex); return m_personOverruledByMouse;}
	bool getBallOverruledByMouse() {QMutexLocker lock(m_propertyMutex); return m_ballOverruledByMouse;}
	bool  getOverrideString() {QMutexLocker lock(m_propertyMutex); return m_overrideString;}
	QString getSomeString() { QMutexLocker lock(m_propertyMutex); return m_someString; }
	QString getPathStartSound() { QMutexLocker lock(m_propertyMutex); return m_pathStartSound; }
	QString getPathLookAwaySound() { QMutexLocker lock(m_propertyMutex); return m_pathLookAwaySound; }
	QString getPathReminderSound() { QMutexLocker lock(m_propertyMutex); return m_pathReminderSound; }

	bool getOverrideFaceDetected() {QMutexLocker lock(m_propertyMutex); return m_overrideFaceDetected;}
	bool getFaceDetected() {QMutexLocker lock(m_propertyMutex); return m_faceDetected;}
	bool getVirtualBallCommands() {QMutexLocker lock(m_propertyMutex); return m_virtualBallCommands;}

	//the recursive mutexlocker is overkill here for setting some global variables during run time;
	bool getFirstRun() {QMutexLocker lock(m_propertyMutex); return m_firstrun;}
	bool getBeforeStart() {QMutexLocker lock(m_propertyMutex); return m_beforestart;}
	bool getInitialised() {QMutexLocker lock(m_propertyMutex); return m_initialised;}

	bool getHeadInCurrentFrame() {QMutexLocker lock(m_propertyMutex); return m_headinframe;}
	double getBallPosition() {QMutexLocker lock(m_propertyMutex); return m_ballPosition.x;}
	double getPersonPosition() {QMutexLocker lock(m_propertyMutex); return m_personPosition.x;}

	/*several behaviors in one method as they are very similar for now/e.g.
	//0 when the ball's program is started
	//1 when head has not been seen for some time
	2 after attentionbehavior has allready been attempted, 
	*/
	void attentionGrabbingBehavior(int type);
	void playBloxSound(int soundnr);
	
	//SOUNDS:
	// this requires creating the objects with "new" so it will not be deleted automatically.
	// so generate them in the stringchange field, 
	// than assign these values to this pointer
	// that would require deleting them before the change of sounds and after deinit
	// another solution could be using a list or vector of sounds, as we know the amount of sounds we will have
	QPointer<QSound> p_startsound; //("mysounds/hallo.wav");
	//QSound* p_lookawaysound;//("mysounds/he.wav");
	QPointer<QSound> p_lookawaysound;//("mysounds/he.wav");
	//QSound* p_remindersound;//("mysounds/Star_Wars_R2-D2_Determined.wav");
	QPointer<QSound> p_remindersound;//("mysounds/Star_Wars_R2-D2_Determined.wav");
	
	//methods used in as steps in process:
	bool woozPersonPosition();
	bool woozBallPosition();
	void woozFaceDetectionReminder();
	void getSetTrackInfoBall();
	void getSetHeadTrackingPosition();
	void getSetHeadRotationTracking();

	QString moveOrStopBall();
	CvMatData drawThePositions(CvMatData src);

	void initialiseSoundAndWidget();

signals:
    void timeWithoutHeadThresholdChanged(double newValue);
	void timeIntervalToRemindAgainChanged(double newValue);
	void sendMessageIntervalChanged(int newvalue);
	void messageDurationChanged(int newvalue);

	void factorMetersMovementToPixelsChanged(double newValue);
	void factorMaxballChanged(double newValue);
	
	void minPosCorrectionChanged(double newValue);
	void pixelsDiffPersonBallChanged(int newValue);	
	
	void framesAveragingPositionChanged(int newValue);

	void ballRollSpeedChanged(int newValue);
	void popupSkipRateChanged(int newvalue);

	void showPopupChanged(bool newValue);
	void overrideStringChanged(bool newVAlue);
    void someStringChanged(QString newValue);
	void playTriggerChanged(bool newValue);
	void personOverruledByMouseChanged(bool newValue);
	void ballOverruledByMouseChanged(bool newValue);
	void overrideFaceDetectedChanged(bool newValue);
	void faceDetectedChanged(bool newValue);
	void pathStartSoundChanged(QString newValue);
	void pathLookAwaySoundChanged(QString newValue);
	void pathReminderSoundChanged(QString newValue);
	
	void virtualBallCommandsChanged(bool newValue);

public slots:
    void setTimeWithoutHeadThreshold(double i) {QMutexLocker lock(m_propertyMutex); if (i>0) m_timeWithoutHeadThreshold = i; emit(timeWithoutHeadThresholdChanged(i));}
	// the negative value can be of use now:
	void setTimeIntervalToRemindAgain(double i) {QMutexLocker lock(m_propertyMutex); m_timeIntervalToRemindAgain = i; emit(timeIntervalToRemindAgainChanged(i));}
	void setSendMessageInterval(int i)  {QMutexLocker lock(m_propertyMutex); m_sendMessageInterval = i; emit(sendMessageIntervalChanged(i));}
	void setMessageDuration(int i);
	void setFactorMetersMovementToPixels(double i) {QMutexLocker lock(m_propertyMutex); if (i>0) { m_factorMetersMovementToPixels = i; emit(factorMetersMovementToPixelsChanged(i));}}
	void setFactorMaxball(double i) {if (i>0) { QMutexLocker lock(m_propertyMutex); m_factorMaxball  = i; emit(factorMaxballChanged(i));}}
	void setMinPosCorrection(double i) {QMutexLocker lock(m_propertyMutex); m_minPosCorrection = i; emit(minPosCorrectionChanged(i));}
	void setPixelsDiffPersonBall(int i) {QMutexLocker lock(m_propertyMutex); if (i>0) m_pixelsDiffPersonBall = i; emit(pixelsDiffPersonBallChanged(i));}
	
	void setFramesAveragingPosition(int i) {QMutexLocker lock(m_propertyMutex); if (i>0) m_framesAveragingPosition = i; emit(framesAveragingPositionChanged(i));}
    void setShowPopup(bool b);
	void setOverrideString(bool b);
	void setBallRollSpeed(int i);
	void setPopupSkipRate(int i);

    void setSomeString(QString s) {QMutexLocker lock(m_propertyMutex); m_someString = s; emit(someStringChanged(s));}
	void setPathStartSound(QString s) ;//{m_pathStartSound = s; emit(pathStartSoundChanged(s));}
	void setPathLookAwaySound(QString s);// {m_pathLookAwaySound = s; emit(pathLookAwaySoundChanged(s));}
	void setPathReminderSound(QString s);// {m_pathReminderSound = s; emit(pathReminderSoundChanged(s));}

	void setPlayTrigger(bool b);
	void setPersonOverruledByMouse(bool b);
	void setBallOverruledByMouse(bool b);
	//TODO the override is not needed to actually override it seems...
	//TODO TEMP bug fix set to false!
	void setOverrideFaceDetected(bool b) {QMutexLocker lock(m_propertyMutex); m_overrideFaceDetected = b; emit(overrideFaceDetectedChanged(b));}
	//void setFaceDetected(bool b) {m_faceDetected  = b; emit(faceDetectedChanged(b));}
	void setFaceDetected(bool b);
	void setVirtualBallCommands(bool b) {QMutexLocker lock(m_propertyMutex); m_virtualBallCommands = b; emit(virtualBallCommandsChanged(b));}
	
	//non gui variables
	void setFirstRun(bool b) {QMutexLocker lock(m_propertyMutex); m_firstrun =  b;}
	void setBeforeStart(bool b) {QMutexLocker lock(m_propertyMutex); m_beforestart = b;}
	void setInitialised(bool b) {QMutexLocker lock(m_propertyMutex); m_initialised = b;}
	void setHeadInCurrentFrame(bool b) {QMutexLocker lock(m_propertyMutex); m_headinframe = b;}
	void setBallPosition(double balpos);
	void setPersonPosition(double personpos);
	

private:
    plv::CvMatDataInputPin* m_inputPin;
    plv::InputPin<QString>* m_inputPinHP;
	plv::InputPin<QString>* m_inputPinHR;
	plv::InputPin<QString>* m_inputPinSB;
	//plv::InputPin< QList<plvblobtracker::BlobTrack> >* m_inputBlobs;

	plv::CvMatDataOutputPin* m_outputPin;
	plv::OutputPin<QString>* m_outputPin2;
	plv::OutputPin<QString>* m_outputPin3;

    //creates a boolean in the gui that will represent showing the widget.
	//if the widget is closed it should set the boolean to off and if the boolean is clicked is should be switched on
	bool m_showPopup;
	int m_skipcounter;
	
	//deal with multiple threads accesising the same list
	QMutex manuallistmutex;
	QMutex timersmutex;
	
	//be able to manualy type a command and send this command at the boolean click whatever input there is 
	bool m_overrideString;
    QString m_someString;
	
	//paths to the sounds
	QString m_pathStartSound;
	QString m_pathLookAwaySound;
	QString m_pathReminderSound;

	//play the attention grabbing sound
	bool m_playTrigger;

	//overruled by mouse will stop sending info from kinect and instead uses the last clicked postiion
	bool m_personOverruledByMouse;
	bool m_ballOverruledByMouse;

	//the virtual ball only needs commands of possition of the person
	bool m_virtualBallCommands;

	//create a popup
	plv::PlvMouseclick m_plvannotationwidget;
	//bool m_popUpExists;

	//check if a head is seen in the last  x frames
	bool m_faceDetected;
	//be able to override this manually
	bool m_overrideFaceDetected;
			
	//save the persumed position of the ball. It should be upgradeabl to another position
	cv::Point2d	m_ballPosition;
	//int m_ballPosition;
	cv::Point2d	m_personPosition;
	//int m_personPosition;
	BloxCommand m_bloxCommandos;

	QList<float> m_possitionsarray;
	QList<bool> m_rotationarray;
		
	//prevent confusing the ball and only send the messages with a certain duration and -100 interval 
	int m_sendMessageInterval;
	int m_messageDuration;

	//some paramaeters for checking the head position and rotation
	//the number of frames with a position to average over
	//int m_someInt;
	double m_timeWithoutHeadThreshold;
	double m_timeIntervalToRemindAgain;
	//now flexible in gui instead of one int m_averagesizelist;
	int m_framesAveragingPosition;
	
	//to set the speed the balls rolls with should be 20 or higher and ?100? or lower
	int m_ballRollSpeed;

	//to reduce processing time and prevent crashing
	int m_popupSkipRate;

	//the factor to multiply with to come from meters to pixels e.g. -0.5m to 0.5m to 640 pixels -> m_middlecorrection = 0.5, m_factor = 640
	int m_factor;
	float m_middlecorrection;	
	//the threshold in difference in pixels bvetween the assumed and the wanted position in order to roll the ball a certain direction.
	int m_threshold_posdiff;
		
	bool m_mouseclicked;
	
	QTime m_lastmovement;
	QTime m_lastheadfocus;
	QTime m_intervaltimer;

	//it is important to know if we are in a first run or have not yet started once, then we shoudl initialize stuff perhaps.
	bool m_firstrun;
	bool m_beforestart;
	bool m_initialised;
	bool m_woozfacefirstremindergiven;

	//void copyImgInto( const cv::Mat& in, cv::Mat& out , int posX, int posY );
	int m_pixelsDiffPersonBall;
	double m_factorMetersMovementToPixels;
	double m_factorMaxball;
	double m_minPosCorrection;

	//for drawing and tracking also kep into account if this current frame had a head in it;
	bool m_headinframe;
};

#endif // BLOXPROCESSOR_H
