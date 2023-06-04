#include <chrono>
#include <iostream>

#include <Manager/DataManager.h>

#include "PluginTest.h"
#include "ProtoTest/Person.pb.h"

PluginTest::PluginTest()
{
}

PluginTest::~PluginTest()
{
    std::cout << " delete PluginTest" << std::endl;
}

std::string PluginTest::Version()
{
    return "0.0.1";
}

std::string PluginTest::Description()
{
    return "测试插件";
}

bool PluginTest::Init()
{
    auto start = std::chrono::steady_clock::now();

    std::cout << "PluginTest plugin load success" << std::endl;

    //// 测试单线程
    // DataManager::instance()->Publish("int", 10);

    //// 测试多线程
    // DataManager::instance()->PublishDetach("int", 10);

    //// 测试线程池
    // std::vector<std::future<void>> futureVec;
    // DataManager::instance()->PublishAsync("int", 10, futureVec);
    // for (auto&& f : futureVec) {
    //     f.wait();
    // }
    // std::cout << "PluginTest plugin publish finish" << std::endl;

    // 测试 protobuf
    TEST::Person p;
    p.set_name("august");
    p.set_id(1);
    p.set_email("august@qq.com");
    p.set_salary(1000);
    p.set_phone_num(1);
    // 原生类
    DataManager::instance()->Publish(TEST::Person::descriptor()->full_name() + "any", p);
    // 序列化
    std::string str = p.SerializeAsString();
    DataManager::instance()->Publish(TEST::Person::descriptor()->full_name(), str);

    auto                          end             = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = std::chrono::duration<double>(end - start);

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
    return false;
}

bool PluginTest::InitAppFinish()
{
	return false;
}

bool PluginTest::Release()
{
	delete this;
	return false;
}

// 插件创建函数
extern "C" PLUGINTEST_API IPlugin* CreatePlugin()
{
    // 替换为你的插件类的实例化
    return new PluginTest();
}
