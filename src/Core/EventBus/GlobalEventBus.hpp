#ifndef __GLOBALEVENTBUS_HPP__
#define __GLOBALEVENTBUS_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef EVENTBUS_EXPORTS
        #define EVENTBUS_API __declspec(dllexport)
    #else
        #define EVENTBUS_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define EVENTBUS_API __attribute__((visibility("default")))
#else
    #define EVENTBUS_API
#endif

#endif
