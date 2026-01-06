#ifndef __LOGGERSQLSINK_H__
#define __LOGGERSQLSINK_H__

#include <exception>
#include <mutex>
#include <sstream>

#include <spdlog/sinks/base_sink.h>
#include <sqlite3.h>
#include <ghc/fs_std.hpp>

template <typename Mutex>
class SQLSink : public spdlog::sinks::base_sink<Mutex>
{
public:
    SQLSink(const std::string& db_path)
    {
        int ret = SQLITE_OK;

        // 打开数据库
        ret = sqlite3_open(db_path.c_str(), &db_);
        if (SQLITE_OK != ret)
        {
            throw std::runtime_error("open database failed");
        }

        // 创建表
        const char* sql = R"(
        CREATE TABLE IF NOT EXISTS log (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            thread_id TEXT NOT NULL,
            level TEXT NOT NULL,
            source TEXT NOT NULL,
            message TEXT NOT NULL
        );)";
        ret             = sqlite3_exec(db_, sql, NULL, NULL, NULL);
        if (SQLITE_OK != ret)
        {
            throw std::runtime_error("create table failed");
        }
    }
    ~SQLSink() {};

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        // 时间戳
        char        buf[64];
        std::time_t t = std::chrono::system_clock::to_time_t(msg.time);
        std::tm     tm_time;
#if defined(_WIN32)
        localtime_s(&tm_time, &t);
#else
        localtime_r(&t, &tm_time);
#endif
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_time);

        // 线程 ID
        std::stringstream thread_ss;
        thread_ss << msg.thread_id;

        // 文件+行号+函数
        auto        filename = fs::path(msg.source.filename);
        std::string tempname = filename.filename().string();
        std::string source   = fmt::format("{}:{} {}", tempname, msg.source.line, msg.source.funcname);

        // 格式化消息
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        std::string log_message = std::string(msg.payload.data(), msg.payload.size()); // fmt::to_string(formatted);

        // 插入 SQLite
        const char*   sql  = "INSERT INTO log (timestamp, thread_id, level, source, message) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, buf, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, thread_ss.str().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, spdlog::level::to_string_view(msg.level).data(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, source.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, log_message.c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                fprintf(stderr, "SQLite insert error: %s\n", sqlite3_errmsg(db_));
            }
        }
        else
        {
            fprintf(stderr, "SQLite prepare error: %s\n", sqlite3_errmsg(db_));
        }

        sqlite3_finalize(stmt);
    }

    void flush_() override
    {
        // SQLite 默认是自动提交，可以不实现
    }

private:
    sqlite3* db_;
};

// 类型别名
using sqlite_sink_detailed_mt = SQLSink<std::mutex>;

#endif