#ifndef __PLUGINMANAGER_H__
#define __PLUGINMANAGER_H__

#include <QtCore/QMap>
#include <QtCore/QPluginLoader>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "GlobalManager.hpp"

struct PluginConfig {
    bool    load;
    QString name;
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
    bool ReadPluginConfig(QString filename = "");

    /**
     * @brief 加载单个插件
     * @param pluginName   插件名称
     */
    bool LoadPluginOne(QString pluginName);

    /**
     * @brief 加载所有插件
     */
    void LoadPluginAll();

private:
    struct PluginManagerPrivate;
    PluginManagerPrivate* m_P;
};

#endif