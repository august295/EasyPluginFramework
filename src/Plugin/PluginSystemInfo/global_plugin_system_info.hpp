#ifndef __GLOBAL_PLUGIN_SYSTEM_INFO_HPP__
#define __GLOBAL_PLUGIN_SYSTEM_INFO_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINSYSTEMINFO_EXPORTS
        #define PLUGINSYSTEMINFO_API __declspec(dllexport)
    #else
        #define PLUGINSYSTEMINFO_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINSYSTEMINFO_API __attribute__((visibility("default")))
#else
    #define PLUGINSYSTEMINFO_API
#endif

#endif
