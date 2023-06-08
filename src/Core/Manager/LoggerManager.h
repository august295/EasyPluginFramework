#ifndef __LOGGERMANAGER_H__
#define __LOGGERMANAGER_H__

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <common/TSingleton.hpp>

#include "GlobalManager.hpp"

/**
 * @brief 日志管理器
 *      1. 统一日志：使用该管理器
 *      2. 单独日志：使用 spdlog 自定义创建
 */
class MANAGER_API LoggerManager : public TSingleton<LoggerManager> {
public:
    std::shared_ptr<spdlog::logger> GetLogger();

private:
    // 只允许使用单例模式，不允许创建对象
    friend class TSingleton<LoggerManager>;
    LoggerManager();
    ~LoggerManager();

    std::shared_ptr<spdlog::logger> m_logger;
};

/**
 * @brief 定义宏，输出 __FILE__ __LINE__ __FUNCTION__
 */
#define LOG_INFO(...)  SPDLOG_LOGGER_INFO(LoggerManager::instance()->GetLogger(), __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(LoggerManager::instance()->GetLogger(), __VA_ARGS__)

#endif