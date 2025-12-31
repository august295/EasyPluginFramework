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
    virtual ~Event() = default;

    int priority; // 优先级
    int type;     // 类型
};

struct MessageEvent
    : public Event
{
    std::string message;

    MessageEvent(int priority, int type, const std::string& message)
    {
        this->priority = priority;
        this->type     = type;
        this->message  = message;
    }
};

#endif