#include "sm2_csr_verify_widget.h"

#include <algorithm>
#include <QAbstractItemView>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextOption>
#include <QVBoxLayout>

namespace
{
    constexpr int kResultRowCount = 10;
    constexpr int kResultFieldColumnWidth = 200;
}

Sm2CsrVerifyWidget::Sm2CsrVerifyWidget(QWidget* parent)
    : QWidget(parent)
    , m_idEdit(nullptr)
    , m_csrEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_resultTable(nullptr)
    , m_verifyButton(nullptr)
    , m_clearButton(nullptr)
{
    createLayout();
}

QString Sm2CsrVerifyWidget::csrData() const
{
    return m_csrEdit->toPlainText();
}

QString Sm2CsrVerifyWidget::userId() const
{
    return m_idEdit->text().trimmed();
}

void Sm2CsrVerifyWidget::showResult(const core::Sm2CsrVerifyResult& result)
{
    m_statusLabel->setText(result.success ? tr("验证结果：成功") : tr("验证结果：失败"));
    m_statusLabel->setStyleSheet(
        result.success ? QStringLiteral("QLabel { color: #1b7f3a; font-weight: 600; }")
                       : QStringLiteral("QLabel { color: #b42318; font-weight: 600; }"));

    setTableRow(0, tr("消息"), QString::fromUtf8(result.message.c_str()));
    setTableRow(1, tr("解码后 CSR 字节数"), QString::number(result.decodedCsrBytes));
    setTableRow(2, tr("序列化请求信息字节数"), QString::number(result.serializedRequestBytes));
    setTableRow(3, tr("请求信息已解析"), result.parsedRequestInfo ? tr("是") : tr("否"));
    setTableRow(4, tr("签名值已解析"), result.parsedSignature ? tr("是") : tr("否"));
    setTableRow(5, tr("公钥已解析"), result.parsedPublicKey ? tr("是") : tr("否"));
    setTableRow(6, tr("CSR Subject"), QString::fromUtf8(result.subjectSummary.c_str()));
    setTableRow(7, tr("签名 R"), QString::fromLatin1(result.signatureRHex.c_str()));
    setTableRow(8, tr("签名 S"), QString::fromLatin1(result.signatureSHex.c_str()));
    setTableRow(9, tr("公钥"), QString::fromLatin1(result.publicKeyHex.c_str()));
}

void Sm2CsrVerifyWidget::clearResult()
{
    m_statusLabel->setText(tr("验证结果：未执行"));
    m_statusLabel->setStyleSheet(QStringLiteral("QLabel { color: #344054; font-weight: 600; }"));

    for (int row = 0; row < kResultRowCount; ++row)
    {
        setTableRow(row, QString(), QString());
    }
    setTableRow(0, tr("消息"), tr("请填写 CSR 数据和 SM2 ID，然后点击“开始验证”。"));
}

void Sm2CsrVerifyWidget::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    m_idEdit       = new QLineEdit(QStringLiteral("1234567812345678"), this);
    m_csrEdit      = new QPlainTextEdit(this);
    m_statusLabel  = new QLabel(this);
    m_resultTable  = new QTableWidget(kResultRowCount, 2, this);
    m_verifyButton = new QPushButton(tr("开始验证"), this);
    m_clearButton  = new QPushButton(tr("清空"), this);

    m_csrEdit->setPlaceholderText(tr("请输入 Base64 编码的 CSR 数据"));
    m_csrEdit->setMinimumHeight(220);

    m_resultTable->setHorizontalHeaderLabels({tr("字段"), tr("值")});
    m_resultTable->verticalHeader()->setVisible(false);
    m_resultTable->horizontalHeader()->setStretchLastSection(true);
    m_resultTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_resultTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_resultTable->setFocusPolicy(Qt::NoFocus);
    m_resultTable->setMinimumHeight(280);

    formLayout->addRow(tr("SM2 ID"), m_idEdit);
    formLayout->addRow(tr("CSR 数据"), m_csrEdit);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_verifyButton);

    rootLayout->addLayout(formLayout);
    rootLayout->addLayout(buttonLayout);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addWidget(m_resultTable);

    clearResult();
    updateResultTableColumnWidths();

    connect(m_verifyButton, &QPushButton::clicked, this, &Sm2CsrVerifyWidget::verifyRequested);
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_csrEdit->clear();
        clearResult();
    });
}

void Sm2CsrVerifyWidget::setTableRow(const int row, const QString& name, const QString& value)
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
    const int height    = value.length() > 96 ? 110 : (lineCount > 1 ? 72 : 40);
    valueEditor->setFixedHeight(height);
    m_resultTable->setRowHeight(row, height + 6);
}

QPlainTextEdit* Sm2CsrVerifyWidget::ensureValueEditor(const int row)
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

void Sm2CsrVerifyWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateResultTableColumnWidths();
}

void Sm2CsrVerifyWidget::updateResultTableColumnWidths()
{
    if (m_resultTable == nullptr)
    {
        return;
    }

    m_resultTable->setColumnWidth(0, kResultFieldColumnWidth);
}
