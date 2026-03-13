#ifndef __GLOBAL_PLUGIN_ASN1_VIEWER_HPP__
#define __GLOBAL_PLUGIN_ASN1_VIEWER_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINASN1VIEWER_EXPORTS
        #define PLUGINASN1VIEWER_API __declspec(dllexport)
    #else
        #define PLUGINASN1VIEWER_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINASN1VIEWER_API __attribute__((visibility("default")))
#else
    #define PLUGINASN1VIEWER_API
#endif

#endif
