#include "mskinectplugin.h"
#include <QtPlugin>
#include <QtDebug>

#include "mskinectproducer.h"
#include "mskinectdatatypes.h"
#include "mskinectfakecolor.h"
#include "skeletondataviewer.h"
#include "kinectthreshold.h"
#include "tokeystrokes.h"
#include <plvgui/RendererFactory.h>

using namespace plv;
using namespace plvmskinect;

//const char* Test::getDataTypeName() const
//{
//    return QMetaType::typeName( qMetaTypeId<plvmskinect::SkeletonFrame>() );
//}

//plvgui::DataRenderer* Test::create(QWidget* parent) const
//{
//    plvgui::DataRenderer* r = static_cast<plvgui::DataRenderer*>(new plvmskinect::SkeletonDataViewer(parent));
//    r->setParent( parent );
//    return r;
//}

MSKinectPlugin::MSKinectPlugin()
{
    qDebug() << "MSKinectPlugin constructor";
}

MSKinectPlugin::~MSKinectPlugin()
{
    qDebug() << "MSKinectPlugin destructor";
}

void MSKinectPlugin::onLoad()
{
    qDebug() << "MSKinectPlugin onLoad";

	//replaced plvmskinect::SkeletonFrame with  NUI_SKELETON_FRAME 
    qRegisterMetaType< NUI_SKELETON_FRAME >( "NUI_SKELETON_FRAME" );
	//qRegisterMetaType< plvmskinect::SkeletonFrame >( "plvmskinect::SkeletonFrame" );

    plvRegisterPipelineElement<plvmskinect::MSKinectProducer>();
    plvRegisterPipelineElement<plvmskinect::KinectThreshold>();
	plvRegisterPipelineElement<plvmskinect::MSKinectFakeColor>();
	plvRegisterPipelineElement<plvmskinect::ToKeyStrokes>();

    // register renderers
    //replaced plvmskinect::SkeletonFrame with NUI_SKELETON_FRAME
	//plvgui::RendererFactory::add<plvmskinect::SkeletonFrame, plvmskinect::SkeletonDataViewer>();
	plvgui::RendererFactory::add<NUI_SKELETON_FRAME, plvmskinect::SkeletonDataViewer>();

    //plvgui::RendererFactory::instance()->add( new Test() );
}

Q_EXPORT_PLUGIN2(mskinect_plugin, MSKinectPlugin)
