#ifndef __GLOBAL_PLUGIN_P7B_CERTIFICATE_CHAIN_HPP__
#define __GLOBAL_PLUGIN_P7B_CERTIFICATE_CHAIN_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINP7BCERTIFICATECHAIN_EXPORTS
        #define PLUGINP7BCERTIFICATECHAIN_API __declspec(dllexport)
    #else
        #define PLUGINP7BCERTIFICATECHAIN_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINP7BCERTIFICATECHAIN_API __attribute__((visibility("default")))
#else
    #define PLUGINP7BCERTIFICATECHAIN_API
#endif

#endif
