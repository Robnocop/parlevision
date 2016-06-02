/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * All rights reserved.
  *
  * This file is part of the plvcore module of ParleVision.
  *
  * ParleVision is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * ParleVision is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * A copy of the GNU General Public License can be found in the root
  * of this software package directory in the file LICENSE.LGPL.
  * If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef PIXELSUMS_H
#define PIXELSUMS_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/Pin.h>
#include <plvcore/Types.h>
#include <opencv/cv.h>

namespace plv
{
    class CvMatDataInputPin;

    /**
      * PixelSum of two images.
      */
    class PixelSumS : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( PixelSumS )
        Q_CLASSINFO("author", "Richard Loos")
        Q_CLASSINFO("name", "Pixel Sum2")
        Q_CLASSINFO("description", "The functions sum calculate and return the sum of array elements, "
                    "independently for each channel. This version divides the sum/255 in order to account for the white values. Returns a Double.");

        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR

    public:
        PixelSumS();
        virtual ~PixelSumS();

    private:
        plv::CvMatDataInputPin* m_inputPin;

//        plv::InputPin<double>* m_inputPin;
//        plv::OutputPin<cv::Scalar>* m_outputPin;
        plv::OutputPin<double>* m_outputPin;
		plv::OutputPin<QString>* m_outputPin2;
    };
}
#endif // PIXELSUMS_H
