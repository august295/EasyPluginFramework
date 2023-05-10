#ifndef __TEST_H__
#define __TEST_H__

#include <IPlugin.hpp>

#include "GlobalTest.hpp"

/**
 * @brief 创建插件，继承自 QObject 和 IPlugin
 * 使用 Q_INTERFACES() 宏告诉 Qt 的元对象系统有关接口的信息
 * 使用 Q_PLUGIN_METADATA() 宏导出插件
 */
class TEST_API Test : public QObject, public IPlugin {
    Q_OBJECT
    Q_INTERFACES(IPlugin)
    Q_PLUGIN_METADATA(IID IPlugin_IID)

public:
    Test();
    ~Test();

    bool Init() override;
};

#endif
