#ifndef __GLOBAL_PLUGIN_SM2_PKCS7_VERIFY_HPP__
#define __GLOBAL_PLUGIN_SM2_PKCS7_VERIFY_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINSM2PKCS7VERIFY_EXPORTS
        #define PLUGINSM2PKCS7VERIFY_API __declspec(dllexport)
    #else
        #define PLUGINSM2PKCS7VERIFY_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINSM2PKCS7VERIFY_API __attribute__((visibility("default")))
#else
    #define PLUGINSM2PKCS7VERIFY_API
#endif

#endif
