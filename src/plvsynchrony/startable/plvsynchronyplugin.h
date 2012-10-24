#ifndef PLVSYNCHRONYPLUGIN_H
#define PLVSYNCHRONYPLUGIN_H

#include "plv_synchrony_plugin_global.h"
//#include "plv_synchrony_plugin_global.h"
#include <QObject>
#include <plvcore/Plugin.h>

class PLV_SYNCHRONY_PLUGINSHARED_EXPORT PlvSynchronyPlugin : public QObject, public plv::Plugin
{
    Q_OBJECT
    Q_INTERFACES(plv::Plugin)

public:
    PlvSynchronyPlugin();
    virtual ~PlvSynchronyPlugin();
    void onLoad();
};

#endif // HELLOWORLDPLUGIN_H
