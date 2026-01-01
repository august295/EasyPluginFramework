#ifndef __LOGGERMANAGER_H__
#define __LOGGERMANAGER_H__

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <Common/TSingleton.hpp>

/**
 * @brief 日志管理器
 *      1. 统一日志：使用该管理器
 *      2. 单独日志：使用 spdlog 自定义创建
 */
class LoggerManager : public TSingleton<LoggerManager>
{
public:
    std::shared_ptr<spdlog::logger> GetLogger(const std::string &loggerName);

private:
    // 只允许使用单例模式，不允许创建对象
    friend class TSingleton<LoggerManager>;
    LoggerManager();
    ~LoggerManager();
};

/**
 * @brief 定义宏，输出 __FILE__ __LINE__ __FUNCTION__
 */
#define EPF_LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define EPF_LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define EPF_LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define EPF_LOG_WARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define EPF_LOG_ERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define EPF_LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

#endif