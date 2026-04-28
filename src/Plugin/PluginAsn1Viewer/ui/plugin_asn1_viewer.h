#ifndef __PLUGIN_ASN1_VIEWER_H__
#define __PLUGIN_ASN1_VIEWER_H__

#include <Common/IPlugin.hpp>
#include <Core/EventBus/IEventBus.hpp>
#include <Core/EventBus/IEventHandler.hpp>

#include <QWidget>

#include <optional>

#include "../GlobalPluginAsn1Viewer.hpp"
#include "../core/asn1_parser_service.h"
#include "hex_view_widget.h"
#include "ui_plugin_asn1_viewer.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class PluginAsn1ViewerClass;
}
QT_END_NAMESPACE

/**
 * @brief ASN.1 解析展示插件。
 */
class PLUGINASN1VIEWER_API PluginAsn1Viewer
    : public QWidget
    , public IPlugin
    , public IEventHandler
{
    Q_OBJECT

public:
    explicit PluginAsn1Viewer(QWidget* parent = nullptr);
    ~PluginAsn1Viewer() override;
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
    void slotPushButtonOpenClicked();
    void slotPushButtonParseInputClicked();
    void slotTreeItemSelectionChanged();

private:
    void initWidget();
    void setupDetailPane();
    void loadDocument(const std::optional<core::Asn1Document>& document, const QString& successLogMessage);
    void showDocument(const core::Asn1Document& document);
    void appendNode(const core::Asn1NodeInfo& nodeInfo, QTreeWidgetItem* parentItem);
    void clearResult();
    void showHexDump();
    void highlightRange(std::size_t offset, std::size_t encodedLength, std::size_t valueLength);
    void publishLog(const std::string& message) const;

private:
    IEventBus*                        m_eventBus;
    core::Asn1ParserService           m_parserService;
    std::optional<core::Asn1Document> m_currentDocument;
    HexViewWidget*                    m_hexView;
    Ui::PluginAsn1ViewerClass*        ui;
};

#endif
