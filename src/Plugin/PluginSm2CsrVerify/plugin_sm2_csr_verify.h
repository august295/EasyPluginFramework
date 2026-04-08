#ifndef __PLUGIN_SM2_CSR_VERIFY_H__
#define __PLUGIN_SM2_CSR_VERIFY_H__

#include <Common/IPlugin.hpp>
#include <Core/EventBus/IEventBus.hpp>
#include <Core/EventBus/IEventHandler.hpp>

#include <QWidget>

#include "core/sm2_csr_verify_service.h"
#include "global_plugin_sm2_csr_verify.hpp"

class Sm2CsrVerifyWidget;

class PLUGINSM2CSRVERIFY_API PluginSm2CsrVerify
    : public QWidget
    , public IPlugin
    , public IEventHandler
{
    Q_OBJECT

public:
    explicit PluginSm2CsrVerify(QWidget* parent = nullptr);
    ~PluginSm2CsrVerify() override;

    bool           Init() override;
    bool           InitAppFinish() override;
    bool           Release() override;
    std::string    Version() override;
    std::string    Name() override;
    std::string    Description() override;
    std::string    Icon() override;
    PluginLocation Location() override;
    void           WidgetShow() override;
    void           OnEvent(const Event* event) override;

private slots:
    void verifyCsr();

private:
    void createLayout();
    void publishLog(const std::string& message) const;

private:
    IEventBus*                  m_eventBus;
    Sm2CsrVerifyWidget*         m_verifyWidget;
    core::Sm2CsrVerifyService   m_verifyService;
};

#endif
