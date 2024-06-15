#include <iostream>

#include "Subscribe.h"

void test(const Easy::Any& data)
{
    std::this_thread::sleep_for(std::chrono::seconds(3));
    int num = Easy::any_cast<int>(data);
    std::cout << num + 1 << std::endl;
}

class Test {
public:
    Test()
    {
        DataManager::instance()->Subscribe("int", [this](const Easy::Any& data) {
            test(data);
        });
    }

    void test(const Easy::Any& data)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        int num = Easy::any_cast<int>(data);
        std::cout << num + 2 << std::endl;
    }
};

void test_all()
{
    // 测试 lambda
    DataManager::instance()->Subscribe("int", [](const Easy::Any& data) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int num = Easy::any_cast<int>(data);
        std::cout << num + 3 << std::endl;
    });
	   // 测试 function
    DataManager::instance()->Subscribe("int", test);
    // 测试 class
    Test t;
}

Subscribe::Subscribe()
{
    DataManager::instance()->Subscribe("log", [=](const Easy::Any& data) {
        auto text = Easy::any_cast<std::string>(data);
        emit signal_log(QString::fromStdString(text));
    });
}

Subscribe::~Subscribe()
{
}