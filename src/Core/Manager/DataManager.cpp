#include "DataManager.h"

struct DataManager::DataManagerPrivate {
    std::map<std::string, std::vector<Callback>> m_CallbackMap;
    std::mutex                                   m_MutexCallback;
    std::shared_ptr<ThreadPool>                  m_ThreadPoolSptr;
};

DataManager::DataManager()
    : TSingleton<DataManager>()
    , m_P(new DataManagerPrivate)
{
    m_P->m_ThreadPoolSptr = std::make_shared<ThreadPool>(std::thread::hardware_concurrency() / 2);
}

DataManager::~DataManager()
{
    delete m_P;
}

void DataManager::Subscribe(std::string name, Callback callback)
{
    std::unique_lock<std::mutex> lock(m_P->m_MutexCallback);
    m_P->m_CallbackMap[name].emplace_back(callback);
}

void DataManager::Unsubscribe(std::string name, Callback callback)
{
    std::unique_lock<std::mutex> lock(m_P->m_MutexCallback);
    auto&                        iterMap = m_P->m_CallbackMap.find(name);
    if (iterMap != m_P->m_CallbackMap.end()) {
        auto& callbackVec = iterMap->second;
        // 删除订阅的回调函数
        callbackVec.erase(std::remove_if(callbackVec.begin(),
                                         callbackVec.end(),
                                         [&](const Callback& iter) {
                                             return iter.target_type() == callback.target_type();
                                         }),
                          callbackVec.end());
        if (iterMap->second.empty()) {
            m_P->m_CallbackMap.erase(iterMap);
        }
    }
}

void DataManager::Publish(std::string name, const std::any& data)
{
    std::unique_lock<std::mutex> lock(m_P->m_MutexCallback);
    auto&                        iterMap = m_P->m_CallbackMap.find(name);
    if (iterMap != m_P->m_CallbackMap.end()) {
        for (auto& callback : iterMap->second) {
            // 单线程
            callback(data);
        }
    }
}

void DataManager::PublishDetach(std::string name, const std::any& data)
{
    std::unique_lock<std::mutex> lock(m_P->m_MutexCallback);
    auto&                        iterMap = m_P->m_CallbackMap.find(name);
    if (iterMap != m_P->m_CallbackMap.end()) {
        for (auto& callback : iterMap->second) {
            // 多线程分离
            std::thread t([&callback, data]() { callback(data); });
            t.detach();
        }
    }
}

void DataManager::PublishAsync(std::string name, const std::any& data, std::vector<std::future<void>>& futureVec)
{
    std::unique_lock<std::mutex> lock(m_P->m_MutexCallback);
    auto&                        iterMap = m_P->m_CallbackMap.find(name);
    if (iterMap != m_P->m_CallbackMap.end()) {
        for (auto& callback : iterMap->second) {
            // 线程池
            std::future<void> f = m_P->m_ThreadPoolSptr->enqueue([&callback, data]() { callback(data); });
            futureVec.emplace_back(std::move(f));
        }
    }
}