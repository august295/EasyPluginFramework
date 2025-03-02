#ifndef __GLOBALPLUGINCERTVIEW_HPP__
#define __GLOBALPLUGINCERTVIEW_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #ifdef PLUGINCERTVIEW_EXPORTS
        #define PLUGINCERTVIEW_API __declspec(dllexport)
    #else
        #define PLUGINCERTVIEW_API __declspec(dllimport)
    #endif
#elif defined(__linux__)
    #define PLUGINCERTVIEW_API __attribute__((visibility("default")))
#else
    #define PLUGINCERTVIEW_API
#endif

#endif
