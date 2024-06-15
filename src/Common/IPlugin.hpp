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
 * @brief 插件类型
 */
enum class PluginType {
    NONE,
    WIDGET,
};

/**
 * @brief 插件显示位置
 */
struct PluginLocation {
    PluginType  m_type;
    std::string m_name;
    std::string m_group;
    std::string m_page;

    PluginLocation()
        : m_type(PluginType::NONE) {}
    PluginLocation(PluginType type, std::string name, std::string group = "默认", std::string page = "默认")
        : m_type(type)
        , m_name(name)
        , m_group(group)
        , m_page(page)
    {
    }
};

/**
 * @brief 插件接口类
 */
class IPLUGIN_API IPlugin
{
public:
    virtual ~IPlugin() {}

    /**
     * @brief 初始化插件
     */
    virtual bool Init() = 0;

    /**
     * @brief 应用程序初始化完成
     */
    virtual bool InitAppFinish() = 0;

    /**
     * @brief 销毁插件
     */
    virtual bool Release() = 0;

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

    /**
     * @brief 获取插件图标
     * @return std::string 插件图标
     */
    virtual std::string Icon() = 0;

    /**
     * @brief 获取插件显示位置
     * @return std::string 插件位置
     */
    virtual PluginLocation Location() = 0;

    /**
     * @brief 插件显示
     */
    virtual void WidgetShow() = 0;
};

#endif
