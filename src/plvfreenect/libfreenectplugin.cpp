#include "libfreenectplugin.h"
#include <QtPlugin>
#include <QtDebug>

#include "freenectproducer.h"

using namespace plv;
using namespace plvfreenect;

/*
todo: reroute log messages to parlevision log;  log_cb(ctx, level, msgbuf); #define LL_FATAL FREENECT_LOG_FATAL
#define LL_ERROR FREENECT_LOG_ERROR
#define LL_WARNING FREENECT_LOG_WARNING
#define LL_NOTICE FREENECT_LOG_NOTICE
#define LL_INFO FREENECT_LOG_INFO
#define LL_DEBUG FREENECT_LOG_DEBUG
#define LL_SPEW FREENECT_LOG_SPEW
#define LL_FLOOD FREENECT_LOG_FLOOD

use freenect_get_video_mode_count and freenect_get_video_mode to fill a list of options for producer (namelu: modes that can be set)

maybe do something wiht usb context to allow multiple devices;

reset depth and video mat cahce whenever new resolution or depth set...

TIP: Perform stream operations only in the thread that calls freenect_process_events()

*/

LibFreenectPlugin::LibFreenectPlugin()
{
    qDebug() << "LibFreenectPlugin constructor";
}

LibFreenectPlugin::~LibFreenectPlugin()
{
    qDebug() << "LibFreenectPlugin destructor";
}

void LibFreenectPlugin::onLoad()
{
    qDebug() << "LibFreenectPlugin onLoad";
    plvRegisterPipelineElement<plvfreenect::FreenectProducer>();

}


Q_EXPORT_PLUGIN2(libfreenect_plugin, LibFreenectPlugin)
