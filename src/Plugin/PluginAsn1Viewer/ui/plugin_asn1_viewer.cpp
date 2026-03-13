#include "plugin_asn1_viewer.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtGui/QScreen>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStyle>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

namespace
{
    QString toQString(const std::string& text)
    {
        return QString::fromUtf8(text.c_str());
    }

    QString buildNodeText(const core::Asn1NodeInfo& nodeInfo)
    {
        const QString prefix = QString("(O:%1, L:%2) %3")
                                   .arg(nodeInfo.offset)
                                   .arg(nodeInfo.length)
                                   .arg(toQString(nodeInfo.tagName));
        if (nodeInfo.children.empty() && !nodeInfo.displayValue.empty())
        {
            QString info = toQString(nodeInfo.displayValue);
            info.replace('\n', ' ');
            return prefix + ": " + info;
        }
        return prefix;
    }

    QIcon buildNodeIcon(const core::Asn1NodeInfo& nodeInfo)
    {
        // TODO(lijun): 根据 ASN.1 节点类型恢复差异化图标。
        Q_UNUSED(nodeInfo);

        QStyle* style = QApplication::style();
        return style != nullptr ? style->standardIcon(QStyle::SP_FileIcon) : QIcon();
    }
} // namespace

PluginAsn1Viewer::PluginAsn1Viewer(QWidget* parent)
    : QWidget(parent)
    , m_eventBus(nullptr)
    , m_parserService()
    , m_currentDocument(std::nullopt)
    , m_hexView(nullptr)
    , ui(new Ui::PluginAsn1ViewerClass())
{
    ui->setupUi(this);
}

PluginAsn1Viewer::~PluginAsn1Viewer()
{
}

bool PluginAsn1Viewer::Init()
{
    m_eventBus      = GetEventBus();
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen != nullptr)
    {
        const QRect screenGeometry = screen->geometry();
        resize(screenGeometry.width() * 2 / 3, screenGeometry.height() * 2 / 3);
    }
    setWindowTitle(tr("ASN1 解析展示"));
    setWindowIcon(QIcon());
    initWidget();
    return true;
}

bool PluginAsn1Viewer::InitAppFinish()
{
    publishLog("ASN1 解析插件初始化完成");
    return true;
}

bool PluginAsn1Viewer::Release()
{
    delete this;
    return true;
}

std::string PluginAsn1Viewer::Version()
{
    return "0.0.1";
}

std::string PluginAsn1Viewer::Name()
{
    return "PluginAsn1Viewer";
}

std::string PluginAsn1Viewer::Description()
{
    return "使用 endecode 解析和展示 ASN1 数据";
}

std::string PluginAsn1Viewer::Icon()
{
    return "";
}

PluginLocation PluginAsn1Viewer::Location()
{
    return PluginLocation(PluginType::WIDGET, "ASN1 解析", "证书", "工具");
}

void PluginAsn1Viewer::WidgetShow()
{
    show();
    raise();
    activateWindow();
}

void PluginAsn1Viewer::OnEvent(const Event* event)
{
    Q_UNUSED(event);
}

void PluginAsn1Viewer::slotPushButtonOpenClicked()
{
    const QString filePath = QFileDialog::getOpenFileName(this, tr("选择 ASN1 文件"), "", tr("ASN1 文件 (*.der *.ber *.cer *.crt *.pem);;所有文件 (*)"));
    if (filePath.isEmpty())
    {
        return;
    }

    const auto document = m_parserService.parseFile(filePath.toStdString());
    if (!document.has_value())
    {
        QMessageBox::warning(this, tr("解析失败"), toQString(m_parserService.getLastError()));
        return;
    }

    showDocument(document.value());
    publishLog(QString("解析 ASN1 文件: %1").arg(filePath).toStdString());
}

void PluginAsn1Viewer::slotTreeItemSelectionChanged()
{
    const QList<QTreeWidgetItem*> selectedItems = ui->treeWidget_asn1->selectedItems();
    if (selectedItems.isEmpty())
    {
        highlightRange(0U, 0U, 0U);
        return;
    }

    QTreeWidgetItem*  item          = selectedItems.first();
    const std::size_t offset        = static_cast<std::size_t>(item->data(0, Qt::UserRole).toULongLong());
    const std::size_t encodedLength = static_cast<std::size_t>(item->data(0, Qt::UserRole + 5).toULongLong());
    const std::size_t valueLength   = static_cast<std::size_t>(item->data(0, Qt::UserRole + 6).toULongLong());
    highlightRange(offset, encodedLength, valueLength);
}

void PluginAsn1Viewer::initWidget()
{
    setupDetailPane();
    connect(ui->pushButton_open, &QPushButton::clicked, this, &PluginAsn1Viewer::slotPushButtonOpenClicked);
    connect(ui->treeWidget_asn1, &QTreeWidget::itemSelectionChanged, this, &PluginAsn1Viewer::slotTreeItemSelectionChanged);
    ui->treeWidget_asn1->setColumnCount(1);
    ui->treeWidget_asn1->setHeaderHidden(true);
    ui->label_summary->hide();
    clearResult();
}

void PluginAsn1Viewer::setupDetailPane()
{
    const int detailIndex = ui->splitter->indexOf(ui->plainTextEdit_detail);
    QWidget*  oldWidget   = ui->plainTextEdit_detail;

    m_hexView = new HexViewWidget(ui->splitter);
    ui->splitter->replaceWidget(detailIndex, m_hexView);
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 0);

    if (oldWidget != nullptr)
    {
        oldWidget->deleteLater();
    }
}

void PluginAsn1Viewer::showDocument(const core::Asn1Document& document)
{
    clearResult();
    m_currentDocument = document;
    ui->lineEdit_file->setText(toQString(document.sourceName));
    for (const core::Asn1NodeInfo& rootNode : document.rootNodes)
    {
        appendNode(rootNode, nullptr);
    }
    showHexDump();
    ui->treeWidget_asn1->expandAll();
}

void PluginAsn1Viewer::appendNode(const core::Asn1NodeInfo& nodeInfo, QTreeWidgetItem* parentItem)
{
    const QString nodeTitle = buildNodeText(nodeInfo);
    auto*         item      = parentItem == nullptr ? new QTreeWidgetItem(ui->treeWidget_asn1) : new QTreeWidgetItem(parentItem);
    item->setText(0, nodeTitle);
    item->setData(0, Qt::UserRole, static_cast<qulonglong>(nodeInfo.offset));
    item->setData(0, Qt::UserRole + 5, static_cast<qulonglong>(nodeInfo.encodedLength));
    item->setData(0, Qt::UserRole + 6, static_cast<qulonglong>(nodeInfo.length));

    // TODO(lijun): 根据 ASN.1 节点类型恢复差异化图标。
    Q_UNUSED(buildNodeIcon(nodeInfo));

    for (const core::Asn1NodeInfo& childNode : nodeInfo.children)
    {
        appendNode(childNode, item);
    }
}

void PluginAsn1Viewer::clearResult()
{
    m_currentDocument.reset();
    ui->lineEdit_file->clear();
    ui->treeWidget_asn1->clear();
    if (m_hexView != nullptr)
    {
        m_hexView->clear();
    }
}

void PluginAsn1Viewer::showHexDump()
{
    if (m_hexView == nullptr)
    {
        return;
    }

    if (!m_currentDocument.has_value())
    {
        m_hexView->clear();
        return;
    }

    m_hexView->setBytes(m_currentDocument->sourceBytes);
}

void PluginAsn1Viewer::highlightRange(const std::size_t offset, const std::size_t encodedLength, const std::size_t valueLength)
{
    if (m_hexView == nullptr)
    {
        return;
    }
    m_hexView->scrollToOffset(offset);
    m_hexView->setHighlightRange(offset, encodedLength, valueLength);
}

void PluginAsn1Viewer::publishLog(const std::string& message) const
{
    if (m_eventBus == nullptr)
    {
        return;
    }
    m_eventBus->publish("log", new LogEvent(0, ET_LOG, "PluginAsn1Viewer", __FILE__, __LINE__, __FUNCTION__, message));
}

extern "C" PLUGINASN1VIEWER_API IPlugin* CreatePlugin()
{
    return new PluginAsn1Viewer();
}
