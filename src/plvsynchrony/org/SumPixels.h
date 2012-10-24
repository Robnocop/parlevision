/** @file
 * \$Id: SumPixels.h,v 1.1 2009/03/19 11:29:54 dennisr Exp $
 *
 * @par Project
 * ParleVision ~ HMI software for development of vision projects\n
 * University of Twente, cluster Human Media Interaction
 * 
 * @par License
 * &copy; University of Twente, cluster Human Media Interaction\n
 * Use is strictly forbidden without prior consent of the owners.
 */

#ifndef __SUMPIXELS_H__
#define __SUMPIXELS_H__


// includes
#include "../Include/ProcBase.h"
#include "resource.h"

namespace Processors
{
	// constants

	// pin constants
	#define SUM_PIN_IN_INPUT					0
	#define SUM_PIN_OUT_SUM						0


	/**  Processor that uses cvSumPixels to calculate the sum of all pixel values in an image
	*
	* @authors Dennis Reidsma
	*
	*/
	class SumPixels : public AbstractProcessor
	{
	public:
		// see AbstractProcessor.h for #define
		ABSTRACTPROCESSOR_DEFAULT_DECLARATIONS(SumPixels)

	protected:

	private:
		double		sumout;

	};
} // namespace Processors {

#endif