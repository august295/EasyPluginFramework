#ifndef __IPLUGIN_HPP__
#define __IPLUGIN_HPP__

#include <QtCore/QObject>

class IPlugin {
public:
    virtual ~IPlugin() {}

    virtual bool Init() = 0;
};

#define IPlugin_IID "interface.IPlugin"
Q_DECLARE_INTERFACE(IPlugin, IPlugin_IID)

#endif
