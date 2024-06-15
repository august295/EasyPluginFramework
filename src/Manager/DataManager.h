#ifndef __DATAMANAGER_H__
#define __DATAMANAGER_H__

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <Common/Any.hpp>
#include <Common/TSingleton.hpp>
#include <Common/ThreadPool.hpp>

#include "GlobalManager.hpp"

/**
 * @brief 数据管理
 */
class MANAGER_API DataManager : public TSingleton<DataManager>
{
public:
    /**
     * @brief 回调函数别名
     */
    using Callback = std::function<void(const Easy::Any&)>;

    /**
     * @brief 订阅
     * @param name         数据类型名称
     * @param callback     回调函数
     */
    void Subscribe(std::string name, Callback callback);

    /**
     * @brief 取消订阅
     * @param name         数据类型名称
     * @param callback     回调函数
     */
    void Unsubscribe(std::string name, Callback callback);

    /**
     * @brief 发布消息，单线程模式
     * @param name         数据类型名称
     * @param data         详细数据
     */
    void Publish(std::string name, const Easy::Any& data);

    /**
     * @brief 发布消息，多线程模式且分离
     * @param name         数据类型名称
     * @param data         详细数据
     */
    void PublishDetach(std::string name, const Easy::Any& data);

    /**
     * @brief 发布消息，线程池模式
     * @param name         数据类型名称
     * @param data         详细数据
     * @param futureVec    [out] 各线程执行结果
     */
    void PublishAsync(std::string name, const Easy::Any& data, std::vector<std::future<void>>& futureVec);

private:
    // 只允许使用单例模式，不允许创建对象
    friend class TSingleton<DataManager>;
    DataManager();
    ~DataManager();

    struct DataManagerPrivate;
    DataManagerPrivate* m_P;
};

#endif
