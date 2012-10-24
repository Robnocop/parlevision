/** @file
 * \$Id: BokerView.cpp,v 1.1 2009/03/19 11:29:54 dennisr Exp $
 *
 * @par Project
 * ParleVision ~ HMI software for development of vision projects\n
 * University of Twente, cluster Human Media Interaction
 * 
 * @par License
 * &copy; University of Twente, cluster Human Media Interaction\n
 * Use is strictly forbidden without prior consent of the owners.
 */

#include "stdafx.h"
#include "BokerView.h"
#include "highgui.h"
#include "math.h"

namespace Processors {

/// constructor
BokerView::BokerView()
{
  //defineProcessor(inPins, outPins, Name, Category, Description, PropertyResourceID)
	defineProcessor(2, 1, "BokerView", "Synchrony Processors", "Given two input functions (double values), calculate the time lagged crosscorrelation according to boker2003.", IDD_PROPSHEET_BOKERVIEW);

  //definePin(PinType,	CONSTANT_ID,				Name,					DataType)
	definePin(eInputPin,	BOKERVIEW_PIN_IN_F,		"f",				eDouble);
	definePin(eInputPin,	BOKERVIEW_PIN_IN_G,		"g",				eDouble);
	definePin(eOutputPin,	BOKERVIEW_PIN_OUT_PREVIEW,	"preview",					eIplImage);

	inputFunctionF = 0;
	inputFunctionG = 0;
	outputPreview = NULL;

	winSize = 40; //4 sec with 10fps?
	winInc = 4;
	maxLag =20;
	lagShift = 4;
	nrOfValuesStored = 0;
	for (int i = 0; i<MAXVALS; i++)
	{
		fHistory[i] = 0;
		gHistory[i] = 0;
	}

}


/// destructor
BokerView::~BokerView()
{
	if (outputPreview != NULL)
		cvReleaseImage(&outputPreview);
}


/// function that gets called before processing starts
bool BokerView::Init(int _width, int _height, int _depth)
{
	imgwidth = _width;
	imgheight = _height;
	imgdepth = _depth;
        currentDisplayLine = 0;

	// create internal images
	if (outputPreview == NULL)
		outputPreview = cvCreateImage(cvSize(_width, _height), _depth, 3);

	// init output pointers
	setOutputPinPtr(BOKERVIEW_PIN_OUT_PREVIEW, outputPreview);

	// let super finish it off
	return AbstractProcessor::Init(_width, _height, _depth);
}


/// function that gets called to process each frame
bool BokerView::Process()
{
	const double* fInput = (double*)getInputPinDataPtr(BOKERVIEW_PIN_IN_F);
	const double* gInput = (double*)getInputPinDataPtr(BOKERVIEW_PIN_IN_G);
	
	//add values to buffer
	fHistory[nrOfValuesStored]=*fInput;
	gHistory[nrOfValuesStored]=*gInput;
	nrOfValuesStored++;
	if (nrOfValuesStored>=MAXVALS) 
	{
		//copy exactly enough values for a full lagged analysis from end of histories to start; 
		//needed: winsize+maxlag frames from both functions
		for (int i = 0; i < winSize+maxLag; i++) {
			fHistory[i] = fHistory[MAXVALS-winSize-maxLag+i];
			gHistory[i] = gHistory[MAXVALS-winSize-maxLag+i];
		}
		//reset nrOfValuesStored to appropriate new value
		nrOfValuesStored = winSize+maxLag;
	}


        currentDisplayLine = (currentDisplayLine+1) % imgheight;

	//if enough values have been filled, start calculating lagged crosscorrelations
	if (nrOfValuesStored >= winSize+maxLag) 
	{
		double r = 0;
		int lag = 0;
		int pixel = 0;
		int val=0;
	    //for each pixel:
		for (pixel=0; pixel < imgwidth/2; pixel++) 
		{
			//calculate lag for this pixel:
			//-center displays lag=0
			//-left pixel displays correlation with f and g-lag
			//-right pixel displays correlation with f-lag and g
			lag = (int)((double)(pixel*2*maxLag)/(double)imgwidth);
			//draw result directly on screen.

			r = corr(nrOfValuesStored-lag-winSize,nrOfValuesStored-winSize);
			val = (int)abs(((double)255)*r*r);
                        cvCircle(outputPreview, cvPoint(imgwidth/2-pixel,currentDisplayLine), 1, CV_RGB(val,val,val), -1);
			r = corr(nrOfValuesStored-winSize,nrOfValuesStored-lag-winSize);
			val = (int)abs(((double)255)*r*r);
			cvCircle(outputPreview, cvPoint(imgwidth/2+pixel,currentDisplayLine), 1, CV_RGB(val,val,val), -1);

		}
		
	}
	else //not enough values yet: draw black/white pattern
	{   
		int pixel = 0;
		for (pixel = 0; pixel < imgwidth/2; pixel++) 
		{
			cvCircle(outputPreview, cvPoint(pixel,currentDisplayLine), 1, CV_RGB(255*(pixel%2),255*(pixel%2),255*(pixel%2)), -1);
			cvCircle(outputPreview, cvPoint(imgwidth-pixel,currentDisplayLine), 1, CV_RGB(255*(pixel%2),255*(pixel%2),255*(pixel%2)), -1);
		}		
	}
	return true;
}

/// callback function to handle interaction with our own Property Page
BOOL CALLBACK BokerView::OptionsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
	case WM_INITDIALOG:
		{
			HINSTANCE hInstance = GetModuleHandle(NULL);

			char buffer[50];

			// set values
			sprintf(buffer, "%d", winSize);
			SendMessage(GetDlgItem(hWnd, IDC_BOKER_WINSIZE), WM_SETTEXT, 0, (LPARAM) buffer);
			sprintf(buffer, "%d", winInc);
			SendMessage(GetDlgItem(hWnd, IDC_BOKER_WININC), WM_SETTEXT, 0, (LPARAM) buffer);
			sprintf(buffer, "%d", maxLag);
			SendMessage(GetDlgItem(hWnd, IDC_BOKER_MAXLAG), WM_SETTEXT, 0, (LPARAM) buffer);
			sprintf(buffer, "%d", lagShift);
			SendMessage(GetDlgItem(hWnd, IDC_BOKER_LAGINC), WM_SETTEXT, 0, (LPARAM) buffer);
			return true; 
		} break;
    case WM_COMMAND:
		switch(HIWORD(wParam))
		{

		case BN_CLICKED:
			switch(LOWORD(wParam))
			{
			case IDC_SPLITTER_APPLY: 
				{
				char buffer[40];
				SendMessage(GetDlgItem(hWnd, IDC_BOKER_WINSIZE), WM_GETTEXT, 10, (LPARAM) &buffer);
				sscanf(buffer, "%d", &winSize);
				SendMessage(GetDlgItem(hWnd, IDC_BOKER_WININC), WM_GETTEXT, 10, (LPARAM) &buffer);
				sscanf(buffer, "%d", &winInc);
				SendMessage(GetDlgItem(hWnd, IDC_BOKER_MAXLAG), WM_GETTEXT, 10, (LPARAM) &buffer);
				sscanf(buffer, "%d", &maxLag);
				SendMessage(GetDlgItem(hWnd, IDC_BOKER_LAGINC), WM_GETTEXT, 10, (LPARAM) &buffer);
				sscanf(buffer, "%d", &lagShift);
				
				sprintf(buffer, "%d", winSize);
				SendMessage(GetDlgItem(hWnd, IDC_BOKER_WINSIZE), WM_SETTEXT, 0, (LPARAM) buffer);
				sprintf(buffer, "%d", winInc);
				SendMessage(GetDlgItem(hWnd, IDC_BOKER_WININC), WM_SETTEXT, 0, (LPARAM) buffer);
				sprintf(buffer, "%d", maxLag);
				SendMessage(GetDlgItem(hWnd, IDC_BOKER_MAXLAG), WM_SETTEXT, 0, (LPARAM) buffer);
				sprintf(buffer, "%d", lagShift);
				SendMessage(GetDlgItem(hWnd, IDC_BOKER_LAGINC), WM_SETTEXT, 0, (LPARAM) buffer);
				}
				break;
		    }
		    break;
		} break;
	default:
		return false; // let DefDlgProc handle message
	}
    return true;}


/// generates config XML node for this Processor so it can be saved in a file
CkXml* BokerView::xmlGenerateConfigNode()
{
	// empty node
    CkXml*		node = new CkXml();
	CkXml*		settings = new CkXml();
	settings->put_Tag("Settings");
	Util::xmlAddAttributeInt(settings, "winSize", winSize);
	Util::xmlAddAttributeInt(settings, "winInc", winInc);
	Util::xmlAddAttributeInt(settings, "maxLag", maxLag);
	Util::xmlAddAttributeInt(settings, "lagShift", lagShift);
	node->AddChildTree(settings);
	delete settings;
	return node;
}


/// called by framework to read the config XML node and load these settings
void BokerView::xmlReadConfigNode(CkXml* node)
{
	CkXml* settings = node->GetChildWithTag("Settings");
	if (settings == NULL) { MessageBox(NULL, "Processor XML parse error!\nXML node object == NULL", "Error!", MB_ICONEXCLAMATION | MB_OK); return; }
	winSize				= Util::xmlGetAttributeInt(settings, "winSize");
	winInc				= Util::xmlGetAttributeInt(settings, "winInc");
	maxLag				= Util::xmlGetAttributeInt(settings, "maxLag");
	lagShift			= Util::xmlGetAttributeInt(settings, "lagShift");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// non-AbstractProcessor interface specific functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/* calculate correlation for two windows of data, which starting at w1 and w2 
r =(n*sum(xy)-sum(x)*sum(y))/sqrt((n*sum(x^2)-sum(x)^2)*(n*sum(y^2)-sum(y)^2)) */
double BokerView::corr(long fIndex, long gIndex)
{
	//http://algorithmsanalyzed.blogspot.com/2008/07/bellkor-algorithm-pearson-correlation.html
	double sumF = 0;
	double sumG = 0;
	double sumFG = 0; 
        double sumSqrF = 0;
	double sumSqrG = 0;
	for (int i = 0; i < winSize; i++)
	{
		sumF += fHistory[fIndex+i];
		sumG += gHistory[gIndex+i];
		sumFG += fHistory[fIndex+i]*gHistory[gIndex+i];
		sumSqrF += fHistory[fIndex+i]*fHistory[fIndex+i];
		sumSqrG += gHistory[gIndex+i]*gHistory[gIndex+i];
	}
	double pearson = (sumFG-(sumF*sumG)/winSize)/sqrt((sumSqrF-sumF*sumF/winSize)*(sumSqrG-sumG*sumG/winSize));
	return pearson;
}

} // namespace Processors {
