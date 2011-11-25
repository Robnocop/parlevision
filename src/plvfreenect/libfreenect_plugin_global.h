#ifndef LIBFREENECT_PLUGIN_GLOBAL_H
#define LIBFREENECT_PLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QDebug>

#if defined(LIBFREENECT_PLUGIN_LIBRARY)
#  define LIBFREENECT_PLUGINSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBFREENECT_PLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBFREENECT_PLUGIN_GLOBAL_H
