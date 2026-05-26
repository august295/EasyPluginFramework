#include "p7b_certificate_chain_widget.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextOption>
#include <QVBoxLayout>

namespace
{
constexpr int kTableColumnCount = 5;
constexpr int kIndexColumnWidth = 70;
constexpr int kSerialColumnWidth = 220;
}

P7bCertificateChainWidget::P7bCertificateChainWidget(QWidget* parent)
    : QWidget(parent)
    , m_chainEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_summaryLabel(nullptr)
    , m_certificateTable(nullptr)
    , m_pemPreviewEdit(nullptr)
    , m_openFileButton(nullptr)
    , m_parseButton(nullptr)
    , m_exportSelectedButton(nullptr)
    , m_exportAllButton(nullptr)
    , m_clearButton(nullptr)
{
    createLayout();
}

QString P7bCertificateChainWidget::chainData() const
{
    return m_chainEdit->toPlainText();
}

void P7bCertificateChainWidget::setChainData(const QString& text)
{
    m_chainEdit->setPlainText(text);
}

int P7bCertificateChainWidget::selectedCertificateIndex() const
{
    return m_certificateTable->currentRow();
}

void P7bCertificateChainWidget::showResult(const core::P7bCertificateChainResult& result)
{
    m_lastResult = result;
    showActionMessage(QString::fromUtf8(result.message.c_str()), result.success);

    if (!result.success)
    {
        m_summaryLabel->setText(tr("未解析到证书"));
        m_certificateTable->setRowCount(0);
        m_pemPreviewEdit->clear();
        setExportButtonsEnabled(false);
        return;
    }

    m_summaryLabel->setText(tr("解析成功：共 %1 张证书，PKCS7 字节数=%2。未选择证书时，下方显示完整 PEM 链。")
                                .arg(static_cast<qulonglong>(result.parsedCertificateCount))
                                .arg(static_cast<qulonglong>(result.parsedPkcs7Bytes)));

    updateCertificateTable(result);
    m_pemPreviewEdit->setPlainText(QString::fromUtf8(result.fullChainPem.c_str()));
    setExportButtonsEnabled(true);
}

void P7bCertificateChainWidget::showActionMessage(const QString& message, const bool success)
{
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet(
        success ? QStringLiteral("QLabel { color: #1b7f3a; font-weight: 600; }")
                : QStringLiteral("QLabel { color: #b42318; font-weight: 600; }"));
}

void P7bCertificateChainWidget::clearResult()
{
    m_lastResult = {};
    m_statusLabel->setText(tr("尚未开始解析"));
    m_statusLabel->setStyleSheet(QStringLiteral("QLabel { color: #344054; font-weight: 600; }"));
    m_summaryLabel->setText(tr("请输入 p7b/pkcs7 证书链内容，然后点击“解析证书链”。"));
    m_certificateTable->setRowCount(0);
    m_pemPreviewEdit->setPlainText(tr("解析成功后，这里会显示完整 PEM 链或当前选中证书的 PEM。"));
    setExportButtonsEnabled(false);
}

void P7bCertificateChainWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateTableColumnWidths();
}

void P7bCertificateChainWidget::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    auto* buttonLayout = new QHBoxLayout();

    auto* inputLabel = new QLabel(tr("p7b / PKCS7 证书链"), this);
    auto* tableLabel = new QLabel(tr("证书列表"), this);
    auto* previewLabel = new QLabel(tr("PEM 预览"), this);

    m_chainEdit = new QPlainTextEdit(this);
    m_statusLabel = new QLabel(this);
    m_summaryLabel = new QLabel(this);
    m_certificateTable = new QTableWidget(0, kTableColumnCount, this);
    m_pemPreviewEdit = new QPlainTextEdit(this);
    m_openFileButton = new QPushButton(tr("选择证书链文件"), this);
    m_parseButton = new QPushButton(tr("解析证书链"), this);
    m_exportSelectedButton = new QPushButton(tr("导出选中证书"), this);
    m_exportAllButton = new QPushButton(tr("导出全部证书"), this);
    m_clearButton = new QPushButton(tr("清空"), this);

    m_chainEdit->setPlaceholderText(tr("请输入 PEM 包裹的 PKCS7 数据，或直接粘贴 Base64 编码的 p7b 内容"));
    m_chainEdit->setMinimumHeight(200);

    m_certificateTable->setHorizontalHeaderLabels({tr("序号"), tr("Subject"), tr("Issuer"), tr("Serial"), tr("有效期")});
    m_certificateTable->verticalHeader()->setVisible(false);
    m_certificateTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_certificateTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_certificateTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_certificateTable->setAlternatingRowColors(true);
    m_certificateTable->horizontalHeader()->setStretchLastSection(true);
    m_certificateTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_certificateTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_certificateTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_certificateTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_certificateTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_certificateTable->setMinimumHeight(220);

    m_pemPreviewEdit->setReadOnly(true);
    m_pemPreviewEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_pemPreviewEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    m_pemPreviewEdit->setMinimumHeight(240);

    buttonLayout->addWidget(m_openFileButton);
    buttonLayout->addWidget(m_parseButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_exportSelectedButton);
    buttonLayout->addWidget(m_exportAllButton);
    buttonLayout->addWidget(m_clearButton);

    rootLayout->addWidget(inputLabel);
    rootLayout->addWidget(m_chainEdit);
    rootLayout->addLayout(buttonLayout);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addWidget(m_summaryLabel);
    rootLayout->addWidget(tableLabel);
    rootLayout->addWidget(m_certificateTable);
    rootLayout->addWidget(previewLabel);
    rootLayout->addWidget(m_pemPreviewEdit);

    clearResult();
    updateTableColumnWidths();

    connect(m_openFileButton, &QPushButton::clicked, this, &P7bCertificateChainWidget::openFileRequested);
    connect(m_parseButton, &QPushButton::clicked, this, &P7bCertificateChainWidget::parseRequested);
    connect(m_exportSelectedButton, &QPushButton::clicked, this, &P7bCertificateChainWidget::exportSelectedRequested);
    connect(m_exportAllButton, &QPushButton::clicked, this, &P7bCertificateChainWidget::exportAllRequested);
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_chainEdit->clear();
        clearResult();
    });
    connect(m_certificateTable, &QTableWidget::itemSelectionChanged, this, &P7bCertificateChainWidget::updatePemPreview);
}

void P7bCertificateChainWidget::updateCertificateTable(const core::P7bCertificateChainResult& result)
{
    m_certificateTable->setRowCount(static_cast<int>(result.certificates.size()));
    for (int row = 0; row < m_certificateTable->rowCount(); ++row)
    {
        const core::P7bCertificateSummary& certificate = result.certificates[static_cast<std::size_t>(row)];

        auto* indexItem = new QTableWidgetItem(QString::number(static_cast<qulonglong>(certificate.index + 1U)));
        auto* subjectItem = new QTableWidgetItem(QString::fromUtf8(certificate.subject.c_str()));
        auto* issuerItem = new QTableWidgetItem(QString::fromUtf8(certificate.issuer.c_str()));
        auto* serialItem = new QTableWidgetItem(QString::fromLatin1(certificate.serialNumber.c_str()));
        auto* validityItem =
            new QTableWidgetItem(QStringLiteral("%1 ~ %2")
                                     .arg(QString::fromUtf8(certificate.notBefore.c_str()))
                                     .arg(QString::fromUtf8(certificate.notAfter.c_str())));

        subjectItem->setData(Qt::UserRole, QString::fromUtf8(certificate.pem.c_str()));

        m_certificateTable->setItem(row, 0, indexItem);
        m_certificateTable->setItem(row, 1, subjectItem);
        m_certificateTable->setItem(row, 2, issuerItem);
        m_certificateTable->setItem(row, 3, serialItem);
        m_certificateTable->setItem(row, 4, validityItem);
    }

    if (m_certificateTable->rowCount() > 0)
    {
        m_certificateTable->selectRow(0);
    }
}

void P7bCertificateChainWidget::updatePemPreview()
{
    const int row = m_certificateTable->currentRow();
    if (row < 0)
    {
        m_pemPreviewEdit->setPlainText(QString::fromUtf8(m_lastResult.fullChainPem.c_str()));
        return;
    }

    QTableWidgetItem* subjectItem = m_certificateTable->item(row, 1);
    if (subjectItem == nullptr)
    {
        m_pemPreviewEdit->setPlainText(QString::fromUtf8(m_lastResult.fullChainPem.c_str()));
        return;
    }

    m_pemPreviewEdit->setPlainText(subjectItem->data(Qt::UserRole).toString());
}

void P7bCertificateChainWidget::updateTableColumnWidths()
{
    if (m_certificateTable == nullptr)
    {
        return;
    }

    m_certificateTable->setColumnWidth(0, kIndexColumnWidth);
    m_certificateTable->setColumnWidth(3, kSerialColumnWidth);
}

void P7bCertificateChainWidget::setExportButtonsEnabled(const bool enabled)
{
    m_exportSelectedButton->setEnabled(enabled);
    m_exportAllButton->setEnabled(enabled);
}
