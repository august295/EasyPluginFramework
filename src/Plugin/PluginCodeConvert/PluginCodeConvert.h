#ifndef __PluginCodeConvert_H__
#define __PluginCodeConvert_H__

#include <Common/IPlugin.hpp>
#include <Core/EventBus/IEventHandler.hpp>
#include <Core/EventBus/IEventBus.hpp>

#include "GlobalCodeConvert.hpp"
#include "ui_PluginCodeConvert.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class PluginCodeConvertClass;
};
QT_END_NAMESPACE

/**
 * @brief 创建插件
 *		如果带有 Q_OBJECT 宏，必须把 QObject 放在第一个继承的位置
 */
class PLUGINCODECONVERT_API PluginCodeConvert
    : public QWidget
    , public IPlugin
    , public IEventHandler
{
    Q_OBJECT

public:
    PluginCodeConvert(QWidget* parent = nullptr);
    ~PluginCodeConvert();

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

private:
    IEventBus* m_eventBus;

private:
    /**
     * 内部函数
     */
    void InitWidget();

public slots:
    void slot_pushButton_add();
    void slot_pushButton_delete();
    void slot_pushButton_convert();

private:
    Ui::PluginCodeConvertClass* ui;

    struct PluginCodeConvertPrivate;
    PluginCodeConvertPrivate* m_p;
};

#endif
