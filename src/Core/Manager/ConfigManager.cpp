#include "ConfigManager.h"

struct ConfigManager::ConfigManagerPrivate {
    std::string m_BinPath; // 主程序运行路径
};

ConfigManager::ConfigManager()
    : TSingleton<ConfigManager>()
    , m_impl(new ConfigManagerPrivate)
{
    // 在构造函数中调用父类的构造函数来保证单例特性
    // 子类自定义构造函数中可以添加自己的逻辑
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
