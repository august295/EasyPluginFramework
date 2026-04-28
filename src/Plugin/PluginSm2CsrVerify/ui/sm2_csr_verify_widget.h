#ifndef __SM2_CSR_VERIFY_WIDGET_H__
#define __SM2_CSR_VERIFY_WIDGET_H__

#include <QWidget>

#include "core/sm2_csr_verify_types.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QTableWidget;

class Sm2CsrVerifyWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit Sm2CsrVerifyWidget(QWidget* parent = nullptr);

    QString csrData() const;
    QString userId() const;
    void    showResult(const core::Sm2CsrVerifyResult& result);
    void    clearResult();

signals:
    void verifyRequested();

private:
    void resizeEvent(QResizeEvent* event) override;
    void createLayout();
    void setTableRow(int row, const QString& name, const QString& value);
    QPlainTextEdit* ensureValueEditor(int row);
    void updateResultTableColumnWidths();

private:
    QLineEdit*      m_idEdit;
    QPlainTextEdit* m_csrEdit;
    QLabel*         m_statusLabel;
    QTableWidget*   m_resultTable;
    QPushButton*    m_verifyButton;
    QPushButton*    m_clearButton;
};

#endif
