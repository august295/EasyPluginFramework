#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <vector>

#include "Common/IPlugin.hpp"
#include "Common/IEvent.hpp"

struct EventItem
{
    std::string            eventName;
    std::shared_ptr<Event> data;
};

struct EventCompare
{
    bool operator()(const EventItem& a, const EventItem& b)
    {
        return a.data->priority < b.data->priority; // 大优先级先出
    }
};

class PluginWrapper
{
public:
    IPlugin*                                                             plugin;
    std::priority_queue<EventItem, std::vector<EventItem>, EventCompare> eventQueue;
    std::mutex                                                           queueMutex;
    std::condition_variable                                              cv;
    std::thread                                                          worker;
    std::atomic<bool>                                                    running{true};

    PluginWrapper(IPlugin* p) : plugin(p)
    {
        worker = std::thread([this]() { this->processLoop(); });
    }

    ~PluginWrapper()
    {
        running = false;
        cv.notify_all();
        if (worker.joinable())
            worker.join();
    }

    void pushEvent(const std::string& name, std::shared_ptr<Event> e)
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        eventQueue.push({name, e});
        cv.notify_one();
    }

private:
    void processLoop()
    {
        while (running)
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] { return !eventQueue.empty() || !running; });
            while (!eventQueue.empty())
            {
                auto item = eventQueue.top();
                eventQueue.pop();
                lock.unlock();

                plugin->OnEvent(item.eventName, *item.data.get());

                lock.lock();
            }
        }
    }
};
