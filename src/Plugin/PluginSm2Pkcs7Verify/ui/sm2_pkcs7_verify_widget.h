#ifndef __SM2_PKCS7_VERIFY_WIDGET_H__
#define __SM2_PKCS7_VERIFY_WIDGET_H__

#include <QWidget>

#include "core/sm2_pkcs7_verify_types.h"

class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QPlainTextEdit;
class QTableWidget;
class QWidget;
class QFormLayout;

class Sm2Pkcs7VerifyWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit Sm2Pkcs7VerifyWidget(QWidget* parent = nullptr);

    QString signatureData() const;
    QString originalData() const;
    QString userId() const;
    bool    hasEmbeddedOriginal() const;
    void    showResult(const core::Sm2Pkcs7VerifyResult& result);
    void    clearResult();

signals:
    void verifyRequested();

private:
    void resizeEvent(QResizeEvent* event) override;
    void createLayout();
    void setOriginalInputVisible(bool visible);
    void setEmbeddedOriginalDisplayVisible(bool visible);
    void setTableRow(int row, const QString& name, const QString& value);
    QPlainTextEdit* ensureValueEditor(int row);
    void updateResultTableColumnWidths();

private:
    QFormLayout*    m_formLayout;
    QCheckBox*      m_embeddedOriginalCheck;
    QLineEdit*      m_idEdit;
    QPlainTextEdit* m_signatureEdit;
    QLabel*         m_originalInputLabel;
    QWidget*        m_originalInputContainer;
    QPlainTextEdit* m_originalEdit;
    QLabel*         m_embeddedOriginalLabel;
    QPlainTextEdit* m_embeddedOriginalView;
    QLabel*         m_statusLabel;
    QTableWidget*   m_resultTable;
    QPushButton*    m_verifyButton;
    QPushButton*    m_clearButton;
};

#endif
