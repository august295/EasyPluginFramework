#ifndef __GLOBALPLUGINBASE64_HPP__
#define __GLOBALPLUGINBASE64_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINBASE64_EXPORTS
        #define PLUGINBASE64_API __declspec(dllexport)
    #else
        #define PLUGINBASE64_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINBASE64_API __attribute__((visibility("default")))
#else
    #define PLUGINBASE64_API
#endif

#endif
