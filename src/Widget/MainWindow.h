#pragma once

#include <QMainWindow>

#include "ui_MainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindowClass;
};
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    /**
     * @brief 初始化
     */
    void Init();
    void InitFramework();
    void InitMenuBar();
	void InitDockWidget();

public slots:

private:
    Ui::MainWindowClass* ui;

    struct MainWindowPrivate;
    MainWindowPrivate* m_impl;
};
