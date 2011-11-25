#ifndef LIBFREENECTPLUGIN_H
#define LIBFREENECTPLUGIN_H

#include "libfreenect_plugin_global.h"
#include <QObject>
#include <plvcore/Plugin.h>

class LIBFREENECT_PLUGINSHARED_EXPORT LibFreenectPlugin : public QObject, public plv::Plugin
{
    Q_OBJECT
    Q_INTERFACES(plv::Plugin)

public:
    LibFreenectPlugin();
    virtual ~LibFreenectPlugin();
    void onLoad();
};

#endif // LIBFREENECTPLUGIN_H
