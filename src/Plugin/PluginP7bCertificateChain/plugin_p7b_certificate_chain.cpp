#include "plugin_p7b_certificate_chain.h"

#include <algorithm>
#include <cctype>

#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>

#include "ui/p7b_certificate_chain_widget.h"

namespace
{
bool isLikelyTextContent(const QByteArray& data)
{
    if (data.isEmpty())
    {
        return false;
    }

    for (const char ch : data)
    {
        const unsigned char value = static_cast<unsigned char>(ch);
        if (value == 0U)
        {
            return false;
        }

        if (std::isprint(value) != 0 || std::isspace(value) != 0)
        {
            continue;
        }

        return false;
    }

    return true;
}
}

PluginP7bCertificateChain::PluginP7bCertificateChain(QWidget* parent)
    : QWidget(parent)
    , m_eventBus(nullptr)
    , m_chainWidget(nullptr)
{
    createLayout();
}

PluginP7bCertificateChain::~PluginP7bCertificateChain() = default;

bool PluginP7bCertificateChain::Init()
{
    m_eventBus = GetEventBus();

    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen != nullptr)
    {
        const QRect screenGeometry = screen->availableGeometry();
        resize(screenGeometry.width() * 2 / 3, screenGeometry.height() * 2 / 3);
    }

    setWindowTitle(tr("P7B 证书链解析"));
    return true;
}

bool PluginP7bCertificateChain::InitAppFinish()
{
    publishLog("P7B 证书链解析插件初始化完成");
    return true;
}

bool PluginP7bCertificateChain::Release()
{
    delete this;
    return true;
}

std::string PluginP7bCertificateChain::Version()
{
    return "0.0.1";
}

std::string PluginP7bCertificateChain::Name()
{
    return "PluginP7bCertificateChain";
}

std::string PluginP7bCertificateChain::Description()
{
    return "基于 endecode 的 P7B/PKCS7 证书链解析与导出工具";
}

std::string PluginP7bCertificateChain::Icon()
{
    return "";
}

PluginLocation PluginP7bCertificateChain::Location()
{
    return PluginLocation(PluginType::WIDGET, "P7B 证书链解析", "证书", "工具");
}

void PluginP7bCertificateChain::WidgetShow()
{
    show();
    raise();
    activateWindow();
}

void PluginP7bCertificateChain::OnEvent(const Event* event)
{
    Q_UNUSED(event);
}

void PluginP7bCertificateChain::openChainFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择证书链文件"),
        QString(),
        tr("证书链文件 (*.p7b *.p7c *.pem *.cer *.crt);;所有文件 (*)"));
    if (filePath.isEmpty())
    {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        m_chainWidget->showActionMessage(tr("文件读取失败：无法打开 %1").arg(filePath), false);
        publishLog(QString("文件读取失败：%1").arg(filePath).toStdString());
        return;
    }

    const QByteArray fileBytes = file.readAll();
    if (fileBytes.isEmpty())
    {
        m_chainWidget->showActionMessage(tr("文件读取失败：文件为空 %1").arg(filePath), false);
        publishLog(QString("文件读取失败：%1").arg(filePath).toStdString());
        return;
    }

    const bool isTextFile = isLikelyTextContent(fileBytes);
    const QString chainText = isTextFile ? QString::fromUtf8(fileBytes) : QString::fromLatin1(fileBytes.toBase64());

    m_chainWidget->setChainData(chainText);
    m_chainWidget->clearResult();
    m_chainWidget->showActionMessage(
        isTextFile ? tr("已载入文本证书链文件：%1").arg(filePath) : tr("已载入 DER 证书链文件并转换为 Base64：%1").arg(filePath),
        true);
    publishLog(
        isTextFile ? QString("已载入文本证书链文件：%1").arg(filePath).toStdString()
                   : QString("已载入 DER 证书链文件并转换为 Base64：%1").arg(filePath).toStdString());
}

void PluginP7bCertificateChain::parseChain()
{
    core::P7bCertificateChainRequest request = {};
    request.chainData = m_chainWidget->chainData().toStdString();

    m_lastResult = m_chainService.parse(request);
    m_chainWidget->showResult(m_lastResult);
    publishLog(m_lastResult.message);
}

void PluginP7bCertificateChain::exportSelectedCertificate()
{
    if (!m_lastResult.success)
    {
        m_chainWidget->showActionMessage(tr("请先成功解析证书链后再导出"), false);
        return;
    }

    const int row = m_chainWidget->selectedCertificateIndex();
    if (row < 0 || static_cast<std::size_t>(row) >= m_lastResult.certificates.size())
    {
        m_chainWidget->showActionMessage(tr("请先选择一张证书再导出"), false);
        return;
    }

    const core::P7bCertificateSummary& certificate = m_lastResult.certificates[static_cast<std::size_t>(row)];
    const bool exported = exportPemToFile(tr("导出选中证书"), QStringLiteral("certificate_%1.pem").arg(row + 1), certificate.pem);
    if (exported)
    {
        m_chainWidget->showActionMessage(tr("选中证书导出成功"), true);
        publishLog("选中证书导出成功");
    }
}

void PluginP7bCertificateChain::exportAllCertificates()
{
    if (!m_lastResult.success || m_lastResult.fullChainPem.empty())
    {
        m_chainWidget->showActionMessage(tr("请先成功解析证书链后再导出"), false);
        return;
    }

    const bool exported = exportPemToFile(tr("导出全部证书"), QStringLiteral("full_chain.pem"), m_lastResult.fullChainPem);
    if (exported)
    {
        m_chainWidget->showActionMessage(tr("完整 PEM 链导出成功"), true);
        publishLog("完整 PEM 链导出成功");
    }
}

bool PluginP7bCertificateChain::exportPemToFile(const QString& title, const QString& defaultFileName, const std::string& pemText)
{
    const QString filePath = QFileDialog::getSaveFileName(this, title, defaultFileName, tr("PEM 文件 (*.pem);;所有文件 (*)"));
    if (filePath.isEmpty())
    {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        m_chainWidget->showActionMessage(tr("导出失败：无法写入文件 %1").arg(filePath), false);
        publishLog(QString("导出失败：%1").arg(filePath).toStdString());
        return false;
    }

    const QByteArray data = QByteArray::fromStdString(pemText);
    if (file.write(data) != data.size())
    {
        m_chainWidget->showActionMessage(tr("导出失败：文件写入不完整 %1").arg(filePath), false);
        publishLog(QString("导出失败：%1").arg(filePath).toStdString());
        return false;
    }

    return true;
}

void PluginP7bCertificateChain::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    m_chainWidget = new P7bCertificateChainWidget(this);
    rootLayout->addWidget(m_chainWidget);

    connect(m_chainWidget, &P7bCertificateChainWidget::openFileRequested, this, &PluginP7bCertificateChain::openChainFile);
    connect(m_chainWidget, &P7bCertificateChainWidget::parseRequested, this, &PluginP7bCertificateChain::parseChain);
    connect(m_chainWidget, &P7bCertificateChainWidget::exportSelectedRequested, this, &PluginP7bCertificateChain::exportSelectedCertificate);
    connect(m_chainWidget, &P7bCertificateChainWidget::exportAllRequested, this, &PluginP7bCertificateChain::exportAllCertificates);
}

void PluginP7bCertificateChain::publishLog(const std::string& message) const
{
    if (m_eventBus == nullptr)
    {
        return;
    }

    m_eventBus->publish("log", new LogEvent(0, ET_LOG, "PluginP7bCertificateChain", __FILE__, __LINE__, __FUNCTION__, message));
}

extern "C" PLUGINP7BCERTIFICATECHAIN_API IPlugin* CreatePlugin()
{
    return new PluginP7bCertificateChain();
}
