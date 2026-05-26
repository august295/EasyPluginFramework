#ifndef __PLUGIN_CERTIFICATE_CHAIN_PARSER_H__
#define __PLUGIN_CERTIFICATE_CHAIN_PARSER_H__

#include <Common/IPlugin.hpp>
#include <Core/EventBus/IEventBus.hpp>
#include <Core/EventBus/IEventHandler.hpp>

#include <QWidget>

#include "core/certificate_chain_service.h"
#include "global_plugin_certificate_chain_parser.hpp"

class CertificateChainParserWidget;

class PLUGINCERTIFICATECHAINPARSER_API PluginCertificateChainParser
    : public QWidget
    , public IPlugin
    , public IEventHandler
{
    Q_OBJECT

public:
    explicit PluginCertificateChainParser(QWidget* parent = nullptr);
    ~PluginCertificateChainParser() override;

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
    void loadDirectory();
    void addCertificate();
    void validateCertificate();
    void clearAnalysis();

private:
    void createLayout();
    void publishLog(const std::string& message) const;
    void runAnalysis(const core::CertificateAnalysisRequest& request);

private:
    IEventBus* m_eventBus;
    CertificateChainParserWidget* m_widget;
    core::CertificateChainService m_service;
};

#endif
