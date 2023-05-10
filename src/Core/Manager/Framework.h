#ifndef __FRAMEWORK_H__
#define __FRAMEWORK_H__

#include <memory>

#include <QtCore/QMap>
#include <QtCore/QString>

#include "PluginManager.h"

/**
 * @brief 框架类
 * 这是一个组合类
 *  - 插件管理
 */
class MANAGER_API Framework {
public:
    Framework();
    ~Framework();

    /**
     * @brief 获取插件管理智能指针
     * @return std::shared_ptr<PluginManager>&
     */
    std::shared_ptr<PluginManager>& GetPluginManager();

private:
    struct FrameworkPrivate;
    FrameworkPrivate* m_P;
};

#endif