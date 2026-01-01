#include "PluginTest.h"

PluginTest::PluginTest()
{
    m_eventBus = nullptr;
}

PluginTest::~PluginTest()
{
}

bool PluginTest::Init()
{
    m_eventBus = GetEventBus();
    m_eventBus->subscribe("test", this);
    return true;
}

bool PluginTest::InitAppFinish()
{
    std::string record = "插件初始化完成";
    m_eventBus->publish("log", new LogEvent(0, ET_LOG, Name(), __FILE__, __LINE__, __FUNCTION__, record));
    return true;
}

bool PluginTest::Release()
{
    delete this;
    return true;
}

std::string PluginTest::Version()
{
    return "0.0.1";
}

std::string PluginTest::Name()
{
    return "PluginTest";
}

std::string PluginTest::Description()
{
    return "测试插件";
}

std::string PluginTest::Icon()
{
    return "";
}

PluginLocation PluginTest::Location()
{
    return PluginLocation();
}

void PluginTest::WidgetShow()
{
}

void PluginTest::OnEvent(const Event* event)
{
    switch (event->type)
    {
    case ET_MESSAGE: {
        const MessageEvent* me = dynamic_cast<const MessageEvent*>(event);
        printf("%s\n", me->message.c_str());
    }
    case ET_LOG: {
        const MessageEvent* me = dynamic_cast<const MessageEvent*>(event);
        printf("%s\n", me->message.c_str());
    }
    break;
    default:
        break;
    }
}

// 插件创建函数
extern "C" PLUGINTEST_API IPlugin* CreatePlugin()
{
    // 替换为你的插件类的实例化
    return new PluginTest();
}
