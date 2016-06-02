//?? why is it case depended?
#include "plvsynchronyprocessor.h"

#include <QDebug>

#include "PlvSynchronyProcessor.h"
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <opencv/cv.h>
#include <plvcore/util.h>

using namespace plv;

//constructor
PlvSynchronyProcessor::PlvSynchronyProcessor() :
        //problem with these vaues is that they should be interval dependent but the fps is not set to a certain value instead it actually seems to depend on the calculation
        //on my pc the processing of a webcam runs on 25fps
        m_winSize(40), //standard 40 --> 40 frames at 10fps 4s in org application and in Reidsma paper, also seen 1,2,4 and 5
        m_winInc(4),
        m_maxLag(20),
        m_lagShift(4),
        m_imgWidth(640),
        m_imgHeight(480),
        m_corSquared(true)

{
    //TODO inspect
    //is this right or should it have been double* here as well
        m_doubleInF = createInputPin<double>("double", this);
        m_doubleInG = createInputPin<double>("double2", this);
        dst = NULL;
        m_outputPin = createCvMatDataOutputPin("output", this);

        //NOOBY C++ remark,
        //* can be read as value pointed by
        //and & adress of
        //but datatype* used when declaring a pointer
        double* fHistory[10000];
        double* gHistory[10000];

        //reset values at beginning
        m_currentDisplayLine = 0;
        //to be reset
        m_nrOfValuesStored = 0;


    for (int i = 0; i<m_MAXVALS; i++)
	{
                fHistory[i] = 0;
                gHistory[i] = 0;
	}
}

//parlevision nooby remark: WTF is this destructor's purpose??
PlvSynchronyProcessor::~PlvSynchronyProcessor()
{
    // originally in parlevision 4.0 the publishing of new image was placed here

    // publish the new image
    //        if (dst != NULL)
    //        {
    //                cvReleaseImage(&dst);
    //        }
}

bool PlvSynchronyProcessor::init()
{
    return true;
}

bool PlvSynchronyProcessor::deinit() throw ()
{
    return true;
}

bool PlvSynchronyProcessor::start()
{
    return true;
}

bool PlvSynchronyProcessor::stop()
{
    return true;
}

bool PlvSynchronyProcessor::process()
{
    assert(m_outputPin != 0);
    assert(m_doubleInF != 0);
    assert(m_doubleInG != 0);

    //minimum mathematical requirements for settings

    if (getImgHeight() < 1)
    {
        setImgHeight(2);
    }

    if (getImgWidth() < 4)
    {
        setImgWidth(3);
    }

    if (getWinInc() < 1 )
    {
        setWinInc(1);
    }

    if (getLagShift() < 1)
    {
        setLagShift(1);
    }

    // for each person the movements are supposed to be given as a pixel difference sum in the incoming pipeline

    //TODO inspect:
    //the double* is changed to double, which might be wrong as double* would be a pointer to a value but this differnece seems to be due to parlevision 4 to 5 handling of input
    //the m_double_InF is declared as an <double>* in the header file
//orignal parlevision v4 bokerview
//    const double* fInput = (double*)getInputPinDataPtr(BOKERVIEW_PIN_IN_F);
//    const double* gInput = (double*)getInputPinDataPtr(BOKERVIEW_PIN_IN_G);
//handling of inputpin in parlevision v5
//     CvMatData in = m_inputImage->get();
	
    const double fInput = m_doubleInF->get();
    const double gInput = m_doubleInG->get();

    int winSize = getWinSize();
    int maxLag = getMaxLag();
    int MAXVALS = m_MAXVALS;

    //TODO inspect
    //is it neater to change this buffer array handling to a set get structure????

    //adds values to buffer
    //as fInput has been changed from double* it now suffices to use fInput
    //original parlevision v4 bokerview
    //  m_fHistory[m_nrOfValuesStored]=*fInput;
    m_fHistory[getNrOfValuesStored()]=fInput;
    m_gHistory[getNrOfValuesStored()]=gInput;

    //DID inspect supposed to be an odd number for lag variable according to Boker[2002,p8]
    //thus should be an odd number of grayscale values for width
    //so changed code added cvcircle lag =0 with corr
    //int imgwidth = getImgWidth(); //src.width();
    //int imgheight = getImgHeight(); //src.height();
    //check on odd number imgwidth
    //might need an odd number for symmetric representation as defined by boker
    //but might need an even number to create a proper processable picture

	//change to iseven return (n&1)==0;
    if ( plv::Util::isEven(getImgWidth()) )
    {
        setImgWidth(getImgWidth()-1);
    }

    if (dst == NULL)
    {
        //depth options depth: pixel depth in bits:
        //8u seems to be a reasonable choice if other depth is chosen don't forget to change val calculation line
        //IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16U,IPL_DEPTH_16S, IPL_DEPTH_32S, IPL_DEPTH_32F, IPL_DEPTH_64F
        dst = cvCreateImage(cvSize(getImgWidth(), getImgHeight()), IPL_DEPTH_8U, 3);

    }
    //TODO improve
    //use the variable changed signal for this instead of evaluating this every frame
    else if (( (dst->width) != getImgWidth()) || ((dst->height) != getImgHeight()))
    {
        dst = cvCreateImage(cvSize(getImgWidth(), getImgHeight()), IPL_DEPTH_8U, 3);
    }

    //for updating frame and storing the number, depends on the incoming doubles
    this->setNrOfValuesStored(this->getNrOfValuesStored()+1);

    //if the buffer is full reset the values
    if (getNrOfValuesStored()>=MAXVALS)
    {
            //copy exactly enough values for a full lagged analysis from end of histories to start;
            //needed: winsize+maxlag frames from both functions
            for (int i = 0; i < winSize+maxLag; i++) {
                    m_fHistory[i] = m_fHistory[MAXVALS-winSize-maxLag+i];
                    m_gHistory[i] = m_gHistory[MAXVALS-winSize-maxLag+i];
            }
            //reset nrOfValuesStored to appropriate new value
            setNrOfValuesStored(winSize+maxLag);
    }

//    following lines are now handled inside an if statement to handle wininc displaying properly
//    //nooby remark/reminder % is modulo in C++ so it is modulo getImgHeight()
//    //the currentdisplayline variable is purposly decoupled from the nrofvalues variable
//    //on the moment of reset as the number of setvalues is always>=winsize+maxlag but the
//    setCurrentDisplayLine((getCurrentDisplayLine()+1) % getImgHeight());


    //if enough values have been filled, start calculating lagged crosscorrelations
    if (getNrOfValuesStored() >= winSize+maxLag)
    {
         double r = 0;
            //debug only
//            double r2 = 0;
         int lag = 0;
         int pixel = 0;
         int val=0;

         //in original bokerview ParleVision v4 lagshift and wininc where not used
         //they were and still are dependant on imgheight and imgwidth

         //skip calculation steps if maxLag is set that way
		 //TODO using modulo here might be an expensive function 
         if (getNrOfValuesStored() % getWinInc() == 0)
         {

           setCurrentDisplayLine((getCurrentDisplayLine()+1) % getImgHeight());
           // TODO
           // skip calculation steps but draw if "resolution" of calculation is set lower with lagShift
           // if (lag % getLagShift() == 0)
           //   {

           //for each pixel:
           //pixel will has imgwidth values 0...319 when using width =640
           //0...618 using 639 which equals an automatically odded image width of 640
             for (pixel=0; pixel < getImgWidth()/2; pixel++)
             {

                //calculate lag for this pixel:
                //-center displays lag=0
                //-left pixel displays correlation with f and g-lag
                //-right pixel displays correlation with f-lag and g

                //DID
                // think there was an error here, it never reached maxlag as max pixel=imwid/2 -1, due to< sign
                // indeed lag 0..19
                //resulting (combined with even img width) in a broader stroke in the middle as values are calculated near the center using the same lag
                //original :
                 //lag = (int)((double)(pixel*2*maxLag)/(double)imgwidth);

                 //better combined with adding zero
                 //maybe TODO use old value if lag is same as previous lag, maybe add modulo %lagshift here as well
                 lag = (int)((double)((pixel)*2*maxLag)/(double)getImgWidth())+1;
//                qDebug() << "pixel : " << pixel << "lag : " << lag << "639/2" << getImgWidth()/2 ;
                    //it is evaluated with value+i and i max is winsize-1 so it never reaches the nr of valuesstored
                    //but as the history starts counting at 0 the number of values are correct and the last value is always in history[nrofvalues-1] this is correct
                    r = corr(getNrOfValuesStored()-lag-winSize,getNrOfValuesStored()-winSize);
                   // r2 = corrwiki(getNrOfValuesStored()-lag-winSize,getNrOfValuesStored()-winSize);

                    //TODO inspect
                    //this square implicitly assumes that negative correlation is just as relevant as positive correlation,
                    //which is not true according to bokers peak finding method for synchrony
                    // as well as an example of difference between negative and positive correlation measured by velocity in turn taking
                    //and also in a social signal way there could be a difference between adapting your pause and reacting on action.
                    //maybe use a 0-255 frame ranging from -1 to 1,  so -1 is black and 1 is white
                    //squaring also changes the amount of white pixels drastically
                    //it is also strange that abs is used along with r*r as it can never be negative anyway
                    //qDebug() << "R values lag left " << getNrOfValuesStored() << "pixel" << pixel << " r: " << r;
                    //qDebug() << "imgwidth/2+pixel values" << getImgWidth()/2+pixel;
                    if (getCorSquared())
                    {
                        val = (int)abs(((double)255)*r*r);
                    }
                    else
                    {
                        val = (int)(((double)255)*((r+1)/2));
                    }

                    //TODO inspect
                    //As in the previous version it uses cvcircle to draw the image but it actually draws points isn't this an ineffective method?
                    //put the values in and draw each pixel as a circular shape with radius 1
                    //cvCircle( CvArr* img, CvPoint center, int radius, double color, int thickness=1 ) if thickness negative indicates a filled circle
                    //from point x=320...320-319=1 and 320...320+319=639
//                    cvCircle(dst, cvPoint(getImgWidth()/2-pixel,getCurrentDisplayLine()), 1, CV_RGB(val,val,val), -1);
                    cvCircle(dst, cvPoint(getImgWidth()/2-pixel-1,getCurrentDisplayLine()), 1, CV_RGB(val,val,val), -1);
                    r = corr(getNrOfValuesStored()-winSize,getNrOfValuesStored()-lag-winSize);
//                    r2 = corrwiki(getNrOfValuesStored()-winSize,getNrOfValuesStored()-lag-winSize);

//                    qDebug() << "R values lag right " << getNrOfValuesStored() << "pixel" << pixel << " r: " << r;
//                    qDebug() << "getImgWidth()/2-pixel values" << getImgWidth()/2-pixel;
                    if (getCorSquared())
                    {
                        val = (int)abs(((double)255)*r*r);
                    }
                    else
                    {
                        val = (int)(((double)255)*((r+1)/2));
                    }
//                    cvCircle(dst, cvPoint(getImgWidth()/2+pixel,getCurrentDisplayLine()), 1, CV_RGB(val,val,val), -1);
                    cvCircle(dst, cvPoint(getImgWidth()/2+pixel+1,getCurrentDisplayLine()), 1, CV_RGB(val,val,val), -1);


               }
             //added 0 outside the for loop
             lag = 0;
             r = corr(getNrOfValuesStored()-winSize,getNrOfValuesStored()-lag-winSize);
             val = (int)abs(((double)255)*r*r);
             cvCircle(dst, cvPoint(getImgWidth()/2+1,getCurrentDisplayLine()), 1, CV_RGB(val,val,val), -1);
         }
         else
         {
//                    void cvLine(CvArr* img, CvPoint pt1, CvPoint pt2, CvScalar color, int thickness=1, int lineType=8, int shift=0)
             cvLine(dst, cvPoint (1,getCurrentDisplayLine()), cvPoint (getImgWidth(),getCurrentDisplayLine()), CV_RGB(0,0,0),1, 8, 0);
         }
    }

    else //not enough values yet: draw black/white pattern
    {
            //used to be in front of the if loop but now uses the wininc aswell
            setCurrentDisplayLine((getCurrentDisplayLine()+1) % getImgHeight());

            int pixel = 0;
            for (pixel = 0; pixel < getImgWidth()/2; pixel++)
            {
                    cvCircle(dst, cvPoint(pixel,getCurrentDisplayLine()), 1, CV_RGB(255*(pixel%2),255*(pixel%2),255*(pixel%2)), -1);
                    cvCircle(dst, cvPoint(getImgWidth()-pixel,getCurrentDisplayLine()), 1, CV_RGB(255*(pixel%2),255*(pixel%2),255*(pixel%2)), -1);
            }

    }

    //when enough frames are processed for drawing the correlation figure
    if (dst != NULL)
    {
        m_outputPin->put(dst);

    }

    else
    {
//        m_outputString->put("test NULL");
    }

    return true;

}

//TODO add get set structure in processor instead of header

//e.g. the dilate erode pipelinelement:
//void DilateErode::setDilationIterations(int d)
//{
//    QMutexLocker lock( m_propertyMutex );
//    if( d >= 0 )
//        m_dilationIterations = d;
//    emit dilationIterationsChanged(m_dilationIterations);
//}

//int DilateErode::getErosionIterations() const
//{
//    QMutexLocker lock( m_propertyMutex );
//    return m_erosionIterations;
//}

// from Dennis Reidsma ParleVision 4.0
// TODO analyse on correctness with paper and logic and appropriatness of using the algorithm
// function needs the globally saved history: m_fHistory m_gHistory
// to simply formula used by Boker 2002, p8 to algorithms analyszed

///////////////////////////////////////////////////////////////////////////////////////////////////
// non-AbstractProcessor interface specific functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/* calculate correlation for two windows of data, which starting at w1 and w2 
//r =(n*sum(xy)-sum(x)*sum(y))/sqrt((n*sum(x^2)-sum(x)^2)*(n*sum(y^2)-sum(y)^2)) */
double PlvSynchronyProcessor::corr(long fIndex, long gIndex)
{
        //http://algorithmsanalyzed.blogspot.com/2008/07/bellkor-algorithm-pearson-correlation.html
        double sumF = 0;
        double sumG = 0;
        double sumFG = 0;
        double sumSqrF = 0;
        double sumSqrG = 0;
        int winSize = getWinSize();
        for (int i = 0; i <  winSize; i++)
        {
                sumF += m_fHistory[fIndex+i];
                sumG += m_gHistory[gIndex+i];
                sumFG += m_fHistory[fIndex+i]*m_gHistory[gIndex+i];
                sumSqrF += m_fHistory[fIndex+i]*m_fHistory[fIndex+i];
                sumSqrG += m_gHistory[gIndex+i]*m_gHistory[gIndex+i];
        }
        double pearson = (sumFG-(sumF*sumG)/winSize)/sqrt((sumSqrF-sumF*sumF/winSize)*(sumSqrG-sumG*sumG/winSize));
        //double pearsonwiki = ((winSize * sumFG) - sumF *sumG )/(sqrt(winSize*sumSqrF-sumF*sumF)*sqrt(winSize*sumSqrG-sumG*sumG));
        return pearson;
}

//Pearson simplicifaction seems to be identical
/*
double PlvSynchronyProcessor::corrwiki(long fIndex, long gIndex)
{
        //http://algorithmsanalyzed.blogspot.com/2008/07/bellkor-algorithm-pearson-correlation.html
        double sumF = 0;
        double sumG = 0;
        double sumFG = 0;
        double sumSqrF = 0;
        double sumSqrG = 0;
        int winSize = getWinSize();
        for (int i = 0; i <  winSize; i++)
        {
                sumF += m_fHistory[fIndex+i];
                sumG += m_gHistory[gIndex+i];
                sumFG += m_fHistory[fIndex+i]*m_gHistory[gIndex+i];
                sumSqrF += m_fHistory[fIndex+i]*m_fHistory[fIndex+i];
                sumSqrG += m_gHistory[gIndex+i]*m_gHistory[gIndex+i];
        }
      //  double pearson = (sumFG-(sumF*sumG)/winSize)/sqrt((sumSqrF-sumF*sumF/winSize)*(sumSqrG-sumG*sumG/winSize));

        double pearsonwiki = ((winSize * sumFG) - sumF *sumG )/(sqrt(winSize*sumSqrF-sumF*sumF)*sqrt(winSize*sumSqrG-sumG*sumG));

        return pearsonwiki;
}
*/
