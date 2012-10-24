/** @file
 * \$Id: BokerView.h,v 1.1 2009/03/19 11:29:54 dennisr Exp $
 *
 * @par Project
 * ParleVision ~ HMI software for development of vision projects\n
 * University of Twente, cluster Human Media Interaction
 * 
 * @par License
 * &copy; University of Twente, cluster Human Media Interaction\n
 * Use is strictly forbidden without prior consent of the owners.
 */

#ifndef __BOKERVIEW_H__
#define __BOKERVIEW_H__


// includes
#include "../Include/ProcBase.h"
#include "resource.h"
#include "HighGui.h"
#include "utils.h"

namespace Processors
{
	// constants

	// pin constants
	#define BOKERVIEW_PIN_IN_F   			0
	#define BOKERVIEW_PIN_IN_G   			1
	#define BOKERVIEW_PIN_OUT_PREVIEW			0
	
	/**  
	* Given two input functions (double values), calculate the time lagged crosscorrelation according to boker2003
	* @authors Dennis Reidsma
	*
	*/
	class BokerView : public AbstractProcessor
	{
	public:
		// see AbstractProcessor.h for #define
		ABSTRACTPROCESSOR_DEFAULT_DECLARATIONS(BokerView)

		double corr(long fIndex, long gIndex);

	protected:

	private:
		double*		inputFunctionF;
		double*		inputFunctionG;
		IplImage*		outputPreview;

		int				winSize; //in frames! cannot be larger than n?
		int				winInc; //in frames!
		int				maxLag;//in frames!
		int				lagShift;//in frames!

		int imgwidth;
		int imgheight;
		int imgdepth;
		int currentDisplayLine;

		//storage of f and g history. When full, reset?
		const static long MAXVALS = 10000;
		double fHistory[10000]; //size??!!
		double gHistory[10000]; //size??!!
		long nrOfValuesStored;
	};
} // namespace Processors {

#endif