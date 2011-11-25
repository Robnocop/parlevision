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

#include "hmifreenect.h"

using namespace plvfreenect;

    QMutex KinectMutex::m_hmifreenectMutex;

    HmiFreenectDevice::HmiFreenectDevice(freenect_context *_ctx, int _index) :
        m_new_video_frame(false),
        m_new_depth_frame(false),
        depthMat(cv::Size(640,480),CV_16UC1),
        videoMat(cv::Size(640,480),CV_8UC3,cv::Scalar(0))

    {
            //no lock, called inside HmiFreenect createdevice
            if(freenect_open_device(_ctx, &m_dev, _index) < 0) throw std::runtime_error("Cannot open Kinect");
            freenect_set_user(m_dev, this);
            freenect_set_video_mode(m_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
            freenect_set_depth_mode(m_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
            freenect_set_depth_callback(m_dev, freenect_depth_callback);
            freenect_set_video_callback(m_dev, freenect_video_callback);
            //freenect_set_log_callback(_ctx,freenect_log_callback);
    }
    HmiFreenectDevice::~HmiFreenectDevice() {
            //no lock, called inside HmiFreenect deletedevice
            if(freenect_close_device(m_dev) < 0){} //FN_WARNING("Device did not shutdown in a clean fashion");
    }

    void HmiFreenectDevice::startVideo() {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            if(freenect_start_video(m_dev) < 0) throw std::runtime_error("Cannot start video callback");
    }
    void HmiFreenectDevice::stopVideo() {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            if(freenect_stop_video(m_dev) < 0) throw std::runtime_error("Cannot stop video callback");
    }
    void HmiFreenectDevice::startDepth() {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            if(freenect_start_depth(m_dev) < 0) throw std::runtime_error("Cannot start depth callback");
    }
    void HmiFreenectDevice::stopDepth() {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            if(freenect_stop_depth(m_dev) < 0) throw std::runtime_error("Cannot stop depth callback");
    }
    void HmiFreenectDevice::setTiltDegrees(double _angle) {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            if(freenect_set_tilt_degs(m_dev, _angle) < 0) throw std::runtime_error("Cannot set angle in degrees");
    }
    void HmiFreenectDevice::setLed(freenect_led_options _option) {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            if(freenect_set_led(m_dev, _option) < 0) throw std::runtime_error("Cannot set led");
    }
    void HmiFreenectDevice::updateState() {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            if (freenect_update_tilt_state(m_dev) < 0) throw std::runtime_error("Cannot update device state");
    }
    FreenectTiltState HmiFreenectDevice::getState() {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            return FreenectTiltState(freenect_get_tilt_state(m_dev));
    }
    void HmiFreenectDevice::setVideoFormat(freenect_video_format requested_format) {
            if (requested_format != m_video_format) {
                    QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
                    freenect_stop_video(m_dev);                   
                    freenect_frame_mode mode = freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, requested_format);
                    if (!mode.is_valid) throw std::runtime_error("Cannot set video format: invalid mode");
                    if (freenect_set_video_mode(m_dev, mode) < 0) throw std::runtime_error("Cannot set video format");
                    freenect_start_video(m_dev);
                    m_video_format = requested_format;
            }
    }
    freenect_video_format HmiFreenectDevice::getVideoFormat() {
            return m_video_format;
    }
    void HmiFreenectDevice::setDepthFormat(freenect_depth_format requested_format) {
            if (requested_format != m_depth_format) {
                    QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
                    freenect_stop_depth(m_dev);
                    freenect_frame_mode mode = freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, requested_format);
                    if (!mode.is_valid) throw std::runtime_error("Cannot set depth format: invalid mode");
                    if (freenect_set_depth_mode(m_dev, mode) < 0) throw std::runtime_error("Cannot set depth format");
                    freenect_start_depth(m_dev);
                    m_depth_format = requested_format;
            }
    }
    freenect_depth_format HmiFreenectDevice::getDepthFormat() {
            return m_depth_format;
    }
    // Do not call directly even in child
    void HmiFreenectDevice::VideoCallback(void* _video, uint32_t timestamp) {
            QMutexLocker lock(&m_video_mutex);
            uint8_t* video = static_cast<uint8_t*>(_video);
            videoMat.data = video;
            m_new_video_frame = true;
    }
    // Do not call directly even in child
    void HmiFreenectDevice::DepthCallback(void* _depth, uint32_t timestamp) {
            QMutexLocker lock(&m_depth_mutex);
            uint16_t* depth = static_cast<uint16_t*>(_depth);
            depthMat.data = (uchar*) depth;
            m_new_depth_frame = true;
    }
    bool HmiFreenectDevice::getVideo(cv::Mat& output) {
            QMutexLocker lock(&m_video_mutex);
            if(m_new_video_frame) {
                    //cv::cvtColor(videoMat, output, CV_RGB2BGR);
                    videoMat.copyTo(output);
                    m_new_video_frame = false;
                    return true;
            }
            return false;
    }
    bool HmiFreenectDevice::newVideoAvailable()
    {
        QMutexLocker lock(&m_video_mutex);
        return m_new_video_frame;
    }

    bool HmiFreenectDevice::getDepth(cv::Mat& output)
    {
        QMutexLocker lock(&m_depth_mutex);
        if(m_new_depth_frame) {
                depthMat.copyTo(output);
                m_new_depth_frame = false;
                return true;
        }
        return false;
    }
    bool HmiFreenectDevice::newDepthAvailable()
    {
        QMutexLocker lock(&m_depth_mutex);
        return m_new_depth_frame;
    }


    HmiFreenect::HmiFreenect() {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            if(freenect_init(&m_ctx, NULL) < 0) throw std::runtime_error("Cannot initialize freenect library");
    }
    HmiFreenect::~HmiFreenect() {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            for(DeviceMap::iterator it = m_devices.begin() ; it != m_devices.end() ; ++it) {
                    delete it->second;
            }
            if(freenect_shutdown(m_ctx) < 0){} //FN_WARNING("Freenect did not shutdown in a clean fashion");
    }

    void HmiFreenect::deleteDevice(int _index) {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            DeviceMap::iterator it = m_devices.find(_index);
            if (it == m_devices.end()) return;
            delete it->second;
            m_devices.erase(it);
    }
    int HmiFreenect::deviceCount() {
            QMutexLocker lock(&(KinectMutex::m_hmifreenectMutex));
            return freenect_num_devices(m_ctx);
    }


