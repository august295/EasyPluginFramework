#ifndef __PLUGINBASE64_H__
#define __PLUGINBASE64_H__

#include <Common/IPlugin.hpp>

#include "GlobalPluginBase64.hpp"
#include "ui_PluginBase64.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class PluginBase64Class;
};
QT_END_NAMESPACE

/**
 * @brief 创建插件
 *		如果带有 Q_OBJECT 宏，必须把 QObject 放在第一个继承的位置
 */
class PLUGINBASE64_API PluginBase64
    : public QWidget
    , public IPlugin
{
    Q_OBJECT

public:
    PluginBase64(QWidget* parent = nullptr);
    ~PluginBase64();

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
    void slot_pushButton_open_clicked();
    void slot_pushButton_img_to_base64_clicked();
    void slot_pushButton_base64_to_img_clicked();
    void slot_pushButton_clear_clicked();

private:
    Ui::PluginBase64Class* ui;

    struct PluginBase64Private;
    PluginBase64Private* m_p;
};

#endif
