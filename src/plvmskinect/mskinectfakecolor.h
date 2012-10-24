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
		Q_CLASSINFO("description", "Shows the greyvalues in a spectrum more suitable for the huma n eye to make distinctions, e.g. to analyze something. Techinfo future pipleines: we bitshifted 3 times to get from kinectdeth to m, but allready shifted once to the other side to get to a larger spectrum of greyvalues, thus bitshift 4 times to get to m from the kinectdevice")
		Q_PROPERTY( int someInt READ getSomeInt WRITE setSomeInt NOTIFY someIntChanged  )
		Q_PROPERTY( int someInt2 READ getSomeInt2 WRITE setSomeInt2 NOTIFY someInt2Changed  )
		Q_PROPERTY( bool someBool READ getSomeBool WRITE setSomeBool NOTIFY someBoolChanged  )
		//Q_PROPERTY( QString someString READ getSomeString WRITE setSomeString NOTIFY someStringChanged )

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
	
	protected:
		 cv::Mat videoMatCache;


	signals:
		void someIntChanged(int newValue);
		void someInt2Changed(int newValue);
		void someBoolChanged(bool newValue);
		

	public slots:
		//
		void setSomeInt(int i); 
		void setSomeInt2(int d);
		void setSomeBool(bool b) {m_someBool = b; emit(someBoolChanged(b));}
		
	private:
		plv::CvMatDataInputPin* m_inputPin;
		plv::CvMatDataOutputPin* m_outputPin;
		plv::CvMatDataOutputPin* m_outputPin2;

		int m_someInt;
		int m_someInt2;
		bool m_someBool;

		//mutable QMutex m_propertyMutex;

	};
}

#endif // HELLOWORLDPROCESSOR_H
