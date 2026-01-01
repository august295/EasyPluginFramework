#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#include "LoggerManager.h"
#include "PluginManager.h"

struct PluginManager::PluginManagerPrivate
{
    std::string                                   m_PluginConfigFile; // 配置文件
    std::unordered_map<std::string, PluginConfig> m_PluginConfigMap;  // 插件映射关系
};

PluginManager::PluginManager()
    : m_impl(std::make_shared<PluginManagerPrivate>())
{
    m_impl->m_PluginConfigFile = "configs/plugins.xml";
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
    if (pugi::status_ok != result.status)
    {
        EPF_LOG_ERROR(result.description());
        return false;
    }

    pugi::xml_node root = doc.child("Plugins");
    for (pugi::xml_node groupNode = root.first_child(); groupNode; groupNode = groupNode.next_sibling())
    {
        for (pugi::xml_node pluginNode = groupNode.first_child(); pluginNode; pluginNode = pluginNode.next_sibling())
        {
            PluginConfig pluginConfig;
            pluginConfig.group = groupNode.name();
            pluginConfig.name  = pluginNode.name();
            pluginConfig.load  = pluginNode.attribute("load").as_bool();
            m_impl->m_PluginConfigMap.emplace(pluginConfig.name, pluginConfig);
        }
    }
    return true;
}

bool PluginManager::WritePluginConfig(const std::unordered_map<std::string, PluginConfig>& pluginConfigMap)
{
    pugi::xml_document     doc;
    pugi::xml_parse_result result = doc.load_file(m_impl->m_PluginConfigFile.c_str(), pugi::parse_full, pugi::encoding_utf8);
    if (pugi::status_ok != result.status)
    {
        EPF_LOG_ERROR(result.description());
        return false;
    }

    for (const auto& iter : pluginConfigMap)
    {
        PluginConfig     pluginConfig = iter.second;
        std::string      node_path    = std::string("/Plugins/") + pluginConfig.group + "/" + pluginConfig.name;
        pugi::xpath_node node         = doc.select_node(pugi::xpath_query(node_path.c_str()));
        if (nullptr != node)
        {
            node.node().attribute("load").set_value(pluginConfig.load);
        }
    }
    doc.save_file(m_impl->m_PluginConfigFile.c_str(), "\t", 1U, pugi::encoding_utf8);
    return true;
}

bool PluginManager::LoadPluginOne(PluginConfig& pluginConfig)
{
    std::string useName = GetSharedName(pluginConfig.name);
    // 加载动态库
    std::string file   = useName;
    LIB_HANDLE  handle = LIB_LOAD(file.c_str());
    if (!handle)
    {
        std::string error   = LIB_ERROR();
        pluginConfig.isLoad = false;
#if defined(_WIN32) || defined(_WIN64)
        error = gbk_to_utf8(RemoveCRLF(error));
#endif
        pluginConfig.error = error;
        EPF_LOG_ERROR("{} Load Failed: {}", file, pluginConfig.error);
        return false;
    }

    // 加载函数
    typedef IPlugin* (*CreatePluginFunc)();
    std::string      FuncName     = "CreatePlugin";
    CreatePluginFunc CreatePlugin = LoadFunction<CreatePluginFunc>(handle, FuncName.c_str());
    if (!CreatePlugin)
    {
        std::string error   = FuncName + " function does not exist";
        pluginConfig.isLoad = false;
        pluginConfig.error  = error;
        EPF_LOG_ERROR(error);
        LIB_UNLOAD(handle);
        return false;
    }
    pluginConfig.isLoad = true;
    pluginConfig.handle = handle;

    // 调用导出函数创建对象，并进行初始化
    IPlugin* plugin          = CreatePlugin();
    pluginConfig.plugin      = plugin;
    pluginConfig.version     = plugin->Version();
    pluginConfig.description = plugin->Description();
    pluginConfig.location    = plugin->Location();

    if (pluginConfig.load)
    {
        plugin->Init();
        plugin->InitAppFinish();
        EPF_LOG_INFO("{} Load Success", file);
    }
    else
    {
        plugin->Release();
        EPF_LOG_INFO("{} Not Load", file);
    }
    return true;
}

void PluginManager::LoadPluginAll()
{
    for (auto& iter : m_impl->m_PluginConfigMap)
    {
        PluginConfig& pluginConfig = iter.second;
        LoadPluginOne(pluginConfig);
    }
}

bool PluginManager::UnloadPluginOne(PluginConfig& pluginConfig)
{
    IPlugin* plugin = m_impl->m_PluginConfigMap[pluginConfig.name].plugin;
    plugin->Release();
    LIB_UNLOAD(pluginConfig.handle);
    m_impl->m_PluginConfigMap.erase(pluginConfig.name);
    return true;
}

void PluginManager::UnloadPluginAll()
{
    for (auto& iter : m_impl->m_PluginConfigMap)
    {
        PluginConfig& pluginConfig = iter.second;
        if (pluginConfig.isLoad)
        {
            UnloadPluginOne(pluginConfig);
        }
    }
}

std::unordered_map<std::string, PluginConfig> PluginManager::GetPluginConfigMap()
{
    return m_impl->m_PluginConfigMap;
}
