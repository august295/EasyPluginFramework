#pragma once

#include <QWidget>

#include <Manager/PluginManager.h>

#include "ui_PluginWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class PluginWidgetClass;
};
QT_END_NAMESPACE

class PluginWidget : public QWidget {
    Q_OBJECT

public:
    PluginWidget(QWidget* parent = nullptr);
    ~PluginWidget();

    /**
     * @brief 初始化
     */
    void Init();

public slots:
    /**
     * @brief 显示插件加载情况
     * @param pluginConfigVec 插件配置数组
     */
    void SlotShowTree(const std::vector<PluginConfig>& pluginConfigVec);

    /**
     * @brief 查找分组节点
     * @param treeWidget   树
     * @param name         分组名称
     * @return QTreeWidgetItem* 分组节点（nullptr表示没有）
     */
    QTreeWidgetItem* SlotFindTopLevelNode(QTreeWidget* treeWidget, const QString& name);

    /**
     * @brief 插件筛选
     * @param filter       筛选条件
     */
    void SlotLineEditFilter(const QString& filter);

private:
    Ui::PluginWidgetClass* ui;
};
