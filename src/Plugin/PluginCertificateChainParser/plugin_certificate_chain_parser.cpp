#include "plugin_certificate_chain_parser.h"

#include <QFileDialog>
#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>

#include "ui/certificate_chain_parser_widget.h"

PluginCertificateChainParser::PluginCertificateChainParser(QWidget* parent)
    : QWidget(parent)
    , m_eventBus(nullptr)
    , m_widget(nullptr)
{
    createLayout();
}

PluginCertificateChainParser::~PluginCertificateChainParser() = default;

bool PluginCertificateChainParser::Init()
{
    m_eventBus = GetEventBus();

    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen != nullptr)
    {
        const QRect screenGeometry = screen->availableGeometry();
        resize(screenGeometry.width() * 2 / 3, screenGeometry.height() * 2 / 3);
    }

    setWindowTitle(tr("证书链解析"));
    return true;
}

bool PluginCertificateChainParser::InitAppFinish()
{
    publishLog("证书链解析插件初始化完成");
    return true;
}

bool PluginCertificateChainParser::Release()
{
    delete this;
    return true;
}

std::string PluginCertificateChainParser::Version()
{
    return "0.0.1";
}

std::string PluginCertificateChainParser::Name()
{
    return "PluginCertificateChainParser";
}

std::string PluginCertificateChainParser::Description()
{
    return "基于 endecode 的证书链树形图解析与校验工具";
}

std::string PluginCertificateChainParser::Icon()
{
    return "";
}

PluginLocation PluginCertificateChainParser::Location()
{
    return PluginLocation(PluginType::WIDGET, "证书链解析", "证书", "工具");
}

void PluginCertificateChainParser::WidgetShow()
{
    show();
    raise();
    activateWindow();
}

void PluginCertificateChainParser::OnEvent(const Event* event)
{
    Q_UNUSED(event);
}

void PluginCertificateChainParser::loadDirectory()
{
    const QString directoryPath = QFileDialog::getExistingDirectory(this, tr("选择证书目录"));
    if (directoryPath.isEmpty())
    {
        return;
    }

    core::CertificateAnalysisRequest request = {};
    request.mode = core::CertificateLoadMode::ReplaceFromDirectory;
    request.path = directoryPath.toStdString();
    runAnalysis(request);
}

void PluginCertificateChainParser::addCertificate()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择单张证书"),
        QString(),
        tr("证书文件 (*.pem *.cer *.crt *.der);;所有文件 (*)"));
    if (filePath.isEmpty())
    {
        return;
    }

    core::CertificateAnalysisRequest request = {};
    request.mode = core::CertificateLoadMode::AppendSingleFile;
    request.path = filePath.toStdString();
    runAnalysis(request);
}

void PluginCertificateChainParser::validateCertificate()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择待验证证书"),
        QString(),
        tr("证书文件 (*.pem *.cer *.crt *.der);;所有文件 (*)"));
    if (filePath.isEmpty())
    {
        return;
    }

    const core::CertificateValidationResult result = m_service.validateCertificate(filePath.toStdString());
    m_widget->showValidationResult(result);
    publishLog(result.message);
}

void PluginCertificateChainParser::clearAnalysis()
{
    m_service.clear();
    m_widget->clearView();
    publishLog("已清空证书链分析结果");
}

void PluginCertificateChainParser::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    m_widget = new CertificateChainParserWidget(this);
    rootLayout->addWidget(m_widget);

    connect(m_widget, &CertificateChainParserWidget::chooseDirectoryRequested, this, &PluginCertificateChainParser::loadDirectory);
    connect(m_widget, &CertificateChainParserWidget::addCertificateRequested, this, &PluginCertificateChainParser::addCertificate);
    connect(m_widget, &CertificateChainParserWidget::validateCertificateRequested, this, &PluginCertificateChainParser::validateCertificate);
    connect(m_widget, &CertificateChainParserWidget::clearRequested, this, &PluginCertificateChainParser::clearAnalysis);
}

void PluginCertificateChainParser::publishLog(const std::string& message) const
{
    if (m_eventBus == nullptr)
    {
        return;
    }

    m_eventBus->publish("log", new LogEvent(0, ET_LOG, "PluginCertificateChainParser", __FILE__, __LINE__, __FUNCTION__, message));
}

void PluginCertificateChainParser::runAnalysis(const core::CertificateAnalysisRequest& request)
{
    const core::CertificateAnalysisResult result = m_service.load(request);
    m_widget->showResult(result);
    publishLog(result.message);
}

extern "C" PLUGINCERTIFICATECHAINPARSER_API IPlugin* CreatePlugin()
{
    return new PluginCertificateChainParser();
}
