#ifndef SKIPFRAMESPROCESSOR_H
#define SKIPFRAMESPROCESSOR_H

#include <QObject>
#include <plvcore/PipelineProcessor.h>
#include <plvcore/RefPtr.h>
#include <plvcore/Pin.h>

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}

class SkipFramesProcessor : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( SkipFramesProcessor )

    Q_CLASSINFO("author", "Robby van Delden")
    Q_CLASSINFO("name", "SkipFramesProcessor")
    Q_PROPERTY( int framesToSkip READ getFramesToSkip WRITE setFramesToSkip NOTIFY framesToSkipChanged  )
    Q_PROPERTY( bool skipOn READ getSkipOn WRITE setSkipOn NOTIFY skipOnChanged  )
    
    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR

public:
    SkipFramesProcessor();
    virtual ~SkipFramesProcessor();

    /** these methods can be overridden if they are necessary for
        your processor */
    virtual bool init();
    virtual bool deinit() throw();
    virtual bool start();
    virtual bool stop();

    /** propery methods */
    int getFramesToSkip() {QMutexLocker lock(m_propertyMutex); return m_framesToSkip; }
    bool getSkipOn() { QMutexLocker lock(m_propertyMutex); return m_skipOn; }
    
signals:
    void framesToSkipChanged(int newValue);
    void skipOnChanged(bool newValue);
    
public slots:
    void setFramesToSkip(int i) {QMutexLocker lock(m_propertyMutex); m_framesToSkip = i; emit(framesToSkipChanged(i));}
    void setSkipOn(bool b) {QMutexLocker lock(m_propertyMutex); m_skipOn = b; emit(skipOnChanged(b));}
    
private:
    plv::CvMatDataInputPin* m_inputPin;
    plv::CvMatDataOutputPin* m_outputPin;
	
    int m_framesToSkip;
    bool m_skipOn;
	bool m_firstrun;
	unsigned int m_lastserial;
};

#endif // SkipFramesProcessor_H
