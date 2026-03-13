#ifndef __SYSTEM_INFO_WIDGET_H__
#define __SYSTEM_INFO_WIDGET_H__

#include <QWidget>

#include "core/system_info_types.h"

class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * @brief 系统信息展示界面。
 */
class SystemInfoWidget
    : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造系统信息界面。
     * @param parent 父窗口。
     */
    explicit SystemInfoWidget(QWidget* parent = nullptr);

    /**
     * @brief 展示系统信息快照。
     * @param snapshot 系统信息快照。
     */
    void displaySnapshot(const core::SystemInfoSnapshot& snapshot);

signals:
    void refreshRequested();

private:
    void createLayout();
    void populateCategory(QTreeWidgetItem* categoryItem, const std::vector<core::SystemInfoEntry>& entries) const;

private:
    QPushButton* m_refreshButton;
    QTreeWidget* m_treeWidget;
};

#endif
