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

#ifndef PIXELSUMS2_H
#define PIXELSUMS2_H

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
    class PlvAdvancedTrackingSum : public plv::PipelineProcessor
    {
        Q_OBJECT
        Q_DISABLE_COPY( PlvAdvancedTrackingSum )
        Q_CLASSINFO("author", "Richard Loos")
        Q_CLASSINFO("name", "Pixel SumTest")
        Q_CLASSINFO("description", "The functions sum calculate and return the sum of array elements, "
                    "independently for each channel. Returns a Double.");

        /** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR

    public:
        PlvAdvancedTrackingSum();
        virtual ~PlvAdvancedTrackingSum();

    private:
        plv::CvMatDataInputPin* m_inputPin;

//        plv::InputPin<double>* m_inputPin;
//        plv::OutputPin<cv::Scalar>* m_outputPin;
        plv::OutputPin<double>* m_outputPin;
    };
}
#endif // PIXELSUMS_H
