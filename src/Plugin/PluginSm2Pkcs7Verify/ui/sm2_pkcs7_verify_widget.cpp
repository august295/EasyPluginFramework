#include "sm2_pkcs7_verify_widget.h"

#include <algorithm>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextOption>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
constexpr int kResultRowCount = 10;
constexpr int kResultFieldColumnWidth = 200;
}

Sm2Pkcs7VerifyWidget::Sm2Pkcs7VerifyWidget(QWidget* parent)
    : QWidget(parent)
    , m_formLayout(nullptr)
    , m_embeddedOriginalCheck(nullptr)
    , m_idEdit(nullptr)
    , m_signatureEdit(nullptr)
    , m_originalInputLabel(nullptr)
    , m_originalInputContainer(nullptr)
    , m_originalEdit(nullptr)
    , m_embeddedOriginalLabel(nullptr)
    , m_embeddedOriginalView(nullptr)
    , m_statusLabel(nullptr)
    , m_resultTable(nullptr)
    , m_verifyButton(nullptr)
    , m_clearButton(nullptr)
{
    createLayout();
}

QString Sm2Pkcs7VerifyWidget::signatureData() const
{
    return m_signatureEdit->toPlainText();
}

QString Sm2Pkcs7VerifyWidget::originalData() const
{
    return m_originalEdit->toPlainText();
}

QString Sm2Pkcs7VerifyWidget::userId() const
{
    return m_idEdit->text().trimmed();
}

bool Sm2Pkcs7VerifyWidget::hasEmbeddedOriginal() const
{
    return m_embeddedOriginalCheck->isChecked();
}

void Sm2Pkcs7VerifyWidget::showResult(const core::Sm2Pkcs7VerifyResult& result)
{
    m_statusLabel->setText(result.success ? tr("验签结果：成功") : tr("验签结果：失败"));
    m_statusLabel->setStyleSheet(
        result.success ? QStringLiteral("QLabel { color: #1b7f3a; font-weight: 600; }")
                       : QStringLiteral("QLabel { color: #b42318; font-weight: 600; }"));

    setTableRow(0, tr("消息"), QString::fromUtf8(result.message.c_str()));
    setTableRow(1, tr("解码后签名字节数"), QString::number(result.decodedSignatureBytes));
    setTableRow(2, tr("签名值已解析"), result.parsedSignature ? tr("是") : tr("否"));
    setTableRow(3, tr("公钥已解析"), result.parsedPublicKey ? tr("是") : tr("否"));
    setTableRow(4, tr("已解析携带原文"), result.parsedEmbeddedOriginal ? tr("是") : tr("否"));
    setTableRow(5, tr("证书主题"), QString::fromUtf8(result.signerSubject.c_str()));
    setTableRow(6, tr("证书颁发者"), QString::fromUtf8(result.signerIssuer.c_str()));
    setTableRow(7, tr("签名 R"), QString::fromLatin1(result.signatureRHex.c_str()));
    setTableRow(8, tr("签名 S"), QString::fromLatin1(result.signatureSHex.c_str()));
    setTableRow(9, tr("公钥"), QString::fromLatin1(result.publicKeyHex.c_str()));

    m_embeddedOriginalView->setPlainText(QString::fromUtf8(result.embeddedOriginalText.c_str()));
    setEmbeddedOriginalDisplayVisible(m_embeddedOriginalCheck->isChecked());
}

void Sm2Pkcs7VerifyWidget::clearResult()
{
    m_statusLabel->setText(tr("验签结果：未执行"));
    m_statusLabel->setStyleSheet(QStringLiteral("QLabel { color: #344054; font-weight: 600; }"));

    for (int row = 0; row < kResultRowCount; ++row)
    {
        setTableRow(row, QString(), QString());
    }
    setTableRow(0, tr("消息"), tr("请填写签名数据和 SM2 ID，然后点击“开始验签”。"));
    setTableRow(4, tr("已解析携带原文"), tr("否"));

    m_embeddedOriginalView->setPlainText(tr("当 PKCS7 携带原文时，解析出的原文会显示在这里。"));
    setEmbeddedOriginalDisplayVisible(m_embeddedOriginalCheck->isChecked());
}

void Sm2Pkcs7VerifyWidget::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    m_formLayout = new QFormLayout();

    m_embeddedOriginalCheck = new QCheckBox(tr("签名数据携带原文"), this);
    m_idEdit = new QLineEdit(QStringLiteral("1234567812345678"), this);
    m_signatureEdit = new QPlainTextEdit(this);
    m_originalInputContainer = new QWidget(this);
    m_originalEdit = new QPlainTextEdit(m_originalInputContainer);
    m_embeddedOriginalView = new QPlainTextEdit(this);
    m_statusLabel = new QLabel(this);
    m_resultTable = new QTableWidget(kResultRowCount, 2, this);
    m_verifyButton = new QPushButton(tr("开始验签"), this);
    m_clearButton = new QPushButton(tr("清空"), this);

    auto* originalLayout = new QVBoxLayout(m_originalInputContainer);
    originalLayout->setContentsMargins(0, 0, 0, 0);
    originalLayout->addWidget(m_originalEdit);

    m_signatureEdit->setPlaceholderText(tr("请输入 Base64 编码的 PKCS7 签名数据"));
    m_originalEdit->setPlaceholderText(tr("请输入参与签名的原文数据"));
    m_embeddedOriginalView->setPlaceholderText(tr("携带原文模式下，这里显示从 PKCS7 中解析出的原文"));

    m_signatureEdit->setMinimumHeight(180);
    m_originalEdit->setMinimumHeight(120);
    m_embeddedOriginalView->setMinimumHeight(120);
    m_embeddedOriginalView->setReadOnly(true);
    m_embeddedOriginalView->setWordWrapMode(QTextOption::WrapAnywhere);

    m_resultTable->setHorizontalHeaderLabels({tr("字段"), tr("值")});
    m_resultTable->verticalHeader()->setVisible(false);
    m_resultTable->horizontalHeader()->setStretchLastSection(true);
    m_resultTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_resultTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_resultTable->setFocusPolicy(Qt::NoFocus);
    m_resultTable->setMinimumHeight(260);

    m_formLayout->addRow(tr("SM2 ID"), m_idEdit);
    m_formLayout->addRow(tr("验签模式"), m_embeddedOriginalCheck);
    m_formLayout->addRow(tr("签名数据"), m_signatureEdit);
    m_formLayout->addRow(tr("原文数据"), m_originalInputContainer);
    m_originalInputLabel = qobject_cast<QLabel*>(m_formLayout->labelForField(m_originalInputContainer));

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_verifyButton);

    rootLayout->addLayout(m_formLayout);
    rootLayout->addLayout(buttonLayout);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addWidget(new QLabel(tr("验签结果"), this));
    rootLayout->addWidget(m_resultTable);
    m_embeddedOriginalLabel = new QLabel(tr("解析出的原文"), this);
    rootLayout->addWidget(m_embeddedOriginalLabel);
    rootLayout->addWidget(m_embeddedOriginalView);

    clearResult();
    setOriginalInputVisible(true);
    setEmbeddedOriginalDisplayVisible(false);
    updateResultTableColumnWidths();

    connect(m_embeddedOriginalCheck, &QCheckBox::toggled, this, [this](const bool checked) {
        setOriginalInputVisible(!checked);
        setEmbeddedOriginalDisplayVisible(checked);
        if (checked)
        {
            m_originalEdit->clear();
        }
    });
    connect(m_verifyButton, &QPushButton::clicked, this, &Sm2Pkcs7VerifyWidget::verifyRequested);
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_signatureEdit->clear();
        m_originalEdit->clear();
        m_embeddedOriginalView->clear();
        clearResult();
    });
}

void Sm2Pkcs7VerifyWidget::setOriginalInputVisible(const bool visible)
{
    if (m_originalInputLabel != nullptr)
    {
        m_originalInputLabel->setVisible(visible);
    }
    m_originalInputContainer->setVisible(visible);
}

void Sm2Pkcs7VerifyWidget::setEmbeddedOriginalDisplayVisible(const bool visible)
{
    if (m_embeddedOriginalLabel != nullptr)
    {
        m_embeddedOriginalLabel->setVisible(visible);
    }
    m_embeddedOriginalView->setVisible(visible);
}

void Sm2Pkcs7VerifyWidget::setTableRow(const int row, const QString& name, const QString& value)
{
    auto* nameItem = m_resultTable->item(row, 0);
    if (nameItem == nullptr)
    {
        nameItem = new QTableWidgetItem();
        m_resultTable->setItem(row, 0, nameItem);
    }
    nameItem->setText(name);

    QPlainTextEdit* valueEditor = ensureValueEditor(row);
    valueEditor->setPlainText(value);

    const int lineCount = std::max(1, value.count(QLatin1Char('\n')) + 1);
    const int height = value.length() > 96 ? 110 : (lineCount > 1 ? 72 : 40);
    valueEditor->setFixedHeight(height);
    m_resultTable->setRowHeight(row, height + 6);
}

QPlainTextEdit* Sm2Pkcs7VerifyWidget::ensureValueEditor(const int row)
{
    auto* editor = qobject_cast<QPlainTextEdit*>(m_resultTable->cellWidget(row, 1));
    if (editor != nullptr)
    {
        return editor;
    }

    editor = new QPlainTextEdit(m_resultTable);
    editor->setReadOnly(true);
    editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    editor->setWordWrapMode(QTextOption::WrapAnywhere);
    editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_resultTable->setCellWidget(row, 1, editor);
    return editor;
}

void Sm2Pkcs7VerifyWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateResultTableColumnWidths();
}

void Sm2Pkcs7VerifyWidget::updateResultTableColumnWidths()
{
    if (m_resultTable == nullptr)
    {
        return;
    }

    m_resultTable->setColumnWidth(0, kResultFieldColumnWidth);
}
