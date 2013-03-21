/**
  * Copyright (C)2010 by Michel Jansen and Richard Loos
  * All rights reserved.
  *
  * This file is part of the plvopencv module of ParleVision.
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

#ifndef RESIZE_H
#define RESIZE_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/Pin.h>
#include <plvcore/Enum.h>
#include <plvcore/CvMatData.h>
#include <plvcore/Types.h>

//*2 ??
#define STITCH_DESTINATION_WIDTH_DEFAULT_RESIZE (320)
#define STITCH_DESTINATION_HEIGHT_DEFAULT_RESIZE (240)

#define STITCH_DESTINATION_WIDTH_MAX_RESIZE (1920)
#define STITCH_DESTINATION_HEIGHT_MAX_RESIZE (1080)

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
    class CvMatData;
}

namespace plvopencv
{
    class Resize : public plv::PipelineProcessor
    {
        Q_OBJECT
        //Q_DISABLE_COPY( Resize )
        Q_CLASSINFO("author", "Robby van Delden")
        Q_CLASSINFO("name", "Resize/scale image")
        Q_CLASSINFO("description", "Resize an image"
                    "Can be used together with stitch to save a huge stitched image on a slow HDD in real time."
					"INTER_AREA may be the preferred method for image decimation, as it gives moire-free results. But when the image is zoomed, it is similar to the INTER_NEAREST method"
					"To shrink an image, it will generally look best with CV_INTER_AREA interpolation, whereas to enlarge an image, it will generally look best with CV_INTER_CUBIC (slow) or CV_INTER_LINEAR (faster but still looks OK)."
					)
		
		/** required standard method declaration for plv::PipelineProcessor */
        PLV_PIPELINE_PROCESSOR

		Q_PROPERTY( int destinationWidth READ getDestinationWidth WRITE setDestinationWidth NOTIFY destinationWidthChanged )
        Q_PROPERTY( int destinationHeight READ getDestinationHeight WRITE setDestinationHeight NOTIFY destinationHeightChanged )
		Q_PROPERTY( plv::Enum interpolation READ getInterpolation WRITE setInterpolation NOTIFY interpolationChanged )
       
    public:
        Resize();
        virtual ~Resize();
		
		int getDestinationWidth();
        int getDestinationHeight();
		plv::Enum getInterpolation() const;
		/** */
        //virtual bool init();
	
	public slots:
		void setDestinationHeight( int height );
        void setDestinationWidth( int width );
		void setInterpolation(plv::Enum e);

	signals:
		void destinationHeightChanged( int height );
        void destinationWidthChanged( int width );
		void interpolationChanged(plv::Enum newValue);
    
	private:
        plv::CvMatDataInputPin* m_imageInput;
        plv::CvMatDataOutputPin* m_outputPin;

		plv::Enum m_interpolation; /** The file extention and thus the format in which the data is stored. */
		int interpolationtype;

		int destinationWidth;
        int destinationHeight;

    };
}

#endif // RESIZE_H
