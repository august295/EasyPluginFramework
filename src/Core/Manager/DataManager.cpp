#include "DataManager.h"

struct DataManager::DataManagerPrivate {
    QString m_BinPath; // 主程序运行路径
};

DataManager::DataManager()
    : TSingleton<DataManager>()
    , m_P(new DataManagerPrivate)
{
    // 在构造函数中调用父类的构造函数来保证单例特性
    // 子类自定义构造函数中可以添加自己的逻辑
}

DataManager::~DataManager()
{
    delete m_P;
}

void DataManager::SetBinPath(QString binPath)
{
    m_P->m_BinPath = binPath;
}

QString DataManager::GetBinPath()
{
	return m_P->m_BinPath;
}
