#include "Subscribe.h"

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
        emit                signal_log(QString::fromStdString(me->message));
    }
    break;
    case ET_LOG: {
        const MessageEvent* me = dynamic_cast<const MessageEvent*>(event);
        emit                signal_log(QString::fromStdString(me->message));
    }
    break;
    default:
        break;
    }
}
