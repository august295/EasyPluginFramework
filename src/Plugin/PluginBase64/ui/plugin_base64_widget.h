#ifndef __PLUGIN_BASE64_WIDGET_H__
#define __PLUGIN_BASE64_WIDGET_H__

#include <Common/IPlugin.hpp>
#include <Core/EventBus/IEventHandler.hpp>
#include <Core/EventBus/IEventBus.hpp>

#include "../GlobalPluginBase64.hpp"
#include "ui_plugin_base64_widget.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class PluginBase64WidgetClass;
};
QT_END_NAMESPACE

/**
 * @brief 创建插件
 *		如果带有 Q_OBJECT 宏，必须把 QObject 放在第一个继承的位置
 */
class PLUGINBASE64_API PluginBase64
    : public QWidget
    , public IPlugin
    , public IEventHandler
{
    Q_OBJECT

public:
    explicit PluginBase64(QWidget* parent = nullptr);
    ~PluginBase64() override;

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
    void slot_pushButton_open_clicked();
    void slot_pushButton_img_to_base64_clicked();
    void slot_pushButton_base64_to_img_clicked();
    void slot_pushButton_save_clicked();
    void slot_pushButton_clear_clicked();

private:
    void UpdatePreviewImage(const QImage& image);
    void UpdateInfoText(const QString& imageInfo, const QString& base64Info);
    void ClearData();
    QString BuildImageInfo(const QImage& image, const QByteArray& format, const QString& fileName = QString()) const;
    QString BuildBase64Info(const QByteArray& rawData, const QString& base64Text) const;
    QByteArray DetectImageFormat(const QString& fileName) const;

private:
    QImage                 m_currentImage;
    QByteArray             m_currentImageBytes;
    QByteArray             m_currentImageFormat;
    QString                m_currentFileName;
    Ui::PluginBase64WidgetClass* ui;
};

#endif
