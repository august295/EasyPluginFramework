#ifndef __BASE64_HELPER_H__
#define __BASE64_HELPER_H__

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtGui/QImage>

struct Base64ImageData
{
    QByteArray rawData;
    QByteArray format;
    QImage     image;
};

class Base64Helper
{
public:
    Base64Helper(/* args */);
    ~Base64Helper();

    // 图片转base64字符串
    static QString    imageToBase64(const QImage& image, const QByteArray& format = "PNG");
    static QByteArray imageToBase64x(const QImage& image, const QByteArray& format = "PNG");

    // base64字符串转图片
    static QImage base64ToImage(const QString& data);
    static QImage base64ToImagex(const QByteArray& data);

    // 文件转base64字符串
    static QString imageFileToBase64(const QString& fileName);

    // base64字符串转图片详细信息
    static Base64ImageData parseBase64Image(const QString& data);
};

#endif
