#ifndef HELLOWORLDPROCESSOR_H
#define HELLOWORLDPROCESSOR_H

#include <QObject>
#include <plvcore/PipelineProcessor.h>
#include <plvcore/RefPtr.h>
#include <plvcore/Pin.h>

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

class HelloWorldProcessor : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( HelloWorldProcessor )

    Q_CLASSINFO("author", "Your Name")
    Q_CLASSINFO("name", "HelloWorldProcessor")
    Q_PROPERTY( int someInt READ getSomeInt WRITE setSomeInt NOTIFY someIntChanged  )
    Q_PROPERTY( double someDouble READ getSomeDouble WRITE setSomeDouble NOTIFY someDoubleChanged  )
    Q_PROPERTY( bool someBool READ getSomeBool WRITE setSomeBool NOTIFY someBoolChanged  )
    Q_PROPERTY( QString someString READ getSomeString WRITE setSomeString NOTIFY someStringChanged )

    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR

public:
    HelloWorldProcessor();
    virtual ~HelloWorldProcessor();

    /** these methods can be overridden if they are necessary for
        your processor */
    virtual bool init();
    virtual bool deinit() throw();
    virtual bool start();
    virtual bool stop();

    /** propery methods */
    int getSomeInt() { return m_someInt; }
    double getSomeDouble() { return m_someDouble; }
    bool getSomeBool() { return m_someBool; }
    QString getSomeString() { return m_someString; }

signals:
    void someIntChanged(int newValue);
    void someDoubleChanged(double newValue);
    void someBoolChanged(bool newValue);
    void someStringChanged(QString newValue);

public slots:
    void setSomeInt(int i) {m_someInt = i; emit(someIntChanged(i));}
    void setSomeDouble(double d) {m_someDouble = d; emit(someDoubleChanged(d));}
    void setSomeBool(bool b) {m_someBool = b; emit(someBoolChanged(b));}
    void setSomeString(QString s) {m_someString = s; emit(someStringChanged(s));}

private:
    plv::CvMatDataInputPin* m_inputPin;
    plv::CvMatDataOutputPin* m_outputPin;
	plv::OutputPin<QString>* m_outputPin2;

    int m_someInt;
    double m_someDouble;
    bool m_someBool;
    QString m_someString;
};

#endif // HelloWorldProcessor_H
