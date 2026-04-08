#ifndef __GLOBAL_PLUGIN_SM2_CSR_VERIFY_HPP__
#define __GLOBAL_PLUGIN_SM2_CSR_VERIFY_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINSM2CSRVERIFY_EXPORTS
        #define PLUGINSM2CSRVERIFY_API __declspec(dllexport)
    #else
        #define PLUGINSM2CSRVERIFY_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINSM2CSRVERIFY_API __attribute__((visibility("default")))
#else
    #define PLUGINSM2CSRVERIFY_API
#endif

#endif
