#ifndef __GLOBALTEST_HPP__
#define __GLOBALTEST_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINTEST_EXPORTS
        #define PLUGINTEST_API __declspec(dllexport)
    #else
        #define PLUGINTEST_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINTEST_API __attribute__((visibility("default")))
#else
    #define PLUGINTEST_API
#endif

#endif
