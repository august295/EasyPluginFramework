#ifndef __GLOBALPLUGINASN1VIEW_HPP__
#define __GLOBALPLUGINASN1VIEW_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINASN1VIEW_EXPORTS
        #define PLUGINASN1VIEW_API __declspec(dllexport)
    #else
        #define PLUGINASN1VIEW_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINASN1VIEW_API __attribute__((visibility("default")))
#else
    #define PLUGINASN1VIEW_API
#endif

#endif
