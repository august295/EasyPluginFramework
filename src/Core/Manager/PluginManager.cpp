#include <pugixml/pugixml.hpp>
#include <spdlog/spdlog.h>

#include "ConfigManager.h"
#include "LoggerManager.h"
#include "PluginManager.h"

struct PluginManager::PluginManagerPrivate {
    std::string                     m_PluginConfigFile; // 配置文件
    std::vector<PluginConfig>       m_PluginConfigVec;  // 插件配置信息
    std::map<std::string, IPlugin*> m_IPluginMap;       // 插件映射关系
};

PluginManager::PluginManager()
    : m_impl(std::make_shared<PluginManagerPrivate>())
{
    m_impl->m_PluginConfigFile = ConfigManager::instance()->GetBinPath() + "/../../resources/configs/plugins.xml";
}

PluginManager::~PluginManager()
{
}

void PluginManager::SetPluginConfigFile(const std::string& filepath)
{
    m_impl->m_PluginConfigFile = filepath;
}

bool PluginManager::ReadPluginConfig()
{
    pugi::xml_document     doc;
    pugi::xml_parse_result result = doc.load_file(m_impl->m_PluginConfigFile.c_str(), pugi::parse_full, pugi::encoding_utf8);
    if (pugi::status_ok != result.status) {
        LOG_ERROR(result.description());
        return false;
    }

    pugi::xml_node root = doc.child("Plugins");
    for (pugi::xml_node groupNode = root.first_child(); groupNode; groupNode = groupNode.next_sibling()) {
        for (pugi::xml_node pluginNode = groupNode.first_child(); pluginNode; pluginNode = pluginNode.next_sibling()) {
            PluginConfig pluginConfig;
            pluginConfig.group = groupNode.name();
            pluginConfig.name  = pluginNode.name();
            pluginConfig.load  = pluginNode.attribute("load").as_bool();
            m_impl->m_PluginConfigVec.push_back(pluginConfig);
        }
    }
    return true;
}

bool PluginManager::WritePluginConfig(const std::vector<PluginConfig>& pluginConfigVec)
{
    pugi::xml_document     doc;
    pugi::xml_parse_result result = doc.load_file(m_impl->m_PluginConfigFile.c_str(), pugi::parse_full, pugi::encoding_utf8);
    if (pugi::status_ok != result.status) {
        LOG_ERROR(result.description());
        return false;
    }

    for (auto plugin : pluginConfigVec) {
        std::string      node_path = std::string("/Plugins/") + plugin.group + "/" + plugin.name;
        pugi::xpath_node node      = doc.select_node(pugi::xpath_query(node_path.c_str()));
        if (nullptr != node) {
            node.node().attribute("load").set_value(plugin.load);
        }
    }
    doc.save_file(m_impl->m_PluginConfigFile.c_str(), "\t", 1U, pugi::encoding_utf8);
    return true;
}

bool PluginManager::LoadPluginOne(PluginConfig& pluginConfig)
{
    std::string useName = GetSharedName(pluginConfig.name);
    // 加载动态库
    std::string file   = ConfigManager::instance()->GetBinPath() + "/" + useName;
    LIB_HANDLE  handle = LIB_LOAD(file.c_str());
    if (!handle) {
        std::string error   = LIB_ERROR();
        pluginConfig.isLoad = false;
#if defined(_WIN32) || defined(_WIN64)
        error = gbk_to_utf8(RemoveCRLF(error));
#endif
        pluginConfig.error = error;
        LOG_ERROR("Failed to load {}: {}", file, pluginConfig.error);
        return false;
    }

    // 加载函数
    typedef IPlugin* (*CreatePluginFunc)();
    std::string      FuncName     = "CreatePlugin";
    CreatePluginFunc CreatePlugin = LoadFunction<CreatePluginFunc>(handle, FuncName.c_str());
    if (!CreatePlugin) {
        std::string error   = FuncName + " function does not exist";
        pluginConfig.isLoad = false;
        pluginConfig.error  = error;
        LOG_ERROR(error);
        LIB_UNLOAD(handle);
        return false;
    }
    pluginConfig.handle = handle;

    // 调用导出函数创建对象，并进行初始化
    IPlugin* plugin          = CreatePlugin();
    pluginConfig.version     = plugin->Version();
    pluginConfig.description = plugin->Description();
    if (pluginConfig.load) {
        plugin->Init();
        m_impl->m_IPluginMap.emplace(pluginConfig.name, plugin);
    } else {
        plugin->Release();
    }
    return true;
}

void PluginManager::LoadPluginAll()
{
    for (auto& pluginConfig : m_impl->m_PluginConfigVec) {
        LoadPluginOne(pluginConfig);
    }
}

bool PluginManager::UnloadPluginOne(PluginConfig& pluginConfig)
{
    IPlugin* plugin = m_impl->m_IPluginMap[pluginConfig.name];
    plugin->Release();
    LIB_UNLOAD(pluginConfig.handle);
    m_impl->m_IPluginMap.erase(pluginConfig.name);
    return true;
}

void PluginManager::UnloadPluginAll()
{
    for (auto& pluginConfig : m_impl->m_PluginConfigVec) {
        if (pluginConfig.isLoad) {
            UnloadPluginOne(pluginConfig);
        }
    }
}

std::vector<PluginConfig> PluginManager::GetPluginConfigVec()
{
    return m_impl->m_PluginConfigVec;
}
