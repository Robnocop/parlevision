#include "plvsynchronyplugin.h"
#include <QtPlugin>
#include <QtDebug>

#include "plvsynchronyprocessor.h"
#include "pixelsums.h"
#include "gray2rgb.h"

using namespace plv;

PlvSynchronyPlugin::PlvSynchronyPlugin()
{
    qDebug() << "PlvSynchronyPlugin constructor";
}

PlvSynchronyPlugin::~PlvSynchronyPlugin()
{
    qDebug() << "PlvSynchrony destructor";
}

void PlvSynchronyPlugin::onLoad()
{
    qDebug() << "PlvSynchrony onLoad";
    plvRegisterPipelineElement<PlvSynchronyProcessor>();
    plvRegisterPipelineElement<PixelSumS>();
	plvRegisterPipelineElement<Gray2RGB>();
}

Q_EXPORT_PLUGIN2(plv_synchrony_plugin, PlvSynchronyPlugin)
