#include <pugixml/pugixml.hpp>
#include <spdlog/spdlog.h>

#include <IPlugin.hpp>
#include <Library.hpp>

#include "ConfigManager.h"
#include "PluginManager.h"

struct PluginConfig {
    bool        load;   // 是否加载
    std::string name;   // 动态库名称
    bool        isLoad; // 加载情况
    std::string error;  // 加载错误信息
    LIB_HANDLE  handle; // 动态库句柄
};

struct PluginManager::PluginManagerPrivate {
    std::vector<PluginConfig>       m_PluginConfigVec; // 插件配置信息
    std::map<std::string, IPlugin*> m_IPluginMap;      // 插件映射关系
};

PluginManager::PluginManager()
    : m_P(new PluginManagerPrivate)
{
}

PluginManager::~PluginManager()
{
}

bool PluginManager::ReadPluginConfig(std::string filename)
{
    if (filename.empty()) {
        filename = ConfigManager::instance()->GetBinPath() + "/../../resources/Plugins.xml";
    }
    pugi::xml_document     doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full, pugi::encoding_utf8);
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

bool PluginManager::LoadPluginOne(PluginConfig& pluginConfig)
{
    std::string useName = pluginConfig.name;
#if (defined _DEBUG)
    useName += "d";
#endif
    // 加载动态库
    std::string file   = ConfigManager::instance()->GetBinPath() + "/" + useName;
    LIB_HANDLE  handle = handle = LIB_LOAD(file.c_str());
    if (!handle) {
        std::string error   = LIB_ERROR();
        pluginConfig.isLoad = false;
        pluginConfig.error  = error;
        SPDLOG_ERROR("Failed to load {}: {}", file, error);
        return false;
    }

    // 加载函数
    typedef IPlugin* (*CreatePluginFunc)();
    CreatePluginFunc CreatePlugin = LoadFunction<CreatePluginFunc>(handle, "CreatePlugin");
    if (!CreatePlugin) {
        std::string error   = "CreatePlugin does not exist or has the wrong type";
        pluginConfig.isLoad = false;
        pluginConfig.error  = error;
        SPDLOG_ERROR(error);
        LIB_UNLOAD(handle);
        return false;
    }
    pluginConfig.handle = handle;

    // 调用导出函数创建对象，并进行初始化
    IPlugin* plugin = CreatePlugin();
    plugin->Init();
    m_P->m_IPluginMap.emplace(pluginConfig.name, plugin);
    return true;
}

void PluginManager::LoadPluginAll()
{
    for (auto& pluginConfig : m_P->m_PluginConfigVec) {
        if (pluginConfig.load) {
            LoadPluginOne(pluginConfig);
        }
    }
}

bool PluginManager::UnloadPluginOne(PluginConfig& pluginConfig)
{
    IPlugin* plugin = m_P->m_IPluginMap[pluginConfig.name];
    plugin->Release();
    LIB_UNLOAD(pluginConfig.handle);
    m_P->m_IPluginMap.erase(pluginConfig.name);
    return false;
}

void PluginManager::UnloadPluginAll()
{
    for (auto& pluginConfig : m_P->m_PluginConfigVec) {
        if (pluginConfig.isLoad) {
            UnloadPluginOne(pluginConfig);
        }
    }
}
