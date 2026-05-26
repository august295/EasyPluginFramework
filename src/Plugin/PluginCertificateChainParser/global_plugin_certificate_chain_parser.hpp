#ifndef __GLOBAL_PLUGIN_CERTIFICATE_CHAIN_PARSER_HPP__
#define __GLOBAL_PLUGIN_CERTIFICATE_CHAIN_PARSER_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINCERTIFICATECHAINPARSER_EXPORTS
        #define PLUGINCERTIFICATECHAINPARSER_API __declspec(dllexport)
    #else
        #define PLUGINCERTIFICATECHAINPARSER_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINCERTIFICATECHAINPARSER_API __attribute__((visibility("default")))
#else
    #define PLUGINCERTIFICATECHAINPARSER_API
#endif

#endif

