#ifndef __BASE64HELPER_H__
#define __BASE64HELPER_H__

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtGui/QImage>

class Base64Helper
{
public:
    Base64Helper(/* args */);
    ~Base64Helper();

    // 图片转base64字符串
    static QString    imageToBase64(const QImage& image);
    static QByteArray imageToBase64x(const QImage& image);

    // base64字符串转图片
    static QImage base64ToImage(const QString& data);
    static QImage base64ToImagex(const QByteArray& data);
};

#endif