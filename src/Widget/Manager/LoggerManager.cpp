#include "LoggerManager.h"
#include "LoggerSQLSink.h"

LoggerManager::LoggerManager()
{
    // 总日志
    auto console     = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto all_file    = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/all.log", 50 * 1024 * 1024, 10);
    auto sqlite_sink = std::make_shared<sqlite_sink_detailed_mt>("logs/sqlite.db");
    auto all_logger  = std::make_shared<spdlog::logger>("all", spdlog::sinks_init_list{console, all_file, sqlite_sink});
    all_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%^%l%$] [%s:%# %!] %v");
    all_logger->set_level(spdlog::level::info);
    all_logger->flush_on(spdlog::level::warn);
    spdlog::register_logger(all_logger);
    spdlog::set_default_logger(all_logger);
    // 插件日志
    auto plugin_file   = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/plugin.log", 50 * 1024 * 1024, 10);
    auto plugin_logger = std::make_shared<spdlog::logger>("plugin", spdlog::sinks_init_list{console, plugin_file});
    spdlog::register_logger(plugin_logger);
    // 定时刷新
    spdlog::flush_every(std::chrono::seconds(5));
}

LoggerManager::~LoggerManager()
{
    spdlog::shutdown();
}

std::shared_ptr<spdlog::logger> LoggerManager::GetLogger(const std::string& loggerName)
{
    return spdlog::get(loggerName);
}
