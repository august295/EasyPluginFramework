#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <memory>

#include "IEventBus.hpp"

using TopicId = uint64_t;

struct QueuedEvent
{
    TopicId                topic;
    std::shared_ptr<Event> event;
};

static TopicId hashTopic(const std::string& topicName)
{
    TopicId hashValue = 1469598103934665603ULL;
    for (char c : topicName)
    {
        hashValue ^= static_cast<uint8_t>(c);
        hashValue *= 1099511628211ULL;
    }
    return hashValue;
}

struct Compare
{
    bool operator()(const QueuedEvent& left, const QueuedEvent& right) const
    {
        return left.event->priority > right.event->priority;
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
        if (event == nullptr)
        {
            return;
        }

        QueuedEvent queuedEvent;
        queuedEvent.topic = hashTopic(id);
        queuedEvent.event.reset(event);
        {
            std::lock_guard<std::mutex> lock(qmtx_);
            queue_.push(queuedEvent);
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
            QueuedEvent queuedEvent;
            {
                std::unique_lock<std::mutex> lock(qmtx_);
                cv_.wait(lock, [&] { return !queue_.empty() || !running_; });
                if (!running_)
                    break;
                queuedEvent = queue_.top();
                queue_.pop();
            }

            std::vector<IEventHandler*> handlers;
            {
                std::lock_guard<std::mutex> lock(smtx_);
                auto                        handlerIter = subs_.find(queuedEvent.topic);
                if (handlerIter != subs_.end()) {
                    handlers.assign(handlerIter->second.begin(), handlerIter->second.end());
                }
            }

            for (auto* handler : handlers)
            {
                handler->OnEvent(queuedEvent.event.get());
            }
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
