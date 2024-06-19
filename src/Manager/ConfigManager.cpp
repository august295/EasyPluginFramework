#include "ConfigManager.h"

// 显式实例化模板，确保单例实例在整个程序中唯一
template class TSingleton<ConfigManager>;

struct ConfigManager::ConfigManagerPrivate {
    std::string m_BinPath; // 主程序运行路径
};

ConfigManager::ConfigManager()
    : m_impl(new ConfigManagerPrivate)
{
}

ConfigManager::~ConfigManager()
{
    delete m_impl;
}

void ConfigManager::SetBinPath(std::string binPath)
{
    m_impl->m_BinPath = binPath;
}

std::string ConfigManager::GetBinPath()
{
    return m_impl->m_BinPath;
}
