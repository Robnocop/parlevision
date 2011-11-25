#ifndef HMIFREENECT_H
#define HMIFREENECT_H
/*
 * This file is part of the OpenKinect Project. http://www.openkinect.org
 *
 * Copyright (c) 2010 individual OpenKinect contributors. See the CONTRIB file
 * for details.
 *
 * This code is licensed to you under the terms of the Apache License, version
 * 2.0, or, at your option, the terms of the GNU General Public License,
 * version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
 * or the following URLs:
 * http://www.apache.org/licenses/LICENSE-2.0
 * http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * If you redistribute this file in source form, modified or unmodified, you
 * may:
 *   1) Leave this header intact and distribute it under the same terms,
 *      accompanying it with the APACHE20 and GPL20 files, or
 *   2) Delete the Apache 2.0 clause and accompany it with the GPL2 file, or
 *   3) Delete the GPL v2 clause and accompany it with the APACHE20 file
 * In all cases you must keep the copyright notice intact and include a copy
 * of the CONTRIB file.
 *
 * Binary distributions must follow the binary distribution requirements of
 * either License.
 *
 * ==========================================================================
 * This is a modified version of libfreenect.hpp from the libfreenect libraries.
 * It has been adapted to use the QMutexLocker libraries for locking, and to not
 * use a separate thread for processing the freenecht events.
 *
 * Dennis Reidsma, Human Media Interaction, University of Twente, the Netherlands
 */

#pragma once

#include <libfreenect.h>
#include <stdexcept>
#include <map>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

namespace plvfreenect {

    class KinectMutex
    {
      public:
        static QMutex m_hmifreenectMutex;
    };



    class Noncopyable {
          public:
                Noncopyable() {}
                ~Noncopyable() {}
          private:
                Noncopyable( const Noncopyable& );
                const Noncopyable& operator=( const Noncopyable& );
        };

        class FreenectTiltState {
          friend class HmiFreenectDevice;
                FreenectTiltState(freenect_raw_tilt_state *_state):
                        m_code(_state->tilt_status), m_state(_state)
                {}
          public:
                void getAccelerometers(double* x, double* y, double* z) {
                        QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
                        freenect_get_mks_accel(m_state, x, y, z);
                }
                double getTiltDegs() {
                        QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
                        return freenect_get_tilt_degs(m_state);
                }
          public:
                freenect_tilt_status_code m_code;
          private:
                freenect_raw_tilt_state *m_state;
        };

        class HmiFreenectDevice : Noncopyable {
          public:
                HmiFreenectDevice(freenect_context *_ctx, int _index);
                virtual ~HmiFreenectDevice();
                void startVideo();
                void stopVideo();
                void startDepth();
                void stopDepth();
                void setTiltDegrees(double _angle);
                void setLed(freenect_led_options _option);
                void updateState();
                FreenectTiltState getState();
                void setVideoFormat(freenect_video_format requested_format);
                freenect_video_format getVideoFormat();
                void setDepthFormat(freenect_depth_format requested_format);
                freenect_depth_format getDepthFormat();
                // Do not call directly even in child
                void VideoCallback(void* _video, uint32_t timestamp);
                // Do not call directly even in child
                void DepthCallback(void* _depth, uint32_t timestamp);
                bool getVideo(cv::Mat& output);
                bool newVideoAvailable();
                bool getDepth(cv::Mat& output);
                bool newDepthAvailable();
          protected:
                /*int getVideoBufferSize(){
                        if(m_video_format == FREENECT_VIDEO_RGB) return FREENECT_VIDEO_RGB_SIZE;
                        if(m_video_format == FREENECT_VIDEO_BAYER) return FREENECT_VIDEO_BAYER_SIZE;
                        if(m_video_format == FREENECT_VIDEO_IR_8BIT) return FREENECT_VIDEO_IR_8BIT_SIZE;
                        if(m_video_format == FREENECT_VIDEO_IR_10BIT) return FREENECT_VIDEO_IR_10BIT_SIZE;
                        if(m_video_format == FREENECT_VIDEO_IR_10BIT_PACKED) return FREENECT_VIDEO_IR_10BIT_PACKED_SIZE;
                        if(m_video_format == FREENECT_VIDEO_YUV_RGB) return FREENECT_VIDEO_YUV_RGB_SIZE;
                        if(m_video_format == FREENECT_VIDEO_YUV_RAW) return FREENECT_VIDEO_YUV_RAW_SIZE;
                        return 0;
                }
                int getDepthBufferSize(){
                        if(m_depth_format == FREENECT_DEPTH_11BIT) return FREENECT_DEPTH_11BIT_SIZE;
                        if(m_depth_format == FREENECT_DEPTH_10BIT) return FREENECT_DEPTH_11BIT_SIZE;
                        if(m_depth_format == FREENECT_DEPTH_11BIT_PACKED) return FREENECT_DEPTH_11BIT_PACKED_SIZE;
                        if(m_depth_format == FREENECT_DEPTH_10BIT_PACKED) return FREENECT_DEPTH_10BIT_PACKED_SIZE;
                        return 0;
                }*/
          private:
                freenect_device *m_dev;
                freenect_video_format m_video_format;
                freenect_depth_format m_depth_format;
                static void freenect_depth_callback(freenect_device *dev, void *depth, uint32_t timestamp) {
                        //QString msg = "depth callback";
                        //qDebug() << msg;
                        //no lock, called inside HmiFreenect processevents
                        HmiFreenectDevice* device = static_cast<HmiFreenectDevice*>(freenect_get_user(dev));
                        device->DepthCallback(depth, timestamp);
                }
                static void freenect_video_callback(freenect_device *dev, void *video, uint32_t timestamp) {
                        //QString msg = "video callback";
                        //qDebug() << msg;
                        //no lock, called inside HmiFreenect processevents
                        HmiFreenectDevice* device = static_cast<HmiFreenectDevice*>(freenect_get_user(dev));
                        device->VideoCallback(video, timestamp);
                }
                static void freenect_log_callback(freenect_context *ctx, freenect_loglevel level, const char* msgbuf)
                {
                    qDebug() << "Message from freenect:";
                    if (level==FREENECT_LOG_ERROR) qWarning() << msgbuf;
                    else if (level==FREENECT_LOG_WARNING) qWarning() << msgbuf;
                    else qDebug() << msgbuf;
                }
                bool m_new_video_frame;
                bool m_new_depth_frame;
                cv::Mat depthMat;
                cv::Mat videoMat;
                mutable QMutex m_video_mutex;
                mutable QMutex m_depth_mutex;
        };

        class HmiFreenect : Noncopyable {

          private:
                typedef std::map<int, HmiFreenectDevice*> DeviceMap;
          public:
                HmiFreenect();
                virtual ~HmiFreenect();
                template <typename ConcreteDevice>
                ConcreteDevice& createDevice(int _index) {
                        QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
                        DeviceMap::iterator it = m_devices.find(_index);
                        if (it != m_devices.end()) delete it->second;
                        ConcreteDevice * device = new ConcreteDevice(m_ctx, _index);
                        m_devices.insert(std::make_pair<int, HmiFreenectDevice*>(_index, device));
                        return *device;
                }
                void deleteDevice(int _index);
                int deviceCount();

          public:
                freenect_context *m_ctx;
          private:
                DeviceMap m_devices;
        };


}


#endif // HMIFREENECT_H
