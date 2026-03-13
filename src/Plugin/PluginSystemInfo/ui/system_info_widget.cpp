#include "system_info_widget.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
QTreeWidgetItem* createCategoryItem(QTreeWidget* treeWidget, const QString& categoryName)
{
    auto* item = new QTreeWidgetItem(treeWidget);
    item->setText(0, categoryName);
    item->setFirstColumnSpanned(true);
    item->setExpanded(true);
    return item;
}
}

SystemInfoWidget::SystemInfoWidget(QWidget* parent)
    : QWidget(parent)
    , m_refreshButton(nullptr)
    , m_treeWidget(nullptr)
{
    createLayout();
}

void SystemInfoWidget::displaySnapshot(const core::SystemInfoSnapshot& snapshot)
{
    m_treeWidget->clear();
    QTreeWidgetItem* deviceItem = createCategoryItem(m_treeWidget, tr("设备规格"));
    populateCategory(deviceItem, snapshot.deviceEntries);
    QTreeWidgetItem* windowsItem = createCategoryItem(m_treeWidget, tr("Windows 规格"));
    populateCategory(windowsItem, snapshot.windowsEntries);
}

void SystemInfoWidget::createLayout()
{
    auto* rootLayout = new QVBoxLayout(this);
    auto* toolLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton(tr("刷新"), this);
    toolLayout->addStretch();
    toolLayout->addWidget(m_refreshButton);
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setColumnCount(2);
    m_treeWidget->setHeaderLabels({tr("项目"), tr("内容")});
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    rootLayout->addLayout(toolLayout);
    rootLayout->addWidget(m_treeWidget);
    connect(m_refreshButton, &QPushButton::clicked, this, &SystemInfoWidget::refreshRequested);
}

void SystemInfoWidget::populateCategory(QTreeWidgetItem* categoryItem, const std::vector<core::SystemInfoEntry>& entries) const
{
    for (const core::SystemInfoEntry& entry : entries)
    {
        auto* childItem = new QTreeWidgetItem(categoryItem);
        childItem->setText(0, QString::fromUtf8(entry.label.c_str()));
        childItem->setText(1, QString::fromUtf8(entry.value.c_str()));
    }
}
