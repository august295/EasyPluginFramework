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
    auto pluginManager      = m_impl->m_FrameworkSptr->GetPluginManager();
    pluginManager->ReadPluginConfig();
    pluginManager->LoadPluginAll();
    m_impl->m_PluginWidgetSptr = std::make_shared<PluginWidget>(this);
    m_impl->m_PluginWidgetSptr->SlotShowTree(pluginManager->GetPluginConfigVec());
}

void MainWindow::InitMenuBar()
{
    connect(ui->action_Plugin, &QAction::triggered, this, [&]() {
        m_impl->m_PluginWidgetSptr->show();
    });
}
