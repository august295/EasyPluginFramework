#include "plugin_sm2_csr_verify.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>

#include "ui/sm2_csr_verify_widget.h"

PluginSm2CsrVerify::PluginSm2CsrVerify(QWidget* parent)
    : QWidget(parent)
    , m_eventBus(nullptr)
    , m_verifyWidget(nullptr)
{
    createLayout();
}

PluginSm2CsrVerify::~PluginSm2CsrVerify() = default;

bool PluginSm2CsrVerify::Init()
{
    m_eventBus = GetEventBus();

    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen != nullptr)
    {
        const QRect screenGeometry = screen->availableGeometry();
        resize(screenGeometry.width() / 2, screenGeometry.height() / 2);
    }

    setWindowTitle(tr("SM2 CSR 验证"));
    return true;
}

bool PluginSm2CsrVerify::InitAppFinish()
{
    publishLog("SM2 CSR 验证插件初始化完成");
    return true;
}

bool PluginSm2CsrVerify::Release()
{
    delete this;
    return true;
}

std::string PluginSm2CsrVerify::Version()
{
    return "0.0.1";
}

std::string PluginSm2CsrVerify::Name()
{
    return "PluginSm2CsrVerify";
}

std::string PluginSm2CsrVerify::Description()
{
    return "基于 endecode 的 SM2 CSR 验证工具";
}

std::string PluginSm2CsrVerify::Icon()
{
    return "";
}

PluginLocation PluginSm2CsrVerify::Location()
{
    return PluginLocation(PluginType::WIDGET, "SM2 CSR 验证", "密码", "工具");
}

void PluginSm2CsrVerify::WidgetShow()
{
    show();
    raise();
    activateWindow();
}

void PluginSm2CsrVerify::OnEvent(const Event* event)
{
    Q_UNUSED(event);
}

void PluginSm2CsrVerify::verifyCsr()
{
    core::Sm2CsrVerifyRequest request = {};
    request.csrBase64 = m_verifyWidget->csrData().toStdString();
    request.userId = m_verifyWidget->userId().toStdString();

    const core::Sm2CsrVerifyResult result = m_verifyService.verify(request);
    m_verifyWidget->showResult(result);
    publishLog(result.message);
}

void PluginSm2CsrVerify::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    m_verifyWidget = new Sm2CsrVerifyWidget(this);
    rootLayout->addWidget(m_verifyWidget);
    connect(m_verifyWidget, &Sm2CsrVerifyWidget::verifyRequested, this, &PluginSm2CsrVerify::verifyCsr);
}

void PluginSm2CsrVerify::publishLog(const std::string& message) const
{
    if (m_eventBus == nullptr)
    {
        return;
    }

    m_eventBus->publish("log", new LogEvent(0, ET_LOG, "PluginSm2CsrVerify", __FILE__, __LINE__, __FUNCTION__, message));
}

extern "C" PLUGINSM2CSRVERIFY_API IPlugin* CreatePlugin()
{
    return new PluginSm2CsrVerify();
}
