#include "Subscribe.h"

Subscribe::Subscribe()
{
    DataManager::instance().Subscribe("log", [=](const Easy::Any& data) {
        auto text = Easy::any_cast<std::string>(data);
        emit signal_log(QString::fromStdString(text));
    });
}

Subscribe::~Subscribe()
{
}
