#ifndef __CONFIGMANAGER_H__
#define __CONFIGMANAGER_H__

#include <string>

#include <Common/TSingleton.hpp>

#include "GlobalManager.hpp"

/**
 * @brief 配置管理
 */
class MANAGER_API ConfigManager : public TSingleton<ConfigManager> {
public:
    /**
     * @brief 设置可执行程序路径
     * @param binPath     可执行程序路径
     */
    void SetBinPath(std::string binPath);

    /**
     * @brief 获取可执行程序路径
     * @return QString     可执行程序路径
     */
    std::string GetBinPath();

private:
    // 只允许使用单例模式，不允许创建对象
    friend class TSingleton<ConfigManager>;
    ConfigManager();
    ~ConfigManager();

    struct ConfigManagerPrivate;
    ConfigManagerPrivate* m_impl;
};

#endif
