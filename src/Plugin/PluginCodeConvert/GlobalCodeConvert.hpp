#ifndef __GLOBALCODECONVERT_HPP__
#define __GLOBALCODECONVERT_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINCODECONVERT_EXPORTS
        #define PLUGINCODECONVERT_API __declspec(dllexport)
    #else
        #define PLUGINCODECONVERT_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINCODECONVERT_API __attribute__((visibility("default")))
#else
    #define PLUGINCODECONVERT_API
#endif

#endif
