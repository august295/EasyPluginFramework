#include "plugin_sm2_pkcs7_verify.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>

#include "ui/sm2_pkcs7_verify_widget.h"

PluginSm2Pkcs7Verify::PluginSm2Pkcs7Verify(QWidget* parent)
    : QWidget(parent)
    , m_eventBus(nullptr)
    , m_verifyWidget(nullptr)
{
    createLayout();
}

PluginSm2Pkcs7Verify::~PluginSm2Pkcs7Verify() = default;

bool PluginSm2Pkcs7Verify::Init()
{
    m_eventBus = GetEventBus();

    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen != nullptr)
    {
        const QRect screenGeometry = screen->availableGeometry();
        resize(screenGeometry.width() / 2, screenGeometry.height() / 2);
    }

    setWindowTitle(tr("SM2 PKCS7 验签"));
    return true;
}

bool PluginSm2Pkcs7Verify::InitAppFinish()
{
    publishLog("SM2 PKCS7 验签插件初始化完成");
    return true;
}

bool PluginSm2Pkcs7Verify::Release()
{
    delete this;
    return true;
}

std::string PluginSm2Pkcs7Verify::Version()
{
    return "0.0.1";
}

std::string PluginSm2Pkcs7Verify::Name()
{
    return "PluginSm2Pkcs7Verify";
}

std::string PluginSm2Pkcs7Verify::Description()
{
    return "基于 endecode 的 SM2 PKCS7 验签工具";
}

std::string PluginSm2Pkcs7Verify::Icon()
{
    return "";
}

PluginLocation PluginSm2Pkcs7Verify::Location()
{
    return PluginLocation(PluginType::WIDGET, "SM2 PKCS7 验签", "密码", "工具");
}

void PluginSm2Pkcs7Verify::WidgetShow()
{
    show();
    raise();
    activateWindow();
}

void PluginSm2Pkcs7Verify::OnEvent(const Event* event)
{
    Q_UNUSED(event);
}

void PluginSm2Pkcs7Verify::verifySignature()
{
    core::Sm2Pkcs7VerifyRequest request = {};
    request.signatureBase64 = m_verifyWidget->signatureData().toStdString();
    request.originalData = m_verifyWidget->originalData().toStdString();
    request.userId = m_verifyWidget->userId().toStdString();
    request.hasEmbeddedOriginal = m_verifyWidget->hasEmbeddedOriginal();

    const core::Sm2Pkcs7VerifyResult result = m_verifyService.verify(request);
    m_verifyWidget->showResult(result);
    publishLog(result.message);
}

void PluginSm2Pkcs7Verify::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    m_verifyWidget = new Sm2Pkcs7VerifyWidget(this);
    rootLayout->addWidget(m_verifyWidget);
    connect(m_verifyWidget, &Sm2Pkcs7VerifyWidget::verifyRequested, this, &PluginSm2Pkcs7Verify::verifySignature);
}

void PluginSm2Pkcs7Verify::publishLog(const std::string& message) const
{
    if (m_eventBus == nullptr)
    {
        return;
    }

    m_eventBus->publish("log", new LogEvent(0, ET_LOG, "PluginSm2Pkcs7Verify", __FILE__, __LINE__, __FUNCTION__, message));
}

extern "C" PLUGINSM2PKCS7VERIFY_API IPlugin* CreatePlugin()
{
    return new PluginSm2Pkcs7Verify();
}
