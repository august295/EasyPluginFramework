#ifndef __P7B_CERTIFICATE_CHAIN_WIDGET_H__
#define __P7B_CERTIFICATE_CHAIN_WIDGET_H__

#include <QWidget>

#include "core/p7b_certificate_chain_types.h"

class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTableWidget;

class P7bCertificateChainWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit P7bCertificateChainWidget(QWidget* parent = nullptr);

    QString chainData() const;
    void    setChainData(const QString& text);
    int     selectedCertificateIndex() const;
    void    showResult(const core::P7bCertificateChainResult& result);
    void    showActionMessage(const QString& message, bool success);
    void    clearResult();

signals:
    void openFileRequested();
    void parseRequested();
    void exportSelectedRequested();
    void exportAllRequested();

private:
    void resizeEvent(QResizeEvent* event) override;
    void createLayout();
    void updateCertificateTable(const core::P7bCertificateChainResult& result);
    void updatePemPreview();
    void updateTableColumnWidths();
    void setExportButtonsEnabled(bool enabled);

private:
    QPlainTextEdit* m_chainEdit;
    QLabel* m_statusLabel;
    QLabel* m_summaryLabel;
    QTableWidget* m_certificateTable;
    QPlainTextEdit* m_pemPreviewEdit;
    QPushButton* m_openFileButton;
    QPushButton* m_parseButton;
    QPushButton* m_exportSelectedButton;
    QPushButton* m_exportAllButton;
    QPushButton* m_clearButton;
    core::P7bCertificateChainResult m_lastResult;
};

#endif
