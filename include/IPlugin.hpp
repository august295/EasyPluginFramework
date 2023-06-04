#ifndef __IPLUGIN_HPP__
#define __IPLUGIN_HPP__

#if defined(_WIN32) || defined(_WIN64)
    #define IPLUGIN_API __declspec(dllexport)
#elif defined(__linux__)
    #define IPLUGIN_API __attribute__((visibility("default")))
#else
    #define IPLUGIN_API
#endif

#include <string>

/**
 * @brief 插件接口类
 */
class IPLUGIN_API IPlugin {
public:
    virtual ~IPlugin() {}

    /**
     * @brief 获取插件版本
     * @return std::string 插件版本
     */
    virtual std::string Version() = 0;

    /**
     * @brief 获取插件描述
     * @return std::string 插件描述
     */
    virtual std::string Description() = 0;

    // 初始化插件
    virtual bool Init() = 0;

    // 应用程序初始化完成
    virtual bool InitAppFinish() = 0;

    // 销毁插件
    virtual bool Release() = 0;
};

#endif
