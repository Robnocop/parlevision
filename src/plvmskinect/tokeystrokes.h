#ifndef TOKEYSTROKES_H
#define TOKEYSTROKES_H

#include <QObject>
#include <plvcore/PipelineProcessor.h>
#include <plvcore/RefPtr.h>
#include <plvcore/Pin.h>


#include <QTimer>

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

namespace plvmskinect
{
	class ToKeyStrokes : public plv::PipelineProcessor
	{
//class ToKeyStrokes : public plv::PipelineProcessor
//{
		Q_OBJECT
		Q_DISABLE_COPY( ToKeyStrokes )

		Q_CLASSINFO("author", "Robby van Delden")
		Q_CLASSINFO("name", "ToKeyStrokes")
					
		Q_PROPERTY( double bendForward READ getBendForward WRITE setBendForward NOTIFY bendForwardChanged  )
		Q_PROPERTY( double bendBackward READ getBendBackward WRITE setBendBackward NOTIFY bendBackwardChanged  )
		Q_PROPERTY( double leanLeft READ getLeanLeft WRITE setLeanLeft NOTIFY leanLeftChanged  )
		Q_PROPERTY( double leanRight READ getLeanRight WRITE setLeanRight NOTIFY leanRightChanged  )
		
		Q_PROPERTY( double minXRot READ getMinXRot WRITE setMinXRot NOTIFY minXRotChanged  )

		Q_PROPERTY( bool someBool READ getSomeBool WRITE setSomeBool NOTIFY someBoolChanged  )
		Q_PROPERTY( QString someString READ getSomeString WRITE setSomeString NOTIFY someStringChanged )
		Q_PROPERTY( int skipFrames READ getSkipFrames WRITE setSkipFrames NOTIFY skipFramesChanged )

		/** required standard method declaration for plv::PipelineProcessor */
		PLV_PIPELINE_PROCESSOR

	public:
		ToKeyStrokes();
		virtual ~ToKeyStrokes();

		/** these methods can be overridden if they are necessary for
			your processor */
		virtual bool init();
		virtual bool deinit() throw();
		virtual bool start();
		virtual bool stop();

		/** propery methods */
		bool getSomeBool() { return m_someBool; }
		QString getSomeString() { return m_someString; }
		double getMinXRot() { return m_minXRot; }

		double getBendForward() { return m_bendForward; }
		double getBendBackward() { return m_bendBackward; }
		double getLeanLeft() { return m_leanLeft; }
		double getLeanRight() { return m_leanRight; }
		int getSkipFrames() { return m_skipFrames; }

	signals:
		void someBoolChanged(bool newValue);
		void someStringChanged(QString newValue);
		void minXRotChanged(double newValue);

		void bendForwardChanged(double d);
		void bendBackwardChanged(double d);
		void leanLeftChanged(double d);
		void leanRightChanged(double d);

		void skipFramesChanged(int i);

	public slots:
		
		void setBendForward(double d);
		void setBendBackward(double d);
		void setLeanLeft(double d);
		void setLeanRight(double d);
		void keyRelease();

		void setSomeBool(bool b) {m_someBool = b; emit(someBoolChanged(b));}
		void setSomeString(QString s) {m_someString = s; emit(someStringChanged(s));}
		void setMinXRot(double d) {m_minXRot = d; emit(minXRotChanged(d));}
		void setSkipFrames(int i) {m_skipFrames = i; emit(skipFramesChanged(i));}

	private:
		plv::CvMatDataInputPin* m_inputPin;
		plv::InputPin<QVector4D>* m_inputPinQ4D;
		plv::CvMatDataOutputPin* m_outputPin;
		plv::OutputPin<QString>* m_outputPin2;

		double m_bendForward;
		double m_bendBackward;
		double m_leanLeft;
		double m_leanRight;
		bool stopBool;
		bool m_someBool;
		QString m_someString;
		double m_minXRot;
		int m_skipFrames;
		int m_skipFrameCounter;

		int keyboardStuff(QVector4D inputBone);
		QTimer *m_keyDownTimer;
		int returnvalueKeyboardStuff;
		int lastReturnvalueKeyboardStuff;
		int repeatedPresses;
		bool pressBool;
		

	};
}

#endif // ToKeyStrokes_H
