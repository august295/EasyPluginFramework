#include <QtCore/QDir>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QTextEdit>

#include "MainWindow.h"
#include "PluginWidget.h"
#include "Subscribe.h"
#include "CertView/CertView.h"

struct MainWindow::MainWindowPrivate
{
    std::shared_ptr<PluginManager> m_PluginManager;     // 插件管理
    std::shared_ptr<PluginWidget>  m_PluginWidgetSptr;  // 插件界面
    QDockWidget*                   m_DockWidgetConsole; // 控制台输出
    QMap<QString, QWidget*>        m_MenuWidgetMap;     //
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
    {
        delete m_impl->m_DockWidgetConsole;
        delete m_impl;
    }
    delete ui;
}

void MainWindow::Init()
{
    setWindowTitle(tr("插件框架"));
    setWindowIcon(QIcon(":/icons/framework.png"));
    showMaximized();

    this->InitDockWidget();
    this->InitFramework();
    this->InitMenuBar();
    this->InitMenuBarPlugin();
}

void MainWindow::InitFramework()
{
    m_impl->m_PluginManager = std::make_shared<PluginManager>();
    m_impl->m_PluginManager->ReadPluginConfig();
    m_impl->m_PluginManager->LoadPluginAll();
}

void MainWindow::InitMenuBar()
{
    // openssl 证书查看器
    CertView* certView = new CertView();
    AddMenuAction("工具", "证书", "openssl证书查看器", certView);
    m_impl->m_MenuWidgetMap.insert("opensslCertView", certView);
}

void MainWindow::AddMenuAction(const QString& page, const QString& group, const QString& name, QWidget* widget)
{
    QMenu* page_menu = ui->menuBar->findChild<QMenu*>(page, Qt::FindDirectChildrenOnly);
    if (!page_menu)
    {
        page_menu = new QMenu(page, ui->menuBar);
        page_menu->setObjectName(page);
        ui->menuBar->addMenu(page_menu);
    }
    QMenu* group_menu = page_menu->findChild<QMenu*>(group, Qt::FindDirectChildrenOnly);
    if (!group_menu)
    {
        group_menu = new QMenu(group, page_menu);
        group_menu->setObjectName(group);
        page_menu->addMenu(group_menu);
    }
    QAction* action = new QAction(name);
    group_menu->addAction(action);
    connect(action, &QAction::triggered, this, [=]() {
        widget->show();
    });
}

void MainWindow::InitMenuBarPlugin()
{
    auto pluginConfigMap = m_impl->m_PluginManager->GetPluginConfigMap();
    // 插件加载情况界面
    m_impl->m_PluginWidgetSptr = std::make_shared<PluginWidget>(this);
    m_impl->m_PluginWidgetSptr->SlotShowTree(pluginConfigMap);
    connect(m_impl->m_PluginWidgetSptr.get(), &PluginWidget::SignalUpdatePluginConfigVec, this, [&](const std::unordered_map<std::string, PluginConfig>& pluginConfigMap) {
        m_impl->m_PluginManager->WritePluginConfig(pluginConfigMap);
    });
    connect(ui->action_Plugin, &QAction::triggered, this, [&]() {
        m_impl->m_PluginWidgetSptr->show();
    });

    // 显示插件
    for (const auto& iter : pluginConfigMap)
    {
        PluginConfig   pluginConfig = iter.second;
        PluginLocation location     = pluginConfig.location;
        if (pluginConfig.load && PluginType::WIDGET == location.m_type)
        {
            QString page      = QString::fromStdString(location.m_page);
            QMenu*  page_menu = ui->menuBar->findChild<QMenu*>(page, Qt::FindDirectChildrenOnly);
            if (!page_menu)
            {
                page_menu = new QMenu(page, ui->menuBar);
                page_menu->setObjectName(page);
                ui->menuBar->addMenu(page_menu);
            }
            QString group      = QString::fromStdString(location.m_group);
            QMenu*  group_menu = page_menu->findChild<QMenu*>(group, Qt::FindDirectChildrenOnly);
            if (!group_menu)
            {
                group_menu = new QMenu(group, page_menu);
                group_menu->setObjectName(group);
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

void MainWindow::InitDockWidget()
{
    // 悬浮窗
    m_impl->m_DockWidgetConsole = new QDockWidget(this);
    m_impl->m_DockWidgetConsole->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_impl->m_DockWidgetConsole->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_impl->m_DockWidgetConsole->setWindowTitle(tr("输出"));
    this->addDockWidget(Qt::BottomDockWidgetArea, m_impl->m_DockWidgetConsole);

    // 文本编辑器
    QTextEdit* edit = new QTextEdit;
    m_impl->m_DockWidgetConsole->setWidget(edit);

    // 反馈信息
    connect(&Subscribe::instance(), &Subscribe::signal_log, this, [=](const QString& text) {
        edit->append(text);
    });
}
