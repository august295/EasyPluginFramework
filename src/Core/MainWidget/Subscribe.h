#ifndef __SUBSCRIBE_H__
#define __SUBSCRIBE_H__

#include <QtCore/QObject>

#include <Manager/DataManager.h>

class Subscribe : public QObject
{
    Q_OBJECT

public:
    Subscribe();
    ~Subscribe();

signals:
    void signal_log(const QString& text);
};

#endif // __SUBSCRIBE_H__