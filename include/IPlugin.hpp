#ifndef __IPLUGIN_HPP__
#define __IPLUGIN_HPP__

#include <QtCore/QObject>

/**
 * @brief 插件接口类
 */
class IPlugin {
public:
    virtual ~IPlugin() {}

    virtual bool Init() = 0;
};

/**
 * 使用 Q_DECLARE_INTERFACE() 宏向 Qt 的元对象系统声明该接口
 * 声明一个独一无二的 IID
 */
#define IPlugin_IID "interface.IPlugin"
Q_DECLARE_INTERFACE(IPlugin, IPlugin_IID)

#endif
