#include "plugin_system_info.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>

#include "ui/system_info_widget.h"

PluginSystemInfo::PluginSystemInfo(QWidget* parent)
    : QWidget(parent)
    , m_eventBus(nullptr)
    , m_systemInfoWidget(nullptr)
{
    createLayout();
}

PluginSystemInfo::~PluginSystemInfo() = default;

bool PluginSystemInfo::Init()
{
    m_eventBus = GetEventBus();
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen != nullptr)
    {
        const QRect screenGeometry = screen->availableGeometry();
        resize(screenGeometry.width() / 2, screenGeometry.height() / 2);
    }
    setWindowTitle(tr("Windows 系统信息"));
    refreshSnapshot();
    return true;
}

bool PluginSystemInfo::InitAppFinish()
{
    publishLog("系统信息插件初始化完成");
    return true;
}

bool PluginSystemInfo::Release()
{
    delete this;
    return true;
}

std::string PluginSystemInfo::Version()
{
    return "0.0.1";
}

std::string PluginSystemInfo::Name()
{
    return "PluginSystemInfo";
}

std::string PluginSystemInfo::Description()
{
    return "获取 Windows 7/10/11 系统信息";
}

std::string PluginSystemInfo::Icon()
{
    return "";
}

PluginLocation PluginSystemInfo::Location()
{
    return PluginLocation(PluginType::WIDGET, "系统信息", "系统", "工具");
}

void PluginSystemInfo::WidgetShow()
{
    show();
    raise();
    activateWindow();
}

void PluginSystemInfo::OnEvent(const Event* event)
{
    Q_UNUSED(event);
}

void PluginSystemInfo::refreshSnapshot()
{
    const core::SystemInfoSnapshot snapshot = m_systemInfoService.collectSnapshot();
    m_systemInfoWidget->displaySnapshot(snapshot);
    publishLog("系统信息刷新完成");
}

void PluginSystemInfo::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    m_systemInfoWidget = new SystemInfoWidget(this);
    rootLayout->addWidget(m_systemInfoWidget);
    connect(m_systemInfoWidget, &SystemInfoWidget::refreshRequested, this, &PluginSystemInfo::refreshSnapshot);
}

void PluginSystemInfo::publishLog(const std::string& message) const
{
    if (m_eventBus == nullptr)
    {
        return;
    }
    m_eventBus->publish("log", new LogEvent(0, ET_LOG, "PluginSystemInfo", __FILE__, __LINE__, __FUNCTION__, message));
}

extern "C" PLUGINSYSTEMINFO_API IPlugin* CreatePlugin()
{
    return new PluginSystemInfo();
}
