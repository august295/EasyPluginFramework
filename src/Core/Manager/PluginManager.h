#ifndef __PLUGINMANAGER_H__
#define __PLUGINMANAGER_H__

#include <string>
#include <unordered_map>
#include <vector>

#include <common/IPlugin.hpp>
#include <common/Library.hpp>
#include <common/Utils.hpp>

#include "GlobalManager.hpp"

struct PluginConfig {
    std::string    group;       // 分组
    bool           load;        // 是否加载
    std::string    name;        // 动态库名称
    bool           isLoad;      // 加载情况
    std::string    error;       // 加载错误信息
    LIB_HANDLE     handle;      // 动态库句柄
    IPlugin*       plugin;      // 插件指针
    std::string    version;     // 版本
    std::string    description; // 描述
    PluginLocation location;    // 显示位置
};

/**
 * @brief 插件管理
 */
class MANAGER_API PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    /**
     * @brief 设置加载插件配置文件路径
     * @param filepath     配置文件
     */
    void SetPluginConfigFile(const std::string& filepath);

    /**
     * @brief 读取插件配置文件
     */
    bool ReadPluginConfig();

    /**
     * @brief 写入插件配置文件
     * @param pluginConfigVec 插件配置信息
     */
    bool WritePluginConfig(const std::unordered_map<std::string, PluginConfig>& pluginConfigMap);

    /**
     * @brief 加载单个插件
     * @param pluginConfig 插件配置信息
     */
    bool LoadPluginOne(PluginConfig& pluginConfig);

    /**
     * @brief 加载所有插件
     */
    void LoadPluginAll();

    /**
     * @brief 卸载单个插件
     * @param pluginConfig 插件配置信息
     */
    bool UnloadPluginOne(PluginConfig& pluginConfig);

    /**
     * @brief 卸载所有插件
     */
    void UnloadPluginAll();

    /**
     * @brief 获取插件配置信息
     */
    std::unordered_map<std::string, PluginConfig> GetPluginConfigMap();

private:
    struct PluginManagerPrivate;
    std::shared_ptr<PluginManagerPrivate> m_impl;
};

#endif
