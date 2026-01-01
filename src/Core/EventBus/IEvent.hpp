#ifndef __IEVENT_HPP__
#define __IEVENT_HPP__

#include <string>

enum EventType
{
    ET_UNKNOWN,
    ET_MESSAGE,
    ET_LOG,
};

struct Event
{
    int priority; // 优先级
    int type;     // 类型

    virtual ~Event() = default;
};

struct MessageEvent
    : public Event
{
    std::string name;
    std::string message;

    MessageEvent(int priority, int type, const std::string& name, const std::string& message)
    {
        this->priority = priority;
        this->type     = type;
        this->name     = name;
        this->message  = message;
    }
};

struct LogEvent
    : public Event
{
    std::string name;
    std::string file;
    int         line;
    std::string function;
    std::string log;

    LogEvent(int priority, int type, const std::string& name, const std::string& file, int line, const std::string& function, const std::string& log)
    {
        this->priority = priority;
        this->type     = type;
        this->name     = name;
        this->file     = file;
        this->line     = line;
        this->function = function;
        this->log      = log;
    }
};

#endif