#ifndef __GLOBALTEST_HPP__
#define __GLOBALTEST_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef TEST_EXPORTS
        #define TEST_API __declspec(dllexport)
    #else
        #define TEST_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define TEST_API __attribute__((visibility("default")))
#else
    #define TEST_API
#endif

#endif
