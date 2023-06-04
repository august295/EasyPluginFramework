#ifndef __PLUGINTEST_H__
#define __PLUGINTEST_H__

#include <IPlugin.hpp>

#include "GlobalTest.hpp"

/**
 * @brief 创建插件
 */
class PLUGINTEST_API PluginTest : public IPlugin {
public:
    PluginTest();
    ~PluginTest();

    std::string Version() override;

    std::string Description() override;

    bool Init() override;

	bool InitAppFinish() override;

	bool Release() override;
};

#endif
