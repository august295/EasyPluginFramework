#ifndef __PLUGIN_SYSTEM_INFO_H__
#define __PLUGIN_SYSTEM_INFO_H__

#include <Common/IPlugin.hpp>
#include <Core/EventBus/IEventBus.hpp>
#include <Core/EventBus/IEventHandler.hpp>

#include <QWidget>

#include "core/system_info_service.h"
#include "global_plugin_system_info.hpp"

class SystemInfoWidget;

/**
 * @brief Windows 系统信息插件。
 */
class PLUGINSYSTEMINFO_API PluginSystemInfo
    : public QWidget
    , public IPlugin
    , public IEventHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造插件实例。
     * @param parent 父窗口。
     */
    explicit PluginSystemInfo(QWidget* parent = nullptr);

    /**
     * @brief 析构插件实例。
     */
    ~PluginSystemInfo() override;

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

private slots:
    void refreshSnapshot();

private:
    void createLayout();
    void publishLog(const std::string& message) const;

private:
    IEventBus* m_eventBus;
    SystemInfoWidget* m_systemInfoWidget;
    core::SystemInfoService m_systemInfoService;
};

#endif
