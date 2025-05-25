#ifndef __PLUGINASN1VIEW_H__
#define __PLUGINASN1VIEW_H__

#include <Common/IPlugin.hpp>

#include "GlobalPluginASN1View.hpp"
#include "ui_PluginASN1View.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class PluginASN1ViewClass;
};
QT_END_NAMESPACE

/**
 * @brief 创建插件
 *		如果带有 Q_OBJECT 宏，必须把 QObject 放在第一个继承的位置
 */
class PluginASN1View_API PluginASN1View
    : public QWidget
    , public IPlugin
{
    Q_OBJECT
public:
    PluginASN1View(QWidget* parent = nullptr);
    ~PluginASN1View();

    bool Init() override;

    bool InitAppFinish() override;

    bool Release() override;

    std::string Version() override;

    std::string Description() override;

    std::string Icon() override;

    PluginLocation Location() override;

    void WidgetShow() override;

    /**
     * 内部函数
     */
    void InitWidget();

public slots:

private:
    Ui::PluginASN1ViewClass* ui;

    struct PluginASN1ViewPrivate;
    PluginASN1ViewPrivate* m_p;
};

#endif
