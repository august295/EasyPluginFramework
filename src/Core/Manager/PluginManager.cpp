#include <pugixml/pugixml.hpp>
#include <spdlog/spdlog.h>

#include <IPlugin.hpp>

#include "DataManager.h"
#include "PluginManager.h"

struct PluginManager::PluginManagerPrivate {
    QVector<PluginConfig>   m_PluginConfigVec; // 插件配置信息
    QMap<QString, IPlugin*> m_IPluginMap;      // 插件映射关系
};

PluginManager::PluginManager()
    : m_P(new PluginManagerPrivate)
{
}

PluginManager::~PluginManager()
{
}

bool PluginManager::ReadPluginConfig(QString filename)
{
    if (filename.isEmpty()) {
        filename = DataManager::instance().GetBinPath() + "/../../resources/Plugins.xml";
    }
    pugi::xml_document     doc;
    pugi::xml_parse_result result = doc.load_file(filename.toUtf8().data(), pugi::parse_full, pugi::encoding_utf8);
    if (pugi::status_ok != result.status) {
        SPDLOG_ERROR(result.description());
        return false;
    }

    pugi::xml_node root = doc.child("Plugins");
    for (pugi::xml_node pluginNode = root.first_child(); pluginNode; pluginNode = pluginNode.next_sibling()) {
        PluginConfig pluginConfig;
        pluginConfig.name = pluginNode.attribute("name").as_string();
        pluginConfig.load = pluginNode.attribute("load").as_bool();
        m_P->m_PluginConfigVec.push_back(pluginConfig);
    }
    return false;
}

bool PluginManager::LoadPluginOne(QString pluginName)
{
#if (defined _DEBUG)
    pluginName += "d";
#endif
    QString        file = DataManager::instance().GetBinPath() + "/" + pluginName;
    QPluginLoader* load = new QPluginLoader(file);
    if (load->load()) {
        QObject* obj    = load->instance();
        IPlugin* plugin = qobject_cast<IPlugin*>(obj);
		m_P->m_IPluginMap.insert(pluginName, plugin);
        plugin->Init();
    } else {
        SPDLOG_ERROR(load->errorString().toUtf8().data());
        return false;
    }
    return true;
}

void PluginManager::LoadPluginAll()
{
    for (auto pluginConfig : m_P->m_PluginConfigVec) {
        if (pluginConfig.load) {
            LoadPluginOne(pluginConfig.name); 
        }
    }
}
