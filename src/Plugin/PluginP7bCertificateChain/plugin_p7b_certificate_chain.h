#ifndef __PLUGIN_P7B_CERTIFICATE_CHAIN_H__
#define __PLUGIN_P7B_CERTIFICATE_CHAIN_H__

#include <Common/IPlugin.hpp>
#include <Core/EventBus/IEventBus.hpp>
#include <Core/EventBus/IEventHandler.hpp>

#include <QWidget>

#include "core/p7b_certificate_chain_service.h"
#include "global_plugin_p7b_certificate_chain.hpp"

class P7bCertificateChainWidget;

class PLUGINP7BCERTIFICATECHAIN_API PluginP7bCertificateChain
    : public QWidget
    , public IPlugin
    , public IEventHandler
{
    Q_OBJECT

public:
    explicit PluginP7bCertificateChain(QWidget* parent = nullptr);
    ~PluginP7bCertificateChain() override;

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
    void openChainFile();
    void parseChain();
    void exportSelectedCertificate();
    void exportAllCertificates();

private:
    bool exportPemToFile(const QString& title, const QString& defaultFileName, const std::string& pemText);
    void createLayout();
    void publishLog(const std::string& message) const;

private:
    IEventBus* m_eventBus;
    P7bCertificateChainWidget* m_chainWidget;
    core::P7bCertificateChainService m_chainService;
    core::P7bCertificateChainResult m_lastResult;
};

#endif
