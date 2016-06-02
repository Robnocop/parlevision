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
#include "../plvblobtracker/Blob.h"
#include "../plvblobtracker/BlobTrack.h"
using namespace plvblobtracker;

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
		//represent the direction the ball has been ordered to move in -1 == left, 0 == stop, 1 ==right
		int direction;
		//the duration this movement will be keeping on
		//for bodymovement a top of 2 Hz for rocking behavior has been mentioned so around 30fps --> <15 fps in order to get a proper reading, however the lag in actuation prevents such a speed
		int duration;
		//the speed the motors have to move with ranging from 0-100, minimum around 20 in order to actually move
		int speed;
		int durationWiggle;
		bool grabAttention;
		//returns the last string in a format that is recognisable for the ball.
		QString getString();
};

class BloxProcessor : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( BloxProcessor )

    Q_CLASSINFO("author", "Robby van Delden")
    Q_CLASSINFO("name", "BloxProcessor")
    //TODO make time and thus a float instead of frames
	Q_PROPERTY( double timeWithoutHeadThreshold READ getTimeWithoutHeadThreshold WRITE setTimeWithoutHeadThreshold NOTIFY timeWithoutHeadThresholdChanged  )
	Q_PROPERTY( double timeIntervalToRemindAgain READ getTimeIntervalToRemindAgain WRITE setTimeIntervalToRemindAgain NOTIFY timeIntervalToRemindAgainChanged  )
  
	Q_PROPERTY( double minPosCorrection READ getMinPosCorrection WRITE setMinPosCorrection NOTIFY minPosCorrectionChanged  )
	Q_PROPERTY( double factorMetersMovementToPixels READ getFactorMetersMovementToPixels WRITE setFactorMetersMovementToPixels NOTIFY factorMetersMovementToPixelsChanged  )
	Q_PROPERTY( int pixelsDiffPersonBall READ getPixelsDiffPersonBall WRITE setPixelsDiffPersonBall NOTIFY pixelsDiffPersonBallChanged  ) 
	
	Q_PROPERTY( int framesAveragingPosition READ getFramesAveragingPosition WRITE setFramesAveragingPosition NOTIFY framesAveragingPositionChanged  )
	Q_PROPERTY( bool showPopup READ getShowPopup WRITE setShowPopup NOTIFY showPopupChanged  )
   	Q_PROPERTY( bool playTrigger READ getPlayTrigger WRITE setPlayTrigger NOTIFY playTriggerChanged  )
	Q_PROPERTY( bool overrideString READ getOverrideString WRITE setOverrideString NOTIFY overrideStringChanged  )
	Q_PROPERTY( QString someString READ getSomeString WRITE setSomeString NOTIFY someStringChanged )
	Q_PROPERTY( bool overruledByMouse READ getOverruledByMouse WRITE setOverruledByMouse NOTIFY overruledByMouseChanged  )

	//create two popups for headorientation tracking, 
	//one showing the override 
	//and the other showing whether it currently is seeing a face of a person, perhaps this cna be combined with a regular viola jones tracker
	//will only change after it is bigger than the threshold time that is set, otherwise it will be flickering all the time
	Q_PROPERTY( bool faceDetected READ getFaceDetected WRITE setFaceDetected NOTIFY faceDetectedChanged  )
	Q_PROPERTY( bool overrideFaceDetected READ getOverrideFaceDetected WRITE setOverrideFaceDetected NOTIFY overrideFaceDetectedChanged  )

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
    double getTimeWithoutHeadThreshold() { return m_timeWithoutHeadThreshold; }
	double getTimeIntervalToRemindAgain() { return m_timeIntervalToRemindAgain; }
	double getFactorMetersMovementToPixels() { return m_factorMetersMovementToPixels; }
	double getMinPosCorrection() {return m_minPosCorrection;}
	int getPixelsDiffPersonBall() { return m_pixelsDiffPersonBall; }
		
	int getFramesAveragingPosition() { return m_framesAveragingPosition; }
    bool getShowPopup() { return m_showPopup; }
	
	bool getPlayTrigger() {return m_playTrigger;}
	bool getOverruledByMouse() {return m_overruledByMouse;}
	bool  getOverrideString() {return m_overrideString;}
	QString getSomeString() { return m_someString; }
	QString getPathStartSound() { return m_pathStartSound; }
	QString getPathLookAwaySound() { return m_pathLookAwaySound; }
	QString getPathReminderSound() { return m_pathReminderSound; }

	bool getOverrideFaceDetected() {return m_overrideFaceDetected;}
	bool getFaceDetected() {return m_faceDetected;}
	
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
	
	//other solution QList<QSound> m_sounds;

signals:
    void timeWithoutHeadThresholdChanged(double newValue);
	void timeIntervalToRemindAgainChanged(double newValue);
	
	void factorMetersMovementToPixelsChanged(double newValue);
	void minPosCorrectionChanged(double newValue);
	void pixelsDiffPersonBallChanged(int newValue);	
	
	void framesAveragingPositionChanged(int newValue);

	void showPopupChanged(bool newValue);
	void overrideStringChanged(bool newVAlue);
    void someStringChanged(QString newValue);
	void playTriggerChanged(bool newValue);
	void overruledByMouseChanged(bool newValue);
	void overrideFaceDetectedChanged(bool newValue);
	void faceDetectedChanged(bool newValue);
	void pathStartSoundChanged(QString newValue);
	void pathLookAwaySoundChanged(QString newValue);
	void pathReminderSoundChanged(QString newValue);
	

public slots:
    void setTimeWithoutHeadThreshold(double i) {if (i>0) m_timeWithoutHeadThreshold = i; emit(timeWithoutHeadThresholdChanged(i));}
	// the negative value can be of use now:
	void setTimeIntervalToRemindAgain(double i) {m_timeIntervalToRemindAgain = i; emit(timeIntervalToRemindAgainChanged(i));}
	void setFactorMetersMovementToPixels(double i) {if (i>0) { m_factorMetersMovementToPixels = i; emit(factorMetersMovementToPixelsChanged(i));}}
	void setMinPosCorrection(double i) {m_minPosCorrection = i; emit(minPosCorrectionChanged(i));}
	void setPixelsDiffPersonBall(int i) {if (i>0) m_pixelsDiffPersonBall = i; emit(pixelsDiffPersonBallChanged(i));}
	
	//pixelsDiffPersonBall factorMetersMovementToPixels minPosCorrection
	void setFramesAveragingPosition(int i) {if (i>0) m_framesAveragingPosition = i; emit(framesAveragingPositionChanged(i));}
    void setShowPopup(bool b);
	void setOverrideString(bool b);
    void setSomeString(QString s) {m_someString = s; emit(someStringChanged(s));}
	void setPathStartSound(QString s) ;//{m_pathStartSound = s; emit(pathStartSoundChanged(s));}
	void setPathLookAwaySound(QString s);// {m_pathLookAwaySound = s; emit(pathLookAwaySoundChanged(s));}
	void setPathReminderSound(QString s);// {m_pathReminderSound = s; emit(pathReminderSoundChanged(s));}

	void setPlayTrigger(bool b);
	void setOverruledByMouse(bool b);
	//TODO the override is not needed to actually override it seems...
	void setOverrideFaceDetected(bool b) {m_overrideFaceDetected = b; emit(overrideFaceDetectedChanged(b));}
	void setFaceDetected(bool b) {m_faceDetected  = b; emit(faceDetectedChanged(b));}

private:
    plv::CvMatDataInputPin* m_inputPin;
    plv::InputPin<QString>* m_inputPinS;
	plv::InputPin<QString>* m_inputPinS2;
	//plv::InputPin<QString>* m_inputPinSB;
	plv::InputPin< QList<plvblobtracker::BlobTrack> >* m_inputBlobs;

	plv::CvMatDataOutputPin* m_outputPin;
	plv::OutputPin<QString>* m_outputPin2;

    //creates a boolean in the gui that will represent showing the widget.
	//if the widget is closed it should set the boolean to off and if the boolean is clicked is should be switched on
	bool m_showPopup;

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
	bool m_overruledByMouse;

	//create a popup
	plv::PlvMouseclick m_plvannotationwidget;
	bool m_popUpExists;

	//check if a head is seen in the last  x frames
	bool m_faceDetected;
	//be able to override this manually
	bool m_overrideFaceDetected;
			
	//save the persumed position of the ball. It should be upgradeabl to another position
	cv::Point2d	m_balPosition;
	cv::Point2d	m_personPosition;
	BloxCommand m_bloxCommandos;

	QList<float> m_possitionsarray;
	QList<bool> m_rotationarray;
		
	//some paramaeters for checking the head position and rotation
	//the number of frames with a position to average over
	//int m_someInt;
	double m_timeWithoutHeadThreshold;
	double m_timeIntervalToRemindAgain;
	//now flexible in gui instead of one int m_averagesizelist;
	int m_framesAveragingPosition;
	//the factor to multiply with to come from meters to pixels e.g. -0.5m to 0.5m to 640 pixels -> m_middlecorrection = 0.5, m_factor = 640
	int m_factor;
	float m_middlecorrection;	
	//the threshold in difference in pixels bvetween the assumed and the wanted position in order to roll the ball a certain direction.
	int m_threshold_posdiff;
		
	bool m_mouseclicked;
	
	QTime m_lastmovement;
	QTime m_lastheadfocus;
	QTime m_intervaltimer;

	bool m_firstrun;
	//void copyImgInto( const cv::Mat& in, cv::Mat& out , int posX, int posY );
	int m_pixelsDiffPersonBall;
	double m_factorMetersMovementToPixels;
	double m_minPosCorrection;
};

#endif // BLOXPROCESSOR_H
