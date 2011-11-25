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
 class CvMatDataOutputPin;
}

class PlvSynchronyProcessor : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( PlvSynchronyProcessor )

    Q_CLASSINFO("author", "Robby van Delden")
    Q_CLASSINFO("name", "PlvSynchrony Processor")
    Q_CLASSINFO("description", "Given two input functions (double values), calculate the time lagged crosscorrelation according to boker2003. As described in Boker2002 p9, uses vectors over time of the given winsize, skips WinInc frames after each computation (vertical), crosscorrelates vectors shifted -maxLag-0 frames backward (horizontal) e.g. -2 is Pearson(double(t-maxLag-winsize -- t-maxLag), double2(t-winsize -- t)), with shifting steps of lagShift so only three values if lagshift==maxlag this not implemented in current version set the width directly instead.")
    //these settings are all in frames
    //sets the size of correlation window,
    Q_PROPERTY( int winSize READ getWinSize WRITE setWinSize NOTIFY winSizeChanged  )
    //no clue yet
    Q_PROPERTY( int winInc READ getWinInc WRITE setWinInc NOTIFY winIncChanged  )
    ////no clue yet
    Q_PROPERTY( int maxLag READ getMaxLag WRITE setMaxLag NOTIFY maxLagChanged  )
    // no clue yet
    Q_PROPERTY( int lagShift READ getLagShift WRITE setLagShift NOTIFY lagShiftChanged  )
    Q_PROPERTY( bool corSquared READ getCorSquared WRITE setCorSquared NOTIFY corSquaredChanged  )

    //don't use notifty??
    // image width
    Q_PROPERTY( int imgWidth READ getImgWidth WRITE setImgWidth NOTIFY imgWidthChanged  )
    // image height
    Q_PROPERTY( int imgHeight READ getImgHeight WRITE setImgHeight NOTIFY imgHeightChanged  )

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
//        double corrwiki(long fIndex, long gIndex);
    /** propery methods */
	int getWinSize() { return m_winSize; }
	int getWinInc() { return m_winInc; }
	int getMaxLag() { return m_maxLag; }
	int getLagShift() { return m_lagShift; }
        int getImgHeight() { return m_imgHeight; }
        int getImgWidth() { return m_imgWidth; }
        bool getCorSquared() { return m_corSquared; }
        long getNrOfValuesStored() { return m_nrOfValuesStored; }
        int getCurrentDisplayLine() {return m_currentDisplayLine; }
       // CvMatData getOutputPreview() {return outputPreview; }
        IplImage* dst;

        //dont think this needs to be changed
//        double* m_inputFunctionF;
//        double* m_inputFunctionG;

        //storage of f and g history. When full, reset?
        const static long m_MAXVALS = 10000;
        double m_fHistory[10000]; //size??!!
        double m_gHistory[10000]; //size??!!
        //int m_currentDisplayline; //set to 0???


//	//the values that don't have to be change only need a get
//        double* getInputFunctionF() {return m_inputFunctionF; };
//        double* getInputFunctionG() {return m_inputFunctionG; };
	
//	//storage of f and g history. When full, reset?
//        long getMAXVALS() {return m_MAXVALS; };
//        double getFHistory() {return m_fHistory; }; //size??!!
//        double getGHistory() {return m_gHistory; }; //size??!!


signals:

	void winSizeChanged(int newValue);
	void winIncChanged(int newValue);
	void maxLagChanged(int newValue);
	void lagShiftChanged(int newValue);
        void imgHeightChanged(int newValue);
        void imgWidthChanged(int newValue);
        void corSquaredChanged(bool newValue);

public slots:
    //will need some extra attention to properly fix this
    //as it gets more complicated it should be included as seperate processes

    void setWinSize(int i) {m_winSize = i; emit(winSizeChanged(i));}
    void setWinInc(int i) {m_winInc = i; emit(winIncChanged(i));}
    void setMaxLag(int i) {m_maxLag = i; emit(maxLagChanged(i));}
    void setLagShift(int i) {m_lagShift = i; emit(lagShiftChanged(i));}
    void setImgHeight(int i) {m_imgHeight = i; emit (imgHeightChanged(i));}
    void setImgWidth(int i) {m_imgWidth = i; emit (imgWidthChanged(i));}
    void setCorSquared(bool i) {m_corSquared = i; emit (corSquaredChanged(i));}
        // set get in processor
//        void setWinInc(int i);
//        void setWinSize(int i);
//        void setMaxLag(int i);
//        void setLagShift(int i);
//        void setImgHeight(int i);
//        void setImgWidth(int i);

    //dont need a signal for these?
    void setNrOfValuesStored(long i) {m_nrOfValuesStored = i;}
    void setCurrentDisplayLine(int i) {m_currentDisplayLine = i;}


private:
    plv::CvMatDataOutputPin* m_outputPin;
    //an outputpitfor real time variables and debugging in string format


    plv::InputPin<double>* m_doubleInF;
    plv::InputPin<double>* m_doubleInG;

//TODO in test producer    plv::OutputPin<double>* m_doubleOut;

    //changeable during process
	int m_winSize;
	int m_winInc;
	int m_maxLag;
	int m_lagShift;
        int m_imgWidth;
        int m_imgHeight;
        bool m_corSquared;

        long m_nrOfValuesStored;
        int m_currentDisplayLine;

};

#endif // PLVSYNCHRONYPROCESSOR_H
