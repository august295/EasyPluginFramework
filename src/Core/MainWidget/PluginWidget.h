#pragma once

#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QWidget>

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

    /**
     * @brief QTreeItem 内容改变触发
     * @param item      当前节点
     * @param column    节点列
     */
    void SlotTreeItemChanged(QTreeWidgetItem* item, int column);

    /**
     * @brief 更新父节点 check 选中状态
     * @param item      子节点
     */
    void SlotUpdateParentItem(QTreeWidgetItem* item);

    /**
     * @brief 更新插件配置
     * @param name      插件名称
     * @param load      是否加载
     */
    void SlotUpdatePluginConfig(const QString& name, bool load);

    /**
     * @brief 获取插件加载情况
     * @param name      插件名称
     * @return int      插件加载情况
     */
    int SlotGetPluginConfigLoad(const QString& name);

    /**
     * @brief 点击确定
     */
    void SlotPushButtonOk();

signals:
    /**
     * @brief 更新插件配置
     */
    void SignalUpdatePluginConfigVec(const std::vector<PluginConfig>& pluginConfigVec);

private:
    Ui::PluginWidgetClass* ui;

    struct PluginWidgetPrivate;
    std::shared_ptr<PluginWidgetPrivate> m_impl;
};
