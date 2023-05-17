#ifndef __IPLUGIN_HPP__
#define __IPLUGIN_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #define IPLUGIN_API __declspec(dllexport)
#elif defined(__linux__)
    #define IPLUGIN_API __attribute__((visibility("default")))
#else
    #define IPLUGIN_API
#endif

/**
 * @brief 插件接口类
 */
class IPLUGIN_API IPlugin {
public:
    virtual ~IPlugin() {}

    virtual bool Init() = 0;
};

#endif
