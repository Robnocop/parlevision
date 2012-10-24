/** @file
 * \$Id: SumPixels.cpp,v 1.1 2009/03/19 11:29:54 dennisr Exp $
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
#include "SumPixels.h"
#include "highgui.h"

namespace Processors {

/// constructor
SumPixels::SumPixels()
{
  //defineProcessor(inPins, outPins, Name, Category, Description, PropertyResourceID)
	defineProcessor(1, 1, "SumPixels", "Synchrony Processors", "Calculate the sum of all pixel values in the input using cvSumPixels", IDD_PROPSHEET_SUM);
	
  //definePin(PinType,	CONSTANT_ID,				Name,			DataType)
	definePin(eInputPin,	SUM_PIN_IN_INPUT,		"input",		eIplImage3C);
	definePin(eOutputPin,	SUM_PIN_OUT_SUM,		"sum",		eDouble);

}


/// destructor
SumPixels::~SumPixels()
{

}


/// function that gets called before processing starts
bool SumPixels::Init(int _width, int _height, int _depth)
{
	sumout = 0;
	// init output pointers
	setOutputPinPtr(SUM_PIN_OUT_SUM, &sumout);

	// let super finish it off
	return AbstractProcessor::Init(_width, _height, _depth);
}


/// function that gets called to process each frame
bool SumPixels::Process()
{
	const IplImage* inputImage = getInputPinImagePtr(SUM_PIN_IN_INPUT);

	sumout = cvSumPixels(inputImage);

	return true;
}

/// callback function to handle interaction with our own Property Page
BOOL CALLBACK SumPixels::OptionsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
	case WM_INITDIALOG:
		{
			return true; 
		} break;
	default:
		return false; // let DefDlgProc handle message
	}
    return true;
}


/// generates config XML node for this Processor so it can be saved in a file
CkXml* SumPixels::xmlGenerateConfigNode()
{
	// empty node
    CkXml*		node = new CkXml();
	CkXml*		settings = new CkXml();
	settings->put_Tag("Settings");
	node->AddChildTree(settings);
	delete settings;
	return node;
}


/// called by framework to read the config XML node and load these settings
void SumPixels::xmlReadConfigNode(CkXml* node)
{
	CkXml* settings = node->GetChildWithTag("Settings");
	if (settings == NULL) { MessageBox(NULL, "Processor XML parse error!\nXML node object == NULL", "Error!", MB_ICONEXCLAMATION | MB_OK); return; }
	delete settings;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// non-AbstractProcessor interface specific functions
///////////////////////////////////////////////////////////////////////////////////////////////////


} // namespace Processors {