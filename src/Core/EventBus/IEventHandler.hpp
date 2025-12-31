#ifndef __IEVENTHANDLER_HPP__
#define __IEVENTHANDLER_HPP__

#include "IEvent.hpp"

struct IEventHandler
{
    virtual ~IEventHandler() = default;

    virtual void OnEvent(const Event* event) = 0;
};

#endif