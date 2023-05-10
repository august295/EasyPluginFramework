#include <QtCore/QDir>
#include <QtWidgets/QApplication>

#include <Manager/DataManager.h>
#include <Manager/Framework.h>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    // ParseHeaderWidget w;
    // w.show();

    QString binPath = QApplication::applicationDirPath();
    DataManager::instance().SetBinPath(binPath);

    Framework framework;
    auto      pluginManager = framework.GetPluginManager();
	pluginManager->ReadPluginConfig();
	pluginManager->LoadPluginAll();

    return a.exec();
}
