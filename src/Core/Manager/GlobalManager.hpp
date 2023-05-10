#ifndef __GLOBALMANAGER_HPP__
#define __GLOBALMANAGER_HPP__

#if defined(WIN32) || defined(_WIN32)
    #ifdef MANAGER_EXPORTS
        #define MANAGER_API __declspec(dllexport)
    #else
        #define MANAGER_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define MANAGER_API __attribute__((visibility("default")))
#else
    #define MANAGER_API
#endif

#endif
