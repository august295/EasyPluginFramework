#ifndef __IEVENTBUS_HPP__
#define __IEVENTBUS_HPP__

#include "IEvent.hpp"
#include "IEventHandler.hpp"
#include "GlobalEventBus.hpp"

struct IEventBus {
    virtual ~IEventBus() = default;

    virtual void publish(const std::string& id, Event* e) = 0;
    virtual void subscribe(const std::string& id, IEventHandler* h) = 0;
    virtual void unsubscribe(const std::string& id, IEventHandler* h) = 0;
};

extern "C" {
    EVENTBUS_API IEventBus* GetEventBus();
}

#endif