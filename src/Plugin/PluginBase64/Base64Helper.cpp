#include "Base64Helper.h"

Base64Helper::Base64Helper(/* args */)
{
}

Base64Helper::~Base64Helper()
{
}

QString Base64Helper::imageToBase64(const QImage& image)
{
    return QString(imageToBase64x(image));
}

QByteArray Base64Helper::imageToBase64x(const QImage& image)
{
    QByteArray data;
    QBuffer    buffer(&data);
    image.save(&buffer, "JPG");
    data = data.toBase64();
    return data;
}

QImage Base64Helper::base64ToImage(const QString& data)
{
    return base64ToImagex(data.toUtf8());
}

QImage Base64Helper::base64ToImagex(const QByteArray& data)
{
    QImage image;
    image.loadFromData(QByteArray::fromBase64(data));
    return image;
}
