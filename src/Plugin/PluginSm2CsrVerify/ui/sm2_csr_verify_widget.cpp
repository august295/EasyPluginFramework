#include "sm2_csr_verify_widget.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QTextOption>
#include <QVBoxLayout>

namespace
{
QString buildResultText(const core::Sm2CsrVerifyResult& result)
{
    QStringList lines;
    lines << QStringLiteral("消息：%1").arg(QString::fromUtf8(result.message.c_str()));
    lines << QStringLiteral("解码后 CSR 字节数：%1").arg(result.decodedCsrBytes);
    lines << QStringLiteral("序列化请求信息字节数：%1").arg(result.serializedRequestBytes);
    lines << QStringLiteral("请求信息已解析：%1").arg(result.parsedRequestInfo ? QStringLiteral("是") : QStringLiteral("否"));
    lines << QStringLiteral("签名值已解析：%1").arg(result.parsedSignature ? QStringLiteral("是") : QStringLiteral("否"));
    lines << QStringLiteral("公钥已解析：%1").arg(result.parsedPublicKey ? QStringLiteral("是") : QStringLiteral("否"));

    if (!result.subjectSummary.empty())
    {
        lines << QStringLiteral("CSR Subject：%1").arg(QString::fromUtf8(result.subjectSummary.c_str()));
    }
    if (!result.signatureRHex.empty())
    {
        lines << QStringLiteral("R：%1").arg(QString::fromLatin1(result.signatureRHex.c_str()));
    }
    if (!result.signatureSHex.empty())
    {
        lines << QStringLiteral("S：%1").arg(QString::fromLatin1(result.signatureSHex.c_str()));
    }
    if (!result.publicKeyHex.empty())
    {
        lines << QStringLiteral("公钥：%1").arg(QString::fromLatin1(result.publicKeyHex.c_str()));
    }

    return lines.join(QLatin1Char('\n'));
}
}

Sm2CsrVerifyWidget::Sm2CsrVerifyWidget(QWidget* parent)
    : QWidget(parent)
    , m_idEdit(nullptr)
    , m_csrEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_resultEdit(nullptr)
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
    m_resultEdit->setPlainText(buildResultText(result));
}

void Sm2CsrVerifyWidget::clearResult()
{
    m_statusLabel->setText(tr("验证结果：未执行"));
    m_statusLabel->setStyleSheet(QStringLiteral("QLabel { color: #344054; font-weight: 600; }"));
    m_resultEdit->setPlainText(tr("请填写 CSR 数据和 SM2 ID，然后点击“开始验证”。"));
}

void Sm2CsrVerifyWidget::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    m_idEdit = new QLineEdit(QStringLiteral("1234567812345678"), this);
    m_csrEdit = new QPlainTextEdit(this);
    m_statusLabel = new QLabel(this);
    m_resultEdit = new QPlainTextEdit(this);
    m_verifyButton = new QPushButton(tr("开始验证"), this);
    m_clearButton = new QPushButton(tr("清空"), this);

    m_csrEdit->setPlaceholderText(tr("请输入 Base64 编码的 CSR 数据"));
    m_csrEdit->setMinimumHeight(220);
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setMinimumHeight(220);
    m_resultEdit->setWordWrapMode(QTextOption::WrapAnywhere);

    formLayout->addRow(tr("SM2 ID"), m_idEdit);
    formLayout->addRow(tr("CSR 数据"), m_csrEdit);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_verifyButton);

    rootLayout->addLayout(formLayout);
    rootLayout->addLayout(buttonLayout);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addWidget(m_resultEdit);

    clearResult();

    connect(m_verifyButton, &QPushButton::clicked, this, &Sm2CsrVerifyWidget::verifyRequested);
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_csrEdit->clear();
        clearResult();
    });
}
