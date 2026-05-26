#ifndef __CERTIFICATE_CHAIN_PARSER_WIDGET_H__
#define __CERTIFICATE_CHAIN_PARSER_WIDGET_H__

#include <QWidget>

#include "core/certificate_chain_types.h"

class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class CertificateChainParserWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit CertificateChainParserWidget(QWidget* parent = nullptr);

    void showResult(const core::CertificateAnalysisResult& result);
    void showValidationResult(const core::CertificateValidationResult& result);
    void showActionMessage(const QString& message, bool success);
    void clearView();
    std::size_t selectedNodeId() const;

signals:
    void chooseDirectoryRequested();
    void addCertificateRequested();
    void validateCertificateRequested();
    void clearRequested();

private:
    void createLayout();
    void populateTree(const core::CertificateAnalysisResult& result);
    void updateDetails();
    void updateValidationPanel();
    QString buildNodeTitle(const core::CertificateNode& node) const;
    QString buildNodeStatus(const core::CertificateNode& node) const;
    QString buildSummaryText(const core::CertificateAnalysisResult& result) const;
    void addTreeItemRecursive(const core::CertificateAnalysisResult& result, std::size_t nodeId, QTreeWidgetItem* parentItem);

private:
    QLabel* m_statusLabel;
    QLabel* m_summaryLabel;
    QLabel* m_detailLabel;
    QLabel* m_validationLabel;
    QLabel* m_validationSummaryLabel;
    QTreeWidget* m_treeWidget;
    QPlainTextEdit* m_detailEdit;
    QPlainTextEdit* m_validationEdit;
    QPushButton* m_loadDirectoryButton;
    QPushButton* m_addCertificateButton;
    QPushButton* m_validateCertificateButton;
    QPushButton* m_clearButton;
    core::CertificateAnalysisResult m_lastResult;
    core::CertificateValidationResult m_lastValidationResult;
};

#endif
