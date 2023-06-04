#include "PluginWidget.h"

PluginWidget::PluginWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PluginWidgetClass())
{
    ui->setupUi(this);

    this->Init();
}

PluginWidget::~PluginWidget()
{
    delete ui;
}

void PluginWidget::Init()
{
    resize(qobject_cast<QWidget*>(parent())->size() / 2);
    // 设置界面属性
    setWindowFlags(Qt::Window);
    // 设置模态
    setWindowModality(Qt::NonModal);

    QStringList header = {"名称", "加载", "版本", "描述"};
    ui->treeWidget->setHeaderLabels(header);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &PluginWidget::SlotLineEditFilter);
}

void PluginWidget::SlotShowTree(const std::vector<PluginConfig>& pluginConfigVec)
{
    for (auto pluginConfig : pluginConfigVec) {
        QString group       = QString::fromStdString(pluginConfig.group);
        bool    load        = pluginConfig.load;
        QString name        = QString::fromStdString(pluginConfig.name);
        bool    isLoad      = pluginConfig.isLoad;
        QString error       = QString::fromStdString(pluginConfig.error);
        QString version     = QString::fromStdString(pluginConfig.version);
        QString description = QString::fromStdString(pluginConfig.description);

        // 创建分组节点
        QTreeWidgetItem* topItem = this->SlotFindTopLevelNode(ui->treeWidget, group);
        if (nullptr == topItem) {
            topItem = new QTreeWidgetItem;
            topItem->setText(0, group);
            topItem->setCheckState(1, Qt::Unchecked);
            ui->treeWidget->addTopLevelItem(topItem);
        }

        // 创建插件节点
        QTreeWidgetItem* childItem = new QTreeWidgetItem;
        childItem->setText(0, name);
        childItem->setCheckState(1, isLoad ? Qt::Checked : Qt::Unchecked);

        if (isLoad) {
            childItem->setText(2, version);
            childItem->setText(3, description);
        } else {
            childItem->setText(2, "");
            childItem->setText(3, error);
        }
        topItem->addChild(childItem);
    }
}

QTreeWidgetItem* PluginWidget::SlotFindTopLevelNode(QTreeWidget* treeWidget, const QString& name)
{
    QList<QTreeWidgetItem*> items = treeWidget->findItems(name, Qt::MatchExactly, 0);
    if (!items.isEmpty()) {
        QTreeWidgetItem* item = items.first();
        if (!item->parent()) {
            return item;
        }
    }
    return nullptr;
}

void PluginWidget::SlotLineEditFilter(const QString& filter)
{
    for (int topIndex = 0; topIndex < ui->treeWidget->topLevelItemCount(); ++topIndex) {
        bool             allHide = true;
        QTreeWidgetItem* topItem = ui->treeWidget->topLevelItem(topIndex);
        for (int childIndex = 0; childIndex < topItem->childCount(); ++childIndex) {
            QTreeWidgetItem* childItem = topItem->child(childIndex);
            if (childItem->text(0).contains(filter)) {
                childItem->setHidden(false);
                allHide = false;
            } else {
                childItem->setHidden(true);
            }
        }
        topItem->setHidden(allHide);
    }
}
