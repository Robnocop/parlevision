#ifndef PLVADVANCEDTRACKINGPLUGIN_H
#define PLVADVANCEDTRACKINGPLUGIN_H

#include "plv_advancedtracking_plugin_global.h"
#include <QObject>
#include <plvcore/Plugin.h>

class PLV_ADVANCEDTRACKING_PLUGINSHARED_EXPORT PlvAdvancedTrackingPlugin: public QObject, public plv::Plugin
{
    Q_OBJECT
    Q_INTERFACES(plv::Plugin)

public:
    PlvAdvancedTrackingPlugin();
    virtual ~PlvAdvancedTrackingPlugin();
    void onLoad();
};

#endif // PLVADVANCEDTRACKINGPLUGIN_H
