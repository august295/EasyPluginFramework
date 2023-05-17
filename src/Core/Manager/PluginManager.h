#ifndef __PLUGINMANAGER_H__
#define __PLUGINMANAGER_H__

#include <map>
#include <string>
#include <vector>

#include "GlobalManager.hpp"

struct PluginConfig {
    bool        load;   // 是否加载
    std::string name;   // 动态库名称
    bool        isLoad; // 加载情况
    std::string error;  // 加载错误信息
};

/**
 * @brief 插件管理
 */
class MANAGER_API PluginManager {
public:
    PluginManager();
    ~PluginManager();

    /**
     * @brief 读取插件配置文件
     * @param filename     配置文件路径
     */
    bool ReadPluginConfig(std::string filename = "");

    /**
     * @brief 加载单个插件
     * @param pluginConfig 插件配置信息
     */
    bool LoadPluginOne(PluginConfig& pluginConfig);

    /**
     * @brief 加载所有插件
     */
    void LoadPluginAll();

private:
    struct PluginManagerPrivate;
    PluginManagerPrivate* m_P;
};

#endif