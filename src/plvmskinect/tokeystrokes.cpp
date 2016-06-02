//temp for fake key
//#define WINVER 0x0500
#define WIN32_WINNT 0x0501
#include <windows.h>
#include <Winuser.h>
using namespace std;

#include <QDebug>

#include "ToKeyStrokes.h"
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>


using namespace plv;
using namespace plvmskinect;



ToKeyStrokes::ToKeyStrokes() :
        m_someBool(true),
		m_bendForward(-0.10),
		m_bendBackward(0.12),
		m_leanLeft(-0.1),
		m_leanRight(0.1),
        m_someString("hello"),
		m_skipFrames(0)
{
	returnvalueKeyboardStuff = 0;
	lastReturnvalueKeyboardStuff = 0;
    m_inputPin = createCvMatDataInputPin("input", this);
	m_inputPinQ4D = plv::createInputPin<QVector4D>("input 4D eg bone", this);

    m_outputPin = createCvMatDataOutputPin("output", this);
	m_outputPin2 = createOutputPin<QString>( "text", this);

	m_inputPin->addSupportedChannels(1);
	m_inputPin->addSupportedChannels(3);

    m_inputPin->addSupportedDepth(CV_8U);
    m_inputPin->addSupportedDepth(CV_16U);

	
	repeatedPresses = 0;
	pressBool = false;
	m_keyDownTimer = new QTimer(this);
	
	

	//m_repeatSoundCatNr = -1;
	connect(m_keyDownTimer, SIGNAL(timeout()), this, SLOT(keyRelease()));

}

ToKeyStrokes::~ToKeyStrokes()
{
}

bool ToKeyStrokes::init()
{
	return true;
}

bool ToKeyStrokes::deinit() throw ()
{
    return true;
}

bool ToKeyStrokes::start()
{
	pressBool = false;
	stopBool=false;
	repeatedPresses = 0;
	//try pressing real quick
	m_keyDownTimer ->start(2);
	m_keyDownTimer ->setSingleShot(false);
	//CHANGED
	//connect(m_keyDownTimer, SIGNAL(timeout()), this, SLOT(keyRelease()));

	m_skipFrameCounter = 0;

    return true;
}

bool ToKeyStrokes::stop()
{
	stopBool = true;
    return true;
}

//apperantly a process only is called when all pins receive a value
//which complicates things if one pin receives more values than the other :S
bool ToKeyStrokes::process()
{
	assert(m_inputPin != 0);
    assert(m_outputPin != 0);

	m_skipFrameCounter++;

	CvMatData src = m_inputPin->get();
    // allocate a target buffer
    CvMatData target;
    target.create( src.width(), src.height(), src.type() );

	//get the current text
	QString texttoshow = "";//getSomeString();

	QVector4D inputBone = m_inputPinQ4D->get();
	//!works qDebug() << inputBone.x() ;
	
	//QString bonevalues = "0";
	//if (!inputBone.isNull())
	//	bonevalues = QString("%1 \t %2 \t %3").arg(inputBone.x()).arg(inputBone.y()).arg(inputBone.z());

	//TODO face needs to be used once if only skeleton is wanted

	//get the current processingserial();
	unsigned int serialInInt = this->getProcessingSerial();
	QString serialIntInString = QString("%1").arg(serialInInt);

	QString boneString = "";

	if (m_skipFrameCounter>getSkipFrames())
	{
		lastReturnvalueKeyboardStuff = returnvalueKeyboardStuff;
		returnvalueKeyboardStuff = 0;
		
		if (!inputBone.isNull())
		{
		
			boneString = QString("%1 \t %2 \t %3 \t %4").arg(inputBone.x()).arg(inputBone.y()).arg(inputBone.z()).arg(inputBone.w());
			//TODO call the keystroke generator method
			returnvalueKeyboardStuff = keyboardStuff(inputBone);
			m_outputPin2->put(texttoshow.append(boneString));
		}
	
	
	    
		// do a flip of the image
		const cv::Mat in = src;
		cv::Mat out = target;
		//inserts picture
		cv::flip( in, out, (int)m_someBool);
		cv::Point right;
		right.x = (int) out.cols/4;
		right.y = (int) out.rows/2;
		cv::Point left;
		left.x = (int) (out.cols/4) *3;
		left.y = (int) out.rows/2;
		cv::Point bottom;
		bottom.x = (int) (out.cols/2);
		bottom.y = (int) (out.rows/4) *3;
		cv::Point top;
		top.x = (int) (out.cols/2);
		top.y = (int) (out.rows/4);

		if (returnvalueKeyboardStuff>0)
		{
			if (!pressBool)
			{
				//m_keyDownTimer ->start(1);
				//m_keyDownTimer ->setSingleShot(false);
				pressBool = true;
			}	

			//draw circles
			//qDebug() << ">0 " <<   returnvalueKeyboardStuff;
			if (returnvalueKeyboardStuff==10 || returnvalueKeyboardStuff==11 || returnvalueKeyboardStuff==12)
			{
				//draw circle if lean left value returned is 10 or 10+1 or 10+2
				circle( out, left, 20, cv::Scalar(0,0,255), -1, 8);
			}
			else if (returnvalueKeyboardStuff==20 || returnvalueKeyboardStuff==21 || returnvalueKeyboardStuff==22)
			{
				//draw circle if lean right value returned is 10 or 10+1 or 10+2
				circle( out, right, 20, cv::Scalar(0,0,255), -1, 8);
			}

			if (returnvalueKeyboardStuff==1 || returnvalueKeyboardStuff==11 || returnvalueKeyboardStuff==21)
			{
				//draw circle if lean left value returned is 10 or 10+1 or 10+2
				circle( out, top, 20, cv::Scalar(0,255,0), -1, 8);
			}
			else if (returnvalueKeyboardStuff==2 || returnvalueKeyboardStuff==21 || returnvalueKeyboardStuff==22)
			{
				//draw circle if lean right value returned is 10 or 10+1 or 10+2
				circle( out, bottom, 20, cv::Scalar(0,255,0), -1, 8);
			}
		}
		else
		{
			pressBool = false;
			//m_keyDownTimer ->setSingleShot(true);
			//m_keyDownTimer ->stop();
		}	
	

		// publish the new image
		m_outputPin->put( out );
		
		//reset the counter
		m_skipFrameCounter = 0;
	}
	else
	{
		// do a flip of the image
		const cv::Mat in = src;
		cv::Mat out = target;
		//inserts picture
		cv::flip( in, out, (int)m_someBool);

		if (!inputBone.isNull())
		{
		
			boneString = QString("%1 \t %2 \t %3 \t %4").arg(inputBone.x()).arg(inputBone.y()).arg(inputBone.z()).arg(inputBone.w());
			//TODO call the keystroke generator method
			returnvalueKeyboardStuff = keyboardStuff(inputBone);
			m_outputPin2->put(texttoshow.append(boneString));
		}
		m_outputPin2->put(texttoshow.append(boneString));
		// publish the new image
		m_outputPin->put( out );
	}
	

    return true;
}

//??
////if x y z
//--> z is voorover/achterover buigen +- 0.13 tot -0.13
//--> y is schouder roteren 
//--> x is links/rechts leunen -0.15 tot 0.15

int ToKeyStrokes::keyboardStuff(QVector4D bone)
{
	//Structure for the keyboard event
    //Set up the general INPUT structure
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.wScan = 0;
	//e.g. ip.ki.wVk = 0x41; //A key https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
//{ "VK_LEFT", 0x25, "LEFT ARROW key" },
//{ "VK_UP", 0x26, "UP ARROW key" },
//{ "VK_RIGHT", 0x27, "RIGHT ARROW key" },
//{ "VK_DOWN", 0x28, "DOWN ARROW key" },

//{ "A key", 0x41, "A key" },

	//if (bone.y()<0.89)
	bool release_a = false;
	bool release_d = false;

	int toreturn = 0;

	//for test use leaning
	if (bone.x()< getLeanLeft())
	{
		//turn left : press left
		//ip.ki.wVk = 0x25;

		//lean left press a
		ip.ki.wVk = 0x41;
		SendInput(1, &ip, sizeof(INPUT));
		release_a = true;
		ip.ki.dwFlags =  KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
		toreturn = 10;
		
	}
	else if (bone.x()>getLeanRight())
	{
		//turn right : press right
		//ip.ki.wVk = 0x27;

		//lean right press d
		ip.ki.wVk = 0x44;
		SendInput(1, &ip, sizeof(INPUT));
		ip.ki.dwFlags =  KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
		toreturn = 20;
	}
	
	//bendforward
	if (bone.z()<getBendForward())
	{
		//lean forward: press up
		//ip.ki.wVk = 0x25;
		
		ip.ki.dwFlags = 0;
		//lean forwad press w
		ip.ki.wVk = 0x57;
		SendInput(1, &ip, sizeof(INPUT));
		ip.ki.dwFlags =  KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
		toreturn = toreturn+1;
	}
	//slighty too
	else if (bone.z()>getBendBackward())
	{
		//lean back : press down
		//ip.ki.wVk = 0x27;
		
		ip.ki.dwFlags = 0;
		//lean back press s
		ip.ki.wVk = 0x53;
		SendInput(1, &ip, sizeof(INPUT));
		ip.ki.dwFlags =  KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
		toreturn = toreturn+2;
	}
	
    return toreturn;
}

void ToKeyStrokes::setBendForward(double d)
{
	QMutexLocker lock(m_propertyMutex);
	m_bendForward = d; 
	emit(bendForwardChanged(d));
}

void ToKeyStrokes::setBendBackward(double d)
{
	QMutexLocker lock(m_propertyMutex);
	m_bendBackward = d; 
	emit(bendBackwardChanged(d));
}

void ToKeyStrokes::setLeanLeft(double d)
{
	QMutexLocker lock(m_propertyMutex);
	m_leanLeft = d; 
	qDebug() << "value bendbackward" << getBendBackward();
	emit(leanLeftChanged(d));
}

void ToKeyStrokes::setLeanRight(double d)
{
	QMutexLocker lock(m_propertyMutex);
	m_leanRight = d; 
	emit(leanRightChanged(d));
}

//a little overkill to press every ms...
void ToKeyStrokes::keyRelease() 
{
	if (stopBool)
	{
		m_keyDownTimer ->stop();
		m_keyDownTimer ->setSingleShot(true);
	//CHANGED

	}
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.wScan = 0;

	//overwrite to a keyup every 200 ms
	if(repeatedPresses>2000)
	{
		pressBool = false;
		repeatedPresses = 0;
	}

	//press key everey ms and release when called
	if (lastReturnvalueKeyboardStuff==10 || lastReturnvalueKeyboardStuff==11 || lastReturnvalueKeyboardStuff==12)
	{
		//turn left : press left
		//ip.ki.wVk = 0x25;
		//lean left press a
		ip.ki.wVk = 0x41;
		if (pressBool)
		{
			SendInput(1, &ip, sizeof(INPUT));
			repeatedPresses++;
		}
		else
		{
			ip.ki.dwFlags =  KEYEVENTF_KEYUP;
			SendInput(1, &ip, sizeof(INPUT));
			repeatedPresses =0;
		}
		
	}
	else if (lastReturnvalueKeyboardStuff==20 || lastReturnvalueKeyboardStuff==21 || lastReturnvalueKeyboardStuff==22)
	{
		//turn right : press right
		//ip.ki.wVk = 0x27;

		//lean right press d
		ip.ki.wVk = 0x44;
		if (pressBool)
		{
			SendInput(1, &ip, sizeof(INPUT));
			repeatedPresses++;
		}
		else
		{
			ip.ki.dwFlags =  KEYEVENTF_KEYUP;
			SendInput(1, &ip, sizeof(INPUT));
			repeatedPresses =0;
		}
	}
	
	//bendforward
	if (lastReturnvalueKeyboardStuff==1 || lastReturnvalueKeyboardStuff==11 || lastReturnvalueKeyboardStuff==21)
	{
		//lean forward: press up
		//ip.ki.wVk = 0x25;
		
		ip.ki.dwFlags = 0;
		//lean forwad press w
		ip.ki.wVk = 0x57;
		if (pressBool)
		{
			SendInput(1, &ip, sizeof(INPUT));
			repeatedPresses++;
		}
		else
		{
			ip.ki.dwFlags =  KEYEVENTF_KEYUP;
			SendInput(1, &ip, sizeof(INPUT));
			repeatedPresses =0;
		}
	}
	else if (lastReturnvalueKeyboardStuff==2 || lastReturnvalueKeyboardStuff==21 || lastReturnvalueKeyboardStuff==22)
	{
		//lean back : press down
		//ip.ki.wVk = 0x27;
		
		ip.ki.dwFlags = 0;
		//lean back press s
		ip.ki.wVk = 0x53;
		if (pressBool)
		{
			SendInput(1, &ip, sizeof(INPUT));	
			repeatedPresses++;
		}
		else
		{
			ip.ki.dwFlags =  KEYEVENTF_KEYUP;
			SendInput(1, &ip, sizeof(INPUT));
			repeatedPresses =0;
		}
	}
}

//ALTERNATIVE CODE THINGS:
// overwrite generate down based on http://stackoverflow.com/questions/3644881/simulating-keyboard-with-sendinput-api-in-directinput-applications
//if ( getSomeBool() )
//{ 
//	ip.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
//	 
//	ip.ki.wVk  = 0x41; //now we do need VK, which might influence it's scope
//	//ip.ki.wScan = 0x1E;  //Set a unicode character to use (A)
//}

// overwrite generate up based on http://stackoverflow.com/questions/3644881/simulating-keyboard-with-sendinput-api-in-directinput-applications
//      if ( getSomeBool() )
//{ 
//	ip.ki.dwFlags  =  KEYEVENTF_KEYUP;
//	ip.ki.dwFlags  |= KEYEVENTF_EXTENDEDKEY;
//	 
//	ip.ki.wVk  = 0x41; //now we do need VK (btw this is a), which might influence it's scope
//	//ip.ki.wScan = 0x1E;  //Set a unicode character to use (A)
//}

///////HARDSCAN CODE:
//TODO PROVIDE FEEDBACK

//This let's you do a hardware scan instead of a virtual keypress
//ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY;
	
//ip.ki.dwFlags ;
//Unfortunatly I cant find the right scancodes
//pressBool = true;
	
//WORD AButton = 0x1E;
////WORD AButton = 0x; 
////WORD leftArrow = 0x1D;
////WORD rightArrow = 0x4B;
////wiki: IBM 1 standard http://en.wikipedia.org/wiki/Scancode
//WORD rightArrow = 0x5B;
//WORD leftArrow = 0x5C;
//WORD upArrow = 0x48;
//WORD downArrow = 0x4D;
//WORD prefix = 0xE0;

//ip.ki.wScan = prefix;  //Set a unicode character to use (A)

//http://www.win.tue.nl/~aeb/linux/kbd/scancodes-14.html
//http://stackoverflow.com/questions/23203944/automating-key-presses-with-sendinput
//2) if you intend to send keystrokes to a game or another application that takes input from Direct 3D (or another DirectInput environment), 
// you need to send the key strokes as scancodes, and therefor you specify the wScan field, and leave wVk blank. See info here: KEYBDINPUT
//3) Remember to switch the window in focus to the application you want to send keystrokes to, 
//before sending the keystroke. Example: MemoryHandler.SwitchWindow(Process.GetProcessesByName("notepad").FirstOrDefault().MainWindowHandle);

//TODO TRY WITH DIRECT UP FOR HARDWARE SCAN AS WELL
//i did LETS TRY WITH PRESSEES ONLY, no key up
//TODO if it works compare bone to press which button
//TODO use qt timed event:
//if (!pressBool)
//{
//	//IS THIS A SINGLE PRESS???
//	//Send the press
//	
//	//SendInput(1, &ip, sizeof(INPUT));

//	//let's try virtual instead of this hardscan
//	//ip.ki.wScan = rightArrow;

//	//lets try virtual again
//	ip.ki.wScan = 0;
//	SendInput(1, &ip, sizeof(INPUT));
//	pressBool = true;
//	//qDebug() << "push in ";

//	//directly release?
//	ip.ki.dwFlags =  KEYEVENTF_KEYUP;
//	SendInput(1, &ip, sizeof(INPUT));
//}

	
//else
//{
//	//?TODO skip for now only uses presses
//	//Prepare a keyup event
//	//ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
//	ip.ki.dwFlags =  KEYEVENTF_KEYUP;
//	//Prepare a keyup event

//	//lets try virtual again
//	ip.ki.wScan = 0;
//	SendInput(1, &ip, sizeof(INPUT));
//	pressBool = false;

//	//qDebug() << "push out ";
//}