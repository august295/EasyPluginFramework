#ifndef __TSINGLETON_HPP__
#define __TSINGLETON_HPP__

/**
 * @brief 单例模板，奇异递归模板
 * @tparam T 实例化单例类
 */
template <class T>
class TSingleton {
public:
    // C++11 Meyer's Singleton，线程安全
    static T* instance()
    {
        static T instance;
        return& instance;
    }

protected:
    TSingleton()  = default;
    ~TSingleton() = default;
    // 删除拷贝构造函数，赋值运算符，移动构造函数，移动赋值运算符，防止复制实例
    TSingleton(const TSingleton&)            = delete;
    TSingleton& operator=(const TSingleton&) = delete;
    TSingleton(TSingleton&&)                 = delete;
    TSingleton& operator=(TSingleton&&)      = delete;
};

#endif
