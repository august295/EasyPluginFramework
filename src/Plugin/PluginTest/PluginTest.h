#ifndef __PLUGINTEST_H__
#define __PLUGINTEST_H__

#include <Common/IPlugin.hpp>

#include "GlobalTest.hpp"

/**
 * @brief 创建插件
 */
class PLUGINTEST_API PluginTest : public IPlugin {
public:
    PluginTest();
    ~PluginTest();

    bool Init() override;

    bool InitAppFinish() override;

    bool Release() override;

    std::string Version() override;

    std::string Description() override;

	std::string Icon() override;

	PluginLocation Location() override;

	void WidgetShow() override;
};

#endif
