#ifndef __IFRAMEWORK_HPP__
#define __IFRAMEWORK_HPP__

class IPlugin;

class IFramework {
public:
    virtual ~IFramework() {}

    virtual IPlugin* GetPlugin() = 0;
};

#endif
