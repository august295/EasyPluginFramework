#include <chrono>
#include <iostream>

#include <Manager/DataManager.h>

#include "PluginTest.h"

PluginTest::PluginTest()
{
}

PluginTest::~PluginTest()
{
    std::cout << " delete PluginTest" << std::endl;
}

bool PluginTest::Init()
{
    auto start = std::chrono::steady_clock::now();

    std::cout << "PluginTest plugin load success" << std::endl;

    // 测试单线程
     DataManager::instance()->Publish("int", 10);

    // 测试多线程
     DataManager::instance()->PublishDetach("int", 10);

    // 测试线程池
     std::vector<std::future<void>> futureVec;
     DataManager::instance()->PublishAsync("int", 10, futureVec);
     for (auto&& f : futureVec) {
         f.wait();
     }
     std::cout << "PluginTest plugin publish finish" << std::endl;

    auto                          end             = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = std::chrono::duration<double>(end - start);

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
    return true;
}

bool PluginTest::InitAppFinish()
{
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

// 插件创建函数
extern "C" PLUGINTEST_API IPlugin* CreatePlugin()
{
    // 替换为你的插件类的实例化
    return new PluginTest();
}
