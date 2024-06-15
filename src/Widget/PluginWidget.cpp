#include <QtCore/QProcess>
#include <QtGui/QIcon>
#include <QtWidgets/QMessageBox>

#include "PluginWidget.h"

struct PluginWidget::PluginWidgetPrivate {
    std::unordered_map<std::string, PluginConfig> m_PluginConfigMap;
};

PluginWidget::PluginWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PluginWidgetClass())
    , m_impl(std::make_shared<PluginWidgetPrivate>())
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
    ui->treeWidget->setColumnWidth(0, 300);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &PluginWidget::SlotLineEditFilter);
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &PluginWidget::SlotTreeItemChanged);
    connect(ui->pushButton_ok, &QPushButton::clicked, this, &PluginWidget::SlotPushButtonOk);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, [&]() {
        hide();
    });
}

void PluginWidget::SlotShowTree(const std::unordered_map<std::string, PluginConfig>& pluginConfigMap)
{
    m_impl->m_PluginConfigMap = pluginConfigMap;
    for (auto iter : m_impl->m_PluginConfigMap) {
        auto    pluginConfig = iter.second;
        QString group        = QString::fromStdString(pluginConfig.group);
        bool    load         = pluginConfig.load;
        QString name         = QString::fromStdString(pluginConfig.name);
        bool    isLoad       = pluginConfig.isLoad;
        QString error        = QString::fromStdString(pluginConfig.error);
        QString version      = QString::fromStdString(pluginConfig.version);
        QString description  = QString::fromStdString(pluginConfig.description);

        // 创建分组节点
        QTreeWidgetItem* topItem = this->SlotFindTopLevelNode(ui->treeWidget, group);
        if (nullptr == topItem) {
            topItem = new QTreeWidgetItem;
            topItem->setText(0, group);
            topItem->setCheckState(1, Qt::Unchecked);
            ui->treeWidget->addTopLevelItem(topItem);
        }

        /**
         * 创建插件节点
         * 是否加载和能否加载两字节
         */
        QTreeWidgetItem* childItem = new QTreeWidgetItem;
        if (load && isLoad) {
            childItem->setText(0, name);
            childItem->setCheckState(1, Qt::Checked);
            childItem->setData(1, Qt::UserRole, 3);
            childItem->setText(2, version);
            childItem->setText(3, description);
            childItem->setIcon(3, QIcon(":/icons/correct.png"));
        } else if (!load && isLoad) {
            childItem->setText(0, name);
            childItem->setCheckState(1, Qt::Unchecked);
            childItem->setData(1, Qt::UserRole, 2);
            childItem->setText(2, version);
            childItem->setText(3, description);
            childItem->setIcon(3, QIcon(":/icons/ready.png"));
        } else {
            childItem->setText(0, name);
            childItem->setCheckState(1, Qt::Unchecked);
            childItem->setData(1, Qt::UserRole, load ? 1 : 0);
            childItem->setText(2, "");
            childItem->setText(3, error);
            childItem->setIcon(3, QIcon(":/icons/error.png"));
            childItem->setFlags(childItem->flags() & ~Qt::ItemIsUserCheckable);
        }
        topItem->addChild(childItem);
    }
    ui->treeWidget->expandAll();
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

void PluginWidget::SlotTreeItemChanged(QTreeWidgetItem* item, int column)
{
    int child_cnt = item->childCount();
    int state_col = 1;
    if (item->checkState(state_col) == Qt::Checked) {
        if (child_cnt > 0) {
            for (int i = 0; i < child_cnt; i++) {
                if (2 > item->child(i)->data(1, Qt::UserRole).toInt()) {
                    continue;
                }
                item->child(i)->setCheckState(state_col, Qt::Checked);
            }
        } else {
            this->SlotUpdateParentItem(item);
        }
        this->SlotUpdatePluginConfig(item->text(0), true);
    } else if (item->checkState(state_col) == Qt::Unchecked) {
        if (child_cnt > 0) {
            for (int i = 0; i < child_cnt; i++) {
                if (2 > item->child(i)->data(1, Qt::UserRole).toInt()) {
                    continue;
                }
                item->child(i)->setCheckState(state_col, Qt::Unchecked);
            }
        } else {
            this->SlotUpdateParentItem(item);
        }
        this->SlotUpdatePluginConfig(item->text(0), false);
    }
}
void PluginWidget::SlotUpdateParentItem(QTreeWidgetItem* item)
{
    QTreeWidgetItem* parent = item->parent();
    if (nullptr == parent) {
        return;
    }
    // 选中的子节点个数
    int selectedCount = 0;
    int child_cnt     = parent->childCount();
    int state_col     = 1;
    for (int i = 0; i < child_cnt; i++) {
        QTreeWidgetItem* childItem = parent->child(i);
        if (childItem->checkState(state_col) == Qt::Checked) {
            selectedCount++;
        }
    }
    if (selectedCount == 0) {
        // 未选中状态
        parent->setCheckState(state_col, Qt::Unchecked);
    } else if (selectedCount > 0 && selectedCount < child_cnt) {
        // 部分选中状态
        parent->setCheckState(state_col, Qt::PartiallyChecked);
    } else if (selectedCount == child_cnt) {
        // 选中状态
        parent->setCheckState(state_col, Qt::Checked);
    }
}

void PluginWidget::SlotUpdatePluginConfig(const QString& name, bool load)
{
    for (auto& iter : m_impl->m_PluginConfigMap) {
        PluginConfig& pluginConfig = iter.second;
        if (name.toStdString() == pluginConfig.name) {
            pluginConfig.load = load;
            return;
        }
    }
}

int PluginWidget::SlotGetPluginConfigLoad(const QString& name)
{
    for (auto& iter : m_impl->m_PluginConfigMap) {
        PluginConfig& pluginConfig = iter.second;
        if (name.toStdString() == pluginConfig.name) {
            int load   = pluginConfig.load ? 1 << 0 : 0;
            int isLoad = pluginConfig.isLoad ? 1 << 1 : 0;
            return load | isLoad;
        }
    }
    return 0;
}

void PluginWidget::SlotPushButtonOk()
{
    int top_cnt = ui->treeWidget->topLevelItemCount();
    for (int i = 0; i < top_cnt; ++i) {
        auto top_item  = ui->treeWidget->topLevelItem(i);
        int  child_cnt = top_item->childCount();
        for (int j = 0; j < child_cnt; ++j) {
            int old_state = top_item->child(j)->data(1, Qt::UserRole).toInt();
            int new_state = SlotGetPluginConfigLoad(top_item->child(j)->text(0));
            if (old_state != new_state) {
                int ret = QMessageBox::information(this, "系统提示", "插件已修改，是否保存重启", QMessageBox::Yes | QMessageBox::No);
                if (QMessageBox::Yes == ret) {
                    emit SignalUpdatePluginConfigVec(m_impl->m_PluginConfigMap);
                    // 重启应用程序
                    qApp->quit();
                    QProcess::startDetached(qApp->applicationFilePath(), QStringList());
                } else {
                    return;
                }
            }
        }
    }
    close();
}
