#ifndef PLVSYNCHRONYPROCESSOR_H
#define PLVSYNCHRONYPROCESSOR_H

#include <QObject>
#include <plvcore/PipelineProcessor.h>
#include <plvcore/RefPtr.h>
#include <plvcore/Pin.h>
#include <opencv/cv.h>
//needed for iplimage format
#include <QDebug>


namespace plv
{
    class CvMatDataInputPin; //might be no longer needed
    class CvMatDataOutputPin;
}

class PlvSynchronyProcessor : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( PlvSynchronyProcessor )

    Q_CLASSINFO("author", "Robby van Delden")
    Q_CLASSINFO("name", "PlvSynchrony Processor")
	Q_CLASSINFO("description", "Given two input functions (double values), calculate the time lagged crosscorrelation according to boker2003." )
    Q_PROPERTY( int someInt READ getSomeInt WRITE setSomeInt NOTIFY someIntChanged  )
	
	//these settings are all in frames
	//sets the size of correlation window, 
	Q_PROPERTY( int winSize READ getWinSize WRITE setWinSize NOTIFY winSizeChanged  )
	//no clue yet
	Q_PROPERTY( int winInc READ getWinInc WRITE setWinInc NOTIFY winIncChanged  )
	////no clue yet
	Q_PROPERTY( int maxLag READ getMaxLag WRITE setMaxLag NOTIFY maxLagChanged  )
	// no clue yet
	Q_PROPERTY( int lagShift READ getLagShift WRITE setLagShift NOTIFY lagShiftChanged  )

    Q_PROPERTY( double someDouble READ getSomeDouble WRITE setSomeDouble NOTIFY someDoubleChanged  )
    Q_PROPERTY( bool someBool READ getSomeBool WRITE setSomeBool NOTIFY someBoolChanged  )
    Q_PROPERTY( QString someString2 READ getSomeString2 WRITE setSomeString2 NOTIFY someStringChanged2 )

    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR

public:
    PlvSynchronyProcessor();
    virtual ~PlvSynchronyProcessor();

    /** these methods can be overridden if they are necessary for
        your processor */
    virtual bool init();
    virtual bool deinit() throw();
    virtual bool start();
    virtual bool stop();

	double corr(long fIndex, long gIndex);

    /** propery methods */
    //remainders from hellowworld plugin
	int getSomeInt() { return m_someInt; }
	double getSomeDouble() { return m_someDouble; }
        bool getSomeBool() { return m_someBool; }
        QString getSomeString2() { return m_someString2; }
	int getWinSize() { return m_winSize; }
	int getWinInc() { return m_winInc; }
	int getMaxLag() { return m_maxLag; }
	int getLagShift() { return m_lagShift; }
        long getNrOfValuesStored() { return m_nrOfValuesStored; }
        int getCurrentDisplayLine() {return m_currentDisplayline; }
       // CvMatData getOutputPreview() {return outputPreview; }
        IplImage* dst;

        //dont think this needs to be changed
        double* m_inputFunctionF;
        double* m_inputFunctionG;

        //storage of f and g history. When full, reset?
        const static long m_MAXVALS = 10000;
        double m_fHistory[10000]; //size??!!
        double m_gHistory[10000]; //size??!!
        int m_currentDisplayline;


//	//the values that don't have to be change only need a get
//        double* getInputFunctionF() {return m_inputFunctionF; };
//        double* getInputFunctionG() {return m_inputFunctionG; };
	
//	//storage of f and g history. When full, reset?
//        long getMAXVALS() {return m_MAXVALS; };
//        double getFHistory() {return m_fHistory; }; //size??!!
//        double getGHistory() {return m_gHistory; }; //size??!!


signals:
    void someIntChanged(int newValue);

	void winSizeChanged(int newValue);
	void winIncChanged(int newValue);
	void maxLagChanged(int newValue);
	void lagShiftChanged(int newValue);
	
    void someDoubleChanged(double newValue);
    void someBoolChanged(bool newValue);
    void someStringChanged2(QString newValue);

public slots:
    void setSomeInt(int i) {m_someInt = i; emit(someIntChanged(i));}

    void setWinSize(int i) {m_winSize = i; emit(winSizeChanged(i));}
    void setWinInc(int i) {m_winInc = i; emit(winIncChanged(i));}
    void setMaxLag(int i) {m_maxLag = i; emit(maxLagChanged(i));}
    void setLagShift(int i) {m_lagShift = i; emit(lagShiftChanged(i));}
    //dont need a signal for nrofvalues I guess
    void setNrOfValuesStored(long i) {m_nrOfValuesStored = i;}
    void setCurrentDisplayLine(int i) {m_currentDisplayLine = i;}

    void setSomeDouble(double d) {m_someDouble = d; emit(someDoubleChanged(d));}
    void setSomeBool(bool b) {m_someBool = b; emit(someBoolChanged(b));}
    void setSomeString2(QString s) {m_someString2 = s; emit(someStringChanged2(s));}

    //don't know if this is really a smart move??
//    void setOutputPreview() {return outputPreview; }

private:
   // plv::CvMatDataInputPin* m_inputPin;
  //  plv::CvMatDataInputPin* m_inputPin2;
    plv::CvMatDataOutputPin* m_outputPin;
    plv::InputPin<double>* m_doubleInF;
    plv::InputPin<double>* m_doubleInG;

//TODO in test producer    plv::OutputPin<double>* m_doubleOut;

    //changeable during process
	int m_winSize;
	int m_winInc;
	int m_maxLag;
	int m_lagShift;

	int m_someInt;

	double m_someDouble;
        bool m_someBool;
        QString m_someString2;

        long m_nrOfValuesStored;
        int m_currentDisplayLine;

};

#endif // PLVSYNCHRONYPROCESSOR_H
