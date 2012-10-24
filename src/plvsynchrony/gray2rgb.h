/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * All rights reserved.
  *
  * The original file was part of the plvcore module of ParleVision being the pixelsums.
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

#ifndef GRAY2RGB_H
#define GRAY2RGB_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/Pin.h>
#include <plvcore/Types.h>
#include <opencv/cv.h>

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
}
/**
    * conversion of an image.
    */
class Gray2RGB : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( Gray2RGB )
    Q_CLASSINFO("author", "Robby van Delden")
    Q_CLASSINFO("name", "Gray to RGB converter")
    Q_CLASSINFO("description", "The function transforms an 8bit or 16bit gray scale picture to a 8bit RGB, "
                "This makes analyses way easier as a distinction between values will become visible.");

    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR

public:
    Gray2RGB();
    virtual ~Gray2RGB();

private:
    plv::CvMatDataInputPin* m_inputPin;

    plv::CvMatDataOutputPin* m_outputPin;
};

#endif // GRAY2RGB_H
