#ifndef __PLUGINCERTVIEW_H__
#define __PLUGINCERTVIEW_H__

#include <Common/IPlugin.hpp>

#include "GlobalPluginCertView.hpp"
#include "ui_PluginCertView.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class PluginCertViewClass;
};
QT_END_NAMESPACE

/**
 * @brief 创建插件
 *		如果带有 Q_OBJECT 宏，必须把 QObject 放在第一个继承的位置
 */
class PLUGINCERTVIEW_API PluginCertView
    : public QWidget
    , public IPlugin
{
    Q_OBJECT

public:
    enum TabType
    {
        TAB_BASE,
        TAB_DETAIL,
    };

public:
    PluginCertView(QWidget* parent = nullptr);
    ~PluginCertView();

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
    void OpenCertDialog();
    void ShowTabBase();
    void ShowTabDetail();

public slots:

private:
    Ui::PluginCertViewClass* ui;

    struct PluginCertViewPrivate;
    PluginCertViewPrivate* m_p;
};

#endif
