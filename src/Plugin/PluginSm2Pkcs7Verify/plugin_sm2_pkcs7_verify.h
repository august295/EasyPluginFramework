#ifndef __PLUGIN_SM2_PKCS7_VERIFY_H__
#define __PLUGIN_SM2_PKCS7_VERIFY_H__

#include <Common/IPlugin.hpp>
#include <Core/EventBus/IEventBus.hpp>
#include <Core/EventBus/IEventHandler.hpp>

#include <QWidget>

#include "core/sm2_pkcs7_verify_service.h"
#include "global_plugin_sm2_pkcs7_verify.hpp"

class Sm2Pkcs7VerifyWidget;

class PLUGINSM2PKCS7VERIFY_API PluginSm2Pkcs7Verify
    : public QWidget
    , public IPlugin
    , public IEventHandler
{
    Q_OBJECT

public:
    explicit PluginSm2Pkcs7Verify(QWidget* parent = nullptr);
    ~PluginSm2Pkcs7Verify() override;

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
    void verifySignature();

private:
    void createLayout();
    void publishLog(const std::string& message) const;

private:
    IEventBus*                    m_eventBus;
    Sm2Pkcs7VerifyWidget*         m_verifyWidget;
    core::Sm2Pkcs7VerifyService   m_verifyService;
};

#endif
