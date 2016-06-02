#include "plvadvancedtrackingplugin.h"
#include <QtPlugin>
#include <QtDebug>

#include "plvadvancedtrackingprocessor.h"
#include "PlvAdvancedTrackingSum.h"
//#include "gray2rgb.h"

using namespace plv;

PlvAdvancedTrackingPlugin::PlvAdvancedTrackingPlugin()
{
    qDebug() << "PlvAdvancedTracking constructor";
}

PlvAdvancedTrackingPlugin::~PlvAdvancedTrackingPlugin()
{
    qDebug() << "PlvAdvancedTracking destructor";
}

void PlvAdvancedTrackingPlugin::onLoad()
{
    qDebug() << "PlvAdvancedTracking onLoad";
    plvRegisterPipelineElement<PlvAdvancedTrackingProcessor>();
	plvRegisterPipelineElement<PlvAdvancedTrackingSum>();
    //plvRegisterPipelineElement<Gray2RGB>();
}

Q_EXPORT_PLUGIN2(plv_advancedtracking_plugin, PlvAdvancedTrackingPlugin)
