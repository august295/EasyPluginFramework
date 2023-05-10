#ifndef __DATAMANAGER_H__
#define __DATAMANAGER_H__

#include <QtCore/QString>

#include <TSingleton.hpp>

#include "GlobalManager.hpp"

/**
 * @brief 数据管理
 */
class MANAGER_API DataManager : public TSingleton<DataManager> {
public:
    /**
     * @brief 设置可执行程序路径
     * @param binPath     可执行程序路径
     */
    void SetBinPath(QString binPath);

    /**
     * @brief 获取可执行程序路径
     * @return QString     可执行程序路径
     */
    QString GetBinPath();

private:
    // 只允许使用单例模式，不允许创建对象
    friend class TSingleton<DataManager>;
    DataManager();
    ~DataManager();

    struct DataManagerPrivate;
    DataManagerPrivate* m_P;
};

#endif
