#include "sm2_pkcs7_verify_widget.h"

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
QString buildResultText(const core::Sm2Pkcs7VerifyResult& result)
{
    QStringList lines;
    lines << QStringLiteral("消息：%1").arg(QString::fromUtf8(result.message.c_str()));
    lines << QStringLiteral("解码后签名字节数：%1").arg(result.decodedSignatureBytes);
    lines << QStringLiteral("签名值已解析：%1").arg(result.parsedSignature ? QStringLiteral("是") : QStringLiteral("否"));
    lines << QStringLiteral("公钥已解析：%1").arg(result.parsedPublicKey ? QStringLiteral("是") : QStringLiteral("否"));

    if (!result.signerSubject.empty())
    {
        lines << QStringLiteral("证书主题：%1").arg(QString::fromUtf8(result.signerSubject.c_str()));
    }
    if (!result.signerIssuer.empty())
    {
        lines << QStringLiteral("证书颁发者：%1").arg(QString::fromUtf8(result.signerIssuer.c_str()));
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

Sm2Pkcs7VerifyWidget::Sm2Pkcs7VerifyWidget(QWidget* parent)
    : QWidget(parent)
    , m_idEdit(nullptr)
    , m_signatureEdit(nullptr)
    , m_originalEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_resultEdit(nullptr)
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

void Sm2Pkcs7VerifyWidget::showResult(const core::Sm2Pkcs7VerifyResult& result)
{
    m_statusLabel->setText(result.success ? tr("验签结果：成功") : tr("验签结果：失败"));
    m_statusLabel->setStyleSheet(
        result.success ? QStringLiteral("QLabel { color: #1b7f3a; font-weight: 600; }")
                       : QStringLiteral("QLabel { color: #b42318; font-weight: 600; }"));
    m_resultEdit->setPlainText(buildResultText(result));
}

void Sm2Pkcs7VerifyWidget::clearResult()
{
    m_statusLabel->setText(tr("验签结果：未执行"));
    m_statusLabel->setStyleSheet(QStringLiteral("QLabel { color: #344054; font-weight: 600; }"));
    m_resultEdit->setPlainText(tr("请填写签名数据、原文和 SM2 ID，然后点击“开始验签”。"));
}

void Sm2Pkcs7VerifyWidget::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    m_idEdit = new QLineEdit(QStringLiteral("1234567812345678"), this);
    m_signatureEdit = new QPlainTextEdit(this);
    m_originalEdit = new QPlainTextEdit(this);
    m_statusLabel = new QLabel(this);
    m_resultEdit = new QPlainTextEdit(this);
    m_verifyButton = new QPushButton(tr("开始验签"), this);
    m_clearButton = new QPushButton(tr("清空"), this);

    m_signatureEdit->setPlaceholderText(tr("请输入 Base64 编码的 PKCS7 签名数据"));
    m_originalEdit->setPlaceholderText(tr("请输入参与签名的原文数据"));
    m_signatureEdit->setMinimumHeight(180);
    m_originalEdit->setMinimumHeight(120);
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setMinimumHeight(220);
    m_resultEdit->setWordWrapMode(QTextOption::WrapAnywhere);

    formLayout->addRow(tr("SM2 ID"), m_idEdit);
    formLayout->addRow(tr("签名数据"), m_signatureEdit);
    formLayout->addRow(tr("原文数据"), m_originalEdit);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_verifyButton);

    rootLayout->addLayout(formLayout);
    rootLayout->addLayout(buttonLayout);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addWidget(m_resultEdit);

    clearResult();

    connect(m_verifyButton, &QPushButton::clicked, this, &Sm2Pkcs7VerifyWidget::verifyRequested);
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_signatureEdit->clear();
        m_originalEdit->clear();
        clearResult();
    });
}
