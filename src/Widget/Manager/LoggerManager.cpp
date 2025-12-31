#include "LoggerManager.h"

// 显式实例化模板，确保单例实例在整个程序中唯一
template class TSingleton<LoggerManager>;

LoggerManager::LoggerManager()
{
    // 输出日志格式（全局）
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#]: %v");
    // 使用当前时间动态生成日志文件名
    m_logger = spdlog::basic_logger_mt("logger_basic_mt", "logs/log.txt");
}

LoggerManager::~LoggerManager()
{
}

std::shared_ptr<spdlog::logger> LoggerManager::GetLogger()
{
    return m_logger;
}
