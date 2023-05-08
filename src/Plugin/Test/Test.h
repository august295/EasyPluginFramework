#ifndef __TEST_H__
#define __TEST_H__

#ifdef TEST_LIBRARY
#define TEST_EXPORT __declspec(dllexport)
#else
#define TEST_EXPORT __declspec(dllimport)
#endif

#include "IPlugin.hpp"

class TEST_EXPORT Test : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPlugin_IID)
    Q_INTERFACES(IPlugin)

public:
    Test();
    ~Test();

    bool Init() override;
};

#endif
