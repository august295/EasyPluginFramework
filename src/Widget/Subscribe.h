#ifndef __SUBSCRIBE_H__
#define __SUBSCRIBE_H__

#include <QtCore/QObject>

#include "Core/EventBus/IEventHandler.hpp"
#include "Core/EventBus/IEventBus.hpp"

class Subscribe
    : public QObject
    , public IEventHandler
{
    Q_OBJECT

        public:
static Subscribe& instance();

private:
    Subscribe();
    ~Subscribe();

    void OnEvent(const Event* event) override;

signals:
    void signal_log(const QString& text);

private:
    IEventBus* m_eventBus;
};

#endif // __SUBSCRIBE_H__