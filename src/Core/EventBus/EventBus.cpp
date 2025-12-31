#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <cstring>

#include "IEventBus.hpp"

using TopicId = uint64_t;

struct QueuedEvent
{
    TopicId topic;
    Event*  event;
};

static TopicId hashTopic(const std::string& s)
{
    // FNV-1a
    TopicId h = 1469598103934665603ULL;
    for (char c : s)
    {
        h ^= (uint8_t)c;
        h *= 1099511628211ULL;
    }
    return h;
}

struct Compare
{
    bool operator()(const QueuedEvent& a, const QueuedEvent& b)
    {
        return a.event->priority > b.event->priority;
    }
};

class EventBus final : public IEventBus
{
public:
    EventBus()
    {
        running_ = true;
        worker_  = std::thread([this] { dispatchLoop(); });
    }

    ~EventBus() override
    {
        running_ = false;
        cv_.notify_all();
        if (worker_.joinable())
            worker_.join();
    }

    void publish(const std::string& id, Event* event) override
    {
        QueuedEvent qe;
        qe.topic = hashTopic(id);
        qe.event = event;
        {
            std::lock_guard<std::mutex> lock(qmtx_);
            queue_.push(std::move(qe));
        }
        cv_.notify_one();
    }

    void subscribe(const std::string& topic, IEventHandler* h) override
    {
        std::lock_guard<std::mutex> lock(smtx_);
        TopicId                     id = hashTopic(topic);
        subs_[id].insert(h);
    }

    void unsubscribe(const std::string& topic, IEventHandler* h) override
    {
        std::lock_guard<std::mutex> lock(smtx_);
        TopicId                     id = hashTopic(topic);
        subs_[id].erase(h);
    }

private:
    void dispatchLoop()
    {
        while (running_)
        {
            QueuedEvent qe;
            {
                std::unique_lock<std::mutex> lock(qmtx_);
                cv_.wait(lock, [&] { return !queue_.empty() || !running_; });
                if (!running_)
                    break;
                qe = std::move(queue_.top());
                queue_.pop();
            }

            std::lock_guard<std::mutex> lock(smtx_);
            auto                        handlers = subs_.find(qe.topic);
            if (handlers != subs_.end())
            {
                for (auto h : handlers->second)
                {
                    h->OnEvent(qe.event);
                }
            }
            delete qe.event;
        }
    }

private:
    std::atomic<bool>                                                   running_{false};
    std::mutex                                                          smtx_;
    std::unordered_map<TopicId, std::unordered_set<IEventHandler*>>     subs_;
    std::mutex                                                          qmtx_;
    std::priority_queue<QueuedEvent, std::vector<QueuedEvent>, Compare> queue_;
    std::condition_variable                                             cv_;
    std::thread                                                         worker_;
};

extern "C" EVENTBUS_API IEventBus* GetEventBus()
{
    static EventBus g_bus;
    return &g_bus;
}
