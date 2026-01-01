#include "Subscribe.h"
#include "Manager/LoggerManager.h"

Subscribe& Subscribe::instance()
{
    static Subscribe subscribe;
    return subscribe;
}

Subscribe::Subscribe()
{
    m_eventBus = GetEventBus();
    m_eventBus->subscribe("test", this);
    m_eventBus->subscribe("log", this);
}

Subscribe::~Subscribe()
{
}

void Subscribe::OnEvent(const Event* event)
{
    switch (event->type)
    {
    case ET_MESSAGE: {
        const MessageEvent* me = dynamic_cast<const MessageEvent*>(event);
        emit                signal_log(QString("[%1] %2").arg(me->name.c_str()).arg(me->message.c_str()));
        LoggerManager::instance().GetLogger("plugin")->info("[{}] {}", me->name, me->message);
    }
    break;
    case ET_LOG: {
        const LogEvent* le = dynamic_cast<const LogEvent*>(event);
        emit            signal_log(QString("[%1] %2").arg(le->name.c_str()).arg(le->log.c_str()));
        LoggerManager::instance().GetLogger("plugin")->info("[{}] [{}:{} {}] {}", le->name, le->file, le->line, le->function, le->log);
    }
    break;
    default:
        break;
    }
}
