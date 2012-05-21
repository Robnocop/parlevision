#ifndef MSKINECTFAKECOLOR_H
#define MSKINECTFAKECOLOR_H

#include <QObject>
#include <plvcore/PipelineProcessor.h>
#include <plvcore/RefPtr.h>
#include <plvcore/Pin.h>
#include <plvcore/Enum.h>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

namespace plvmskinect
	{
	class MSKinectFakeColor : public plv::PipelineProcessor
	{
		Q_OBJECT
		Q_DISABLE_COPY( MSKinectFakeColor )

		Q_CLASSINFO("author", "Your Name")
		Q_CLASSINFO("name", "MSKinectFakeColor")
		Q_PROPERTY( int someInt READ getSomeInt WRITE setSomeInt NOTIFY someIntChanged  )
		Q_PROPERTY( int someInt2 READ getSomeInt2 WRITE setSomeInt2 NOTIFY someInt2Changed  )
		Q_PROPERTY( bool someBool READ getSomeBool WRITE setSomeBool NOTIFY someBoolChanged  )
		Q_PROPERTY( QString someString READ getSomeString WRITE setSomeString NOTIFY someStringChanged )

		/** required standard method declaration for plv::PipelineProcessor */
		PLV_PIPELINE_PROCESSOR

	public:
		MSKinectFakeColor();
		virtual ~MSKinectFakeColor();

		/** these methods can be overridden if they are necessary for
			your processor */
		virtual bool init();
		virtual bool deinit() throw();
		virtual bool start();
		virtual bool stop();

		/** propery methods */
		int getSomeInt() { return m_someInt; }
		int getSomeInt2() { return m_someInt2; }
		bool getSomeBool() { return m_someBool; }
		QString getSomeString() { return m_someString; }
	
	protected:
		 cv::Mat videoMatCache;


	signals:
		void someIntChanged(int newValue);
		void someInt2Changed(int newValue);
		void someBoolChanged(bool newValue);
		void someStringChanged(QString newValue);

	public slots:
		void setSomeInt(int i) {m_someInt = i; emit(someIntChanged(i));}
		void setSomeInt2(int d) {m_someInt2 = d; emit(someInt2Changed(d));}
		void setSomeBool(bool b) {m_someBool = b; emit(someBoolChanged(b));}
		void setSomeString(QString s) {m_someString = s; emit(someStringChanged(s));}

	private:
		plv::CvMatDataInputPin* m_inputPin;
		plv::CvMatDataOutputPin* m_outputPin;

		int m_someInt;
		int m_someInt2;
		bool m_someBool;
		QString m_someString;

	};
}

#endif // HELLOWORLDPROCESSOR_H
