#include <QApplication>

#include "MainWindow.h"
#include "Subscribe.h"
#include "Manager/LoggerManager.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

int main(int argc, char* argv[])
{
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // 单例
    Subscribe::instance();
    LoggerManager::instance();

    // 界面
    QApplication a(argc, argv);
    MainWindow   w;
    w.show();
    return a.exec();
}
