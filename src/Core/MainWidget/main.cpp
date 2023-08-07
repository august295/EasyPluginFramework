#include <QApplication>

#include "MainWindow.h"

#include <Manager/DataManager.h>
//#include <ProtoTest/Person.pb.h>

//void test(const std::any& data)
//{
//    std::this_thread::sleep_for(std::chrono::seconds(3));
//    int num = std::any_cast<int>(data);
//    std::cout << num + 1 << std::endl;
//}
//
//class Test {
//public:
//    Test()
//    {
//        DataManager::instance()->Subscribe("int", [this](const std::any& data) {
//            test(data);
//        });
//    }
//
//    void test(const std::any& data)
//    {
//        std::this_thread::sleep_for(std::chrono::seconds(2));
//        int num = std::any_cast<int>(data);
//        std::cout << num + 2 << std::endl;
//    }
//};
//
//void test()
//{
//    //   // 测试 lambda
//    //   DataManager::instance()->Subscribe("int", [](const std::any& data) {
//    //       std::this_thread::sleep_for(std::chrono::seconds(1));
//    //       int num = std::any_cast<int>(data);
//    //       std::cout << num + 3 << std::endl;
//    //   });
//    //	 // 测试 function
//    //   DataManager::instance()->Subscribe("int", test);
//    //   // 测试 class
//    //   Test t;
//
//    // 测试 protobuf 结构体
//    DataManager::instance()->Subscribe(TEST::Person::descriptor()->full_name() + "any", [](const std::any data) {
//        TEST::Person p = std::any_cast<TEST::Person>(data);
//        std::cout << p.name() << std::endl;
//        std::cout << p.email() << std::endl;
//    });
//    // 测试 protobuf 序列化
//    DataManager::instance()->Subscribe(TEST::Person::descriptor()->full_name(), [](const std::any data) {
//        std::string  str = std::any_cast<std::string>(data);
//        TEST::Person p;
//        p.ParseFromString(str);
//        std::cout << p.name() << std::endl;
//        std::cout << p.email() << std::endl;
//    });
//}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
	MainWindow w;
	w.show();
    return a.exec();
}
