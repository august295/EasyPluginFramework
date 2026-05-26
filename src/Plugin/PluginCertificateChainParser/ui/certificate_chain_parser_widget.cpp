#include "certificate_chain_parser_widget.h"

#include <QColor>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextOption>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
constexpr qulonglong kInvalidNodeId = static_cast<qulonglong>(-1);
}

CertificateChainParserWidget::CertificateChainParserWidget(QWidget* parent)
    : QWidget(parent)
    , m_statusLabel(nullptr)
    , m_summaryLabel(nullptr)
    , m_detailLabel(nullptr)
    , m_validationLabel(nullptr)
    , m_validationSummaryLabel(nullptr)
    , m_treeWidget(nullptr)
    , m_detailEdit(nullptr)
    , m_validationEdit(nullptr)
    , m_loadDirectoryButton(nullptr)
    , m_addCertificateButton(nullptr)
    , m_validateCertificateButton(nullptr)
    , m_clearButton(nullptr)
{
    createLayout();
}

void CertificateChainParserWidget::showResult(const core::CertificateAnalysisResult& result)
{
    m_lastResult = result;
    showActionMessage(QString::fromUtf8(result.message.c_str()), result.success);
    m_summaryLabel->setText(buildSummaryText(result));
    populateTree(result);
    updateDetails();
    updateValidationPanel();
}

void CertificateChainParserWidget::showValidationResult(const core::CertificateValidationResult& result)
{
    m_lastValidationResult = result;
    updateValidationPanel();
}

void CertificateChainParserWidget::showActionMessage(const QString& message, const bool success)
{
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet(
        success ? QStringLiteral("QLabel { color: #1b7f3a; font-weight: 600; }")
                : QStringLiteral("QLabel { color: #b42318; font-weight: 600; }"));
}

void CertificateChainParserWidget::clearView()
{
    m_lastResult = {};
    m_lastValidationResult = {};
    m_statusLabel->setText(tr("尚未载入证书"));
    m_statusLabel->setStyleSheet(QStringLiteral("QLabel { color: #344054; font-weight: 600; }"));
    m_summaryLabel->setText(tr("请选择证书目录，或在已有结果上单独添加证书并刷新树形图。"));
    m_treeWidget->clear();
    m_detailEdit->setPlainText(tr("树节点详情、有效性和扩展信息会显示在这里。"));
    m_validationSummaryLabel->setText(tr("请先点击“验证证书”选择待验证证书。"));
    m_validationSummaryLabel->setStyleSheet(QStringLiteral("QLabel { color: #475467; font-weight: 700; }"));
    m_validationEdit->setPlainText(tr("点击下方“验证证书”后，这里会显示是否由树中证书签发，以及 TBS 签名验证结果。"));
}

std::size_t CertificateChainParserWidget::selectedNodeId() const
{
    const QTreeWidgetItem* item = m_treeWidget->currentItem();
    if (item == nullptr)
    {
        return static_cast<std::size_t>(-1);
    }

    return static_cast<std::size_t>(item->data(0, Qt::UserRole).toULongLong());
}

void CertificateChainParserWidget::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    auto* buttonLayout = new QHBoxLayout();
    auto* contentLayout = new QHBoxLayout();
    auto* leftLayout = new QVBoxLayout();
    auto* rightLayout = new QVBoxLayout();
    auto* validationLayout = new QVBoxLayout();

    m_statusLabel = new QLabel(this);
    m_summaryLabel = new QLabel(this);
    m_detailLabel = new QLabel(tr("证书详情"), this);
    m_validationLabel = new QLabel(tr("验证结果"), this);
    m_validationSummaryLabel = new QLabel(this);
    m_treeWidget = new QTreeWidget(this);
    m_detailEdit = new QPlainTextEdit(this);
    m_validationEdit = new QPlainTextEdit(this);
    m_loadDirectoryButton = new QPushButton(tr("扫描目录"), this);
    m_addCertificateButton = new QPushButton(tr("添加证书"), this);
    m_clearButton = new QPushButton(tr("清空"), this);
    m_validateCertificateButton = new QPushButton(tr("验证证书"), this);

    m_treeWidget->setHeaderLabels({tr("证书链树"), tr("状态")});
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_treeWidget->setAlternatingRowColors(true);

    m_detailEdit->setReadOnly(true);
    m_detailEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_detailEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    m_validationLabel->setWordWrap(true);
    m_validationSummaryLabel->setWordWrap(true);
    m_validationSummaryLabel->setStyleSheet(QStringLiteral("QLabel { color: #344054; font-weight: 600; }"));
    m_validationEdit->setReadOnly(true);
    m_validationEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_validationEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    m_validationEdit->setMinimumHeight(180);

    buttonLayout->addWidget(m_loadDirectoryButton);
    buttonLayout->addWidget(m_addCertificateButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_clearButton);

    leftLayout->addWidget(new QLabel(tr("证书链树形图"), this));
    leftLayout->addWidget(m_treeWidget);

    rightLayout->addWidget(m_detailLabel);
    rightLayout->addWidget(m_detailEdit);

    contentLayout->addLayout(leftLayout, 3);
    contentLayout->addLayout(rightLayout, 2);

    validationLayout->addWidget(m_validationLabel);
    validationLayout->addWidget(m_validateCertificateButton);
    validationLayout->addWidget(m_validationSummaryLabel);
    validationLayout->addWidget(m_validationEdit);

    rootLayout->addLayout(buttonLayout);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addWidget(m_summaryLabel);
    rootLayout->addLayout(contentLayout);
    rootLayout->addLayout(validationLayout);

    clearView();

    connect(m_loadDirectoryButton, &QPushButton::clicked, this, &CertificateChainParserWidget::chooseDirectoryRequested);
    connect(m_addCertificateButton, &QPushButton::clicked, this, &CertificateChainParserWidget::addCertificateRequested);
    connect(m_validateCertificateButton, &QPushButton::clicked, this, &CertificateChainParserWidget::validateCertificateRequested);
    connect(m_clearButton, &QPushButton::clicked, this, &CertificateChainParserWidget::clearRequested);
    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &CertificateChainParserWidget::updateDetails);
}

void CertificateChainParserWidget::populateTree(const core::CertificateAnalysisResult& result)
{
    m_treeWidget->clear();

    if (result.nodes.empty())
    {
        return;
    }

    for (const core::CertificateNode& node : result.nodes)
    {
        if (node.parentId == static_cast<std::size_t>(-1))
        {
            addTreeItemRecursive(result, node.id, nullptr);
        }
    }

    if (m_treeWidget->topLevelItemCount() > 0)
    {
        m_treeWidget->expandAll();
        m_treeWidget->setCurrentItem(m_treeWidget->topLevelItem(0));
    }
}

void CertificateChainParserWidget::updateDetails()
{
    const std::size_t nodeId = selectedNodeId();
    if (nodeId == static_cast<std::size_t>(-1) || nodeId >= m_lastResult.nodes.size())
    {
        if (!m_lastResult.issues.empty())
        {
            QString text = tr("导入提示：\n");
            for (const core::CertificateFileIssue& issue : m_lastResult.issues)
            {
                text += QStringLiteral("- %1: %2\n")
                            .arg(QString::fromUtf8(issue.path.c_str()))
                            .arg(QString::fromUtf8(issue.message.c_str()));
            }
            m_detailEdit->setPlainText(text);
        }
        return;
    }

    const core::CertificateNode& node = m_lastResult.nodes[nodeId];
    QString detailText;
    detailText += tr("文件: %1\n").arg(QString::fromUtf8(node.sourcePath.c_str()));
    detailText += tr("Subject: %1\n").arg(QString::fromUtf8(node.subject.c_str()));
    detailText += tr("Issuer: %1\n").arg(QString::fromUtf8(node.issuer.c_str()));
    detailText += tr("Serial: %1\n").arg(QString::fromLatin1(node.serialNumber.c_str()));
    detailText += tr("有效期: %1 ~ %2\n").arg(QString::fromUtf8(node.notBefore.c_str()), QString::fromUtf8(node.notAfter.c_str()));
    detailText += tr("节点类型: %1\n").arg(buildNodeTitle(node));
    detailText += tr("校验状态: %1\n").arg(QString::fromUtf8(node.status.summary.c_str()));
    detailText += tr("是否 CA: %1\n").arg(node.isCa ? tr("是") : tr("否"));
    detailText += tr("SKI: %1\n").arg(QString::fromLatin1(node.subjectKeyIdentifier.c_str()));
    detailText += tr("AKI: %1\n").arg(QString::fromLatin1(node.authorityKeyIdentifier.c_str()));
    detailText += tr("签名算法: %1\n").arg(QString::fromLatin1(node.signatureAlgorithm.c_str()));
    detailText += tr("扩展摘要: %1\n").arg(QString::fromUtf8(node.extensionSummary.c_str()));
    detailText += tr("\nPEM:\n%1").arg(QString::fromUtf8(node.pem.c_str()));

    if (!m_lastResult.issues.empty())
    {
        detailText += tr("\n\n导入提示:\n");
        for (const core::CertificateFileIssue& issue : m_lastResult.issues)
        {
            detailText += QStringLiteral("- %1: %2\n")
                              .arg(QString::fromUtf8(issue.path.c_str()))
                              .arg(QString::fromUtf8(issue.message.c_str()));
        }
    }

    m_detailEdit->setPlainText(detailText);
}

void CertificateChainParserWidget::updateValidationPanel()
{
    if (!m_lastValidationResult.certificateParsed)
    {
        if (!m_lastValidationResult.message.empty())
        {
            m_validationSummaryLabel->setText(tr("验证结果：%1").arg(QString::fromUtf8(m_lastValidationResult.message.c_str())));
            m_validationSummaryLabel->setStyleSheet(QStringLiteral("QLabel { color: #b42318; font-weight: 700; }"));
            m_validationEdit->setPlainText(tr("验证失败：%1").arg(QString::fromUtf8(m_lastValidationResult.message.c_str())));
        }
        return;
    }

    m_validationSummaryLabel->setText(tr("验证结果：%1").arg(QString::fromUtf8(m_lastValidationResult.message.c_str())));
    m_validationSummaryLabel->setStyleSheet(
        m_lastValidationResult.success ? QStringLiteral("QLabel { color: #027a48; font-weight: 700; }")
                                       : QStringLiteral("QLabel { color: #b42318; font-weight: 700; }"));

    QString detailText;
    detailText += tr("验证证书文件: %1\n").arg(QString::fromUtf8(m_lastValidationResult.certificate.sourcePath.c_str()));
    detailText += tr("Subject: %1\n").arg(QString::fromUtf8(m_lastValidationResult.certificate.subject.c_str()));
    detailText += tr("Issuer: %1\n").arg(QString::fromUtf8(m_lastValidationResult.certificate.issuer.c_str()));
    detailText += tr("Serial: %1\n").arg(QString::fromLatin1(m_lastValidationResult.certificate.serialNumber.c_str()));
    detailText += tr("有效期: %1 ~ %2\n")
                      .arg(
                          QString::fromUtf8(m_lastValidationResult.certificate.notBefore.c_str()),
                          QString::fromUtf8(m_lastValidationResult.certificate.notAfter.c_str()));
    detailText += tr("树中签发者匹配: %1\n").arg(m_lastValidationResult.issuerMatched ? tr("是") : tr("否"));
    detailText += tr("签发者是否 CA: %1\n").arg(m_lastValidationResult.issuerIsCa ? tr("是") : tr("否"));
    detailText += tr("TBS 签名校验已执行: %1\n").arg(m_lastValidationResult.tbsSignatureCheckAttempted ? tr("是") : tr("否"));
    detailText += tr("TBS 签名校验结果: %1\n")
                      .arg(m_lastValidationResult.tbsSignatureVerified ? tr("通过") : tr("未通过"));
    detailText += tr("总体结果: %1\n").arg(QString::fromUtf8(m_lastValidationResult.message.c_str()));

    if (!m_lastValidationResult.issuerSubject.empty())
    {
        detailText += tr("树中签发者 Subject: %1\n").arg(QString::fromUtf8(m_lastValidationResult.issuerSubject.c_str()));
    }
    if (!m_lastValidationResult.tbsSignatureMessage.empty())
    {
        detailText += tr("TBS 签名说明: %1\n").arg(QString::fromUtf8(m_lastValidationResult.tbsSignatureMessage.c_str()));
    }

    detailText += tr("\n待验证证书 PEM:\n%1").arg(QString::fromUtf8(m_lastValidationResult.certificate.pem.c_str()));
    m_validationEdit->setPlainText(detailText);
}

QString CertificateChainParserWidget::buildNodeTitle(const core::CertificateNode& node) const
{
    switch (node.kind)
    {
    case core::CertificateNodeKind::Root:
        return tr("根证书");
    case core::CertificateNodeKind::Intermediate:
        return tr("中间证书");
    case core::CertificateNodeKind::Leaf:
        return tr("叶子证书");
    case core::CertificateNodeKind::Orphan:
    default:
        return tr("孤儿证书");
    }
}

QString CertificateChainParserWidget::buildNodeStatus(const core::CertificateNode& node) const
{
    return QString::fromUtf8(node.status.summary.c_str());
}

QString CertificateChainParserWidget::buildSummaryText(const core::CertificateAnalysisResult& result) const
{
    if (!result.success)
    {
        if (!result.issues.empty())
        {
            return tr("没有成功导入证书，但记录了 %1 条失败/提示信息。").arg(static_cast<qulonglong>(result.issues.size()));
        }
        return tr("当前没有可展示的证书链。");
    }

    return tr("共 %1 张证书，根 %2 个，孤儿 %3 个，导入提示 %4 条。")
        .arg(static_cast<qulonglong>(result.loadedCertificateCount))
        .arg(static_cast<qulonglong>(result.rootCount))
        .arg(static_cast<qulonglong>(result.orphanCount))
        .arg(static_cast<qulonglong>(result.issues.size()));
}

void CertificateChainParserWidget::addTreeItemRecursive(
    const core::CertificateAnalysisResult& result,
    const std::size_t nodeId,
    QTreeWidgetItem* parentItem)
{
    if (nodeId >= result.nodes.size())
    {
        return;
    }

    const core::CertificateNode& node = result.nodes[nodeId];
    auto* item = new QTreeWidgetItem();
    item->setText(0, QStringLiteral("%1 [%2]").arg(QString::fromUtf8(node.subject.c_str()), buildNodeTitle(node)));
    item->setText(1, buildNodeStatus(node));
    item->setData(0, Qt::UserRole, QVariant::fromValue<qulonglong>(static_cast<qulonglong>(node.id)));

    if (node.kind == core::CertificateNodeKind::Orphan)
    {
        item->setForeground(0, QColor(QStringLiteral("#b42318")));
        item->setForeground(1, QColor(QStringLiteral("#b42318")));
    }
    else if (node.kind == core::CertificateNodeKind::Root)
    {
        item->setForeground(0, QColor(QStringLiteral("#175cd3")));
    }
    else if (node.kind == core::CertificateNodeKind::Intermediate)
    {
        item->setForeground(0, QColor(QStringLiteral("#027a48")));
    }

    if (parentItem == nullptr)
    {
        m_treeWidget->addTopLevelItem(item);
    }
    else
    {
        parentItem->addChild(item);
    }

    for (const std::size_t childId : node.children)
    {
        addTreeItemRecursive(result, childId, item);
    }
}
