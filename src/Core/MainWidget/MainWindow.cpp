#include <QtCore/QDir>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

#include <Manager/ConfigManager.h>
#include <Manager/DataManager.h>
#include <Manager/Framework.h>

#include "MainWindow.h"
#include "PluginWidget.h"

struct MainWindow::MainWindowPrivate {
    std::shared_ptr<Framework>    m_FrameworkSptr;    // 框架
    std::shared_ptr<PluginWidget> m_PluginWidgetSptr; // 插件界面
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass())
    , m_impl(new MainWindowPrivate)
{
    ui->setupUi(this);

    this->Init();
}

MainWindow::~MainWindow()
{
    delete m_impl;
    delete ui;
}

void MainWindow::Init()
{
    // 获取屏幕信息
    QDesktopWidget* desktop = QApplication::desktop();
    resize(desktop->availableGeometry().size());
    showMaximized();

    this->InitFramework();
    this->InitMenuBar();
}

void MainWindow::InitFramework()
{
    QString binPath = QApplication::applicationDirPath();
    ConfigManager::instance()->SetBinPath(binPath.toStdString());

    m_impl->m_FrameworkSptr = std::make_shared<Framework>();
    m_impl->m_FrameworkSptr->GetPluginManager()->ReadPluginConfig();
    m_impl->m_FrameworkSptr->GetPluginManager()->LoadPluginAll();
}

void MainWindow::InitMenuBar()
{
    auto pluginConfigMap = m_impl->m_FrameworkSptr->GetPluginManager()->GetPluginConfigMap();
    // 插件加载情况界面
    m_impl->m_PluginWidgetSptr = std::make_shared<PluginWidget>(this);
    m_impl->m_PluginWidgetSptr->SlotShowTree(pluginConfigMap);
    connect(m_impl->m_PluginWidgetSptr.get(), &PluginWidget::SignalUpdatePluginConfigVec, this, [&](const std::unordered_map<std::string, PluginConfig>& pluginConfigMap) {
        m_impl->m_FrameworkSptr->GetPluginManager()->WritePluginConfig(pluginConfigMap);
    });
    connect(ui->action_Plugin, &QAction::triggered, this, [&]() {
        m_impl->m_PluginWidgetSptr->show();
    });

    // 显示插件
    for (auto iter : pluginConfigMap) {
        PluginConfig   pluginConfig = iter.second;
        PluginLocation location     = pluginConfig.location;
        if (pluginConfig.load && PluginType::WIDGET == location.m_type) {
            QString page      = QString::fromStdString(location.m_page);
            QMenu*  page_menu = ui->menuBar->findChild<QMenu*>(page, Qt::FindDirectChildrenOnly);
            if (!page_menu) {
                page_menu = new QMenu(page);
				ui->menuBar->addMenu(page_menu);
            }
            QString group      = QString::fromStdString(location.m_group);
            QMenu*  group_menu = page_menu->findChild<QMenu*>(group, Qt::FindDirectChildrenOnly);
            if (!group_menu) {
                group_menu = new QMenu(group);
				page_menu->addMenu(group_menu);
            }
            QString  name   = QString::fromStdString(location.m_name);
            QAction* action = new QAction(name);
            group_menu->addAction(action);
            connect(action, &QAction::triggered, this, [=]() {
                pluginConfig.plugin->WidgetShow();
            });
        }
    }
}
