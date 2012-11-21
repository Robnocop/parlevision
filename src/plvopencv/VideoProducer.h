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

#ifndef VIDEOPRODUCER_H
#define VIDEOPRODUCER_H

#include <plvcore/PipelineProducer.h>
#include <plvcore/Pin.h>
#include <plvcore/CvMatData.h>

//used for fps reduction to fit actual fps
#include <QTime>

namespace plv
{
    class CvMatDataOutputPin;
}

namespace cv
{
    class VideoCapture;
}

namespace plvopencv
{
    class VideoProducer : public plv::PipelineProducer
    {
        Q_OBJECT
        Q_CLASSINFO("author", "Richard Loos")
        Q_CLASSINFO("name", "VideoProducer")
        Q_CLASSINFO("description", "" )

        Q_PROPERTY( QString filename READ getFilename WRITE setFilename NOTIFY filenameChanged )
        Q_PROPERTY( QString directory READ getDirectory WRITE setDirectory NOTIFY directoryChanged )
		Q_PROPERTY( bool fpsLimit READ getFpsLimit WRITE setFpsLimit NOTIFY fpsLimitChanged )

        /** required standard method declaration for Producer */
        PLV_PIPELINE_PRODUCER

    public:
        VideoProducer();
        virtual ~VideoProducer();

        virtual bool init();
        virtual bool deinit() throw();

        /** property methods **/
        QString getFilename();
        void updateFilename(const QString& s){ setFilename(s); filenameChanged(s); }

        QString getDirectory();
        void updateDirectory(const QString& s){ setDirectory(s); directoryChanged(s); }

		//GUI controlled settings
		bool getFpsLimit() {return fpsLimit;}

    signals:
        void filenameChanged(const QString& newValue);
        void directoryChanged(const QString& newValue);
		void fpsLimitChanged (bool b);

    public slots:
        void setFilename(const QString& filename);
        void setDirectory(const QString& directory);
		void setFpsLimit(bool b) {fpsLimit = b; emit (fpsLimitChanged(b));}

    private:
        QString m_filename;  /** the filename of the image to load */
        QString m_directory; /** the directory which contains the image. */
        int m_frameCount; /** total frame count of the video */
        long m_posMillis; /** Film current position in milliseconds or video capture timestamp */
        double m_ratio; /** Relative position of the video file (0 - start of the film, 1 - end of the film) */
        int m_fps; /** frame rate of the video */
		
		QTime m_fpstimer;
		int m_prevtime;

        plv::CvMatData m_frame;
        plv::CvMatDataOutputPin* m_outputPin;
        plv::OutputPin<int>* m_outFrameCount;
        plv::OutputPin<long>* m_outPositionMillis;
        plv::OutputPin<double>* m_outRatio;
        plv::OutputPin<int>* m_outFps;
        cv::VideoCapture* m_capture;

		//GUI controlled settings
		bool fpsLimit;

        /** This method checks whether the extension of the filename is one of the
          * accepted extensions for images by OpenCV. See:
          * http://opencv.willowgarage.com/documentation/c/reading_and_writing_images_and_video.html */
        static bool validateExtension(const QString& filename);
    };
}

#endif // VIDEOPRODUCER_H
