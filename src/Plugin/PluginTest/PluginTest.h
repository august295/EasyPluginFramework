#ifndef __PLUGINTEST_H__
#define __PLUGINTEST_H__

#include <Common/IPlugin.hpp>
#include <Core/EventBus/IEventHandler.hpp>
#include <Core/EventBus/IEventBus.hpp>

#include "GlobalTest.hpp"

/**
 * @brief 创建插件
 */
class PLUGINTEST_API PluginTest 
    : public IPlugin
    , public IEventHandler
{
public:
    PluginTest();
    ~PluginTest();

    bool Init() override;

    bool InitAppFinish() override;

    bool Release() override;

    std::string Version() override;

    std::string Name() override;

    std::string Description() override;

    std::string Icon() override;

    PluginLocation Location() override;

    void WidgetShow() override;

    void OnEvent(const Event* event) override;

private:
    IEventBus* m_eventBus;
};

#endif
