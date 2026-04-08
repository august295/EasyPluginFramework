#ifndef __SM2_PKCS7_VERIFY_WIDGET_H__
#define __SM2_PKCS7_VERIFY_WIDGET_H__

#include <QWidget>

#include "core/sm2_pkcs7_verify_types.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;

class Sm2Pkcs7VerifyWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit Sm2Pkcs7VerifyWidget(QWidget* parent = nullptr);

    QString signatureData() const;
    QString originalData() const;
    QString userId() const;
    void    showResult(const core::Sm2Pkcs7VerifyResult& result);
    void    clearResult();

signals:
    void verifyRequested();

private:
    void createLayout();

private:
    QLineEdit*      m_idEdit;
    QPlainTextEdit* m_signatureEdit;
    QPlainTextEdit* m_originalEdit;
    QLabel*         m_statusLabel;
    QPlainTextEdit* m_resultEdit;
    QPushButton*    m_verifyButton;
    QPushButton*    m_clearButton;
};

#endif
