#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include <QtWidgets/QApplication>

#include "IFramework.hpp"
#include "IPlugin.hpp"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    // ParseHeaderWidget w;
    // w.show();

    QString        dir  = QApplication::applicationDirPath();
    QString        file = dir + "/Testd";
    QPluginLoader* load = new QPluginLoader(file);
    if (load->load()) {
        QObject* obj    = load->instance();
        IPlugin* plugin = qobject_cast<IPlugin*>(obj);
        plugin->Init();
    } else {
        QString error = load->errorString();
    }

    return a.exec();
}
