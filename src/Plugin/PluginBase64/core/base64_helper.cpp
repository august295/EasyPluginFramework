#include "base64_helper.h"

#include <QtCore/QFile>

Base64Helper::Base64Helper(/* args */)
{
}

Base64Helper::~Base64Helper()
{
}

QString Base64Helper::imageToBase64(const QImage& image, const QByteArray& format)
{
    return QString::fromLatin1(imageToBase64x(image, format));
}

QByteArray Base64Helper::imageToBase64x(const QImage& image, const QByteArray& format)
{
    QByteArray data;
    QBuffer    buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format.constData());
    data = data.toBase64();
    return data;
}

QImage Base64Helper::base64ToImage(const QString& data)
{
    return parseBase64Image(data).image;
}

QImage Base64Helper::base64ToImagex(const QByteArray& data)
{
    QImage image;
    image.loadFromData(QByteArray::fromBase64(data));
    return image;
}

QString Base64Helper::imageFileToBase64(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        return "";
    }
    return QString::fromLatin1(file.readAll().toBase64());
}

Base64ImageData Base64Helper::parseBase64Image(const QString& data)
{
    Base64ImageData imageData;
    QString normalizedData = data.trimmed();
    const int base64MarkerIndex = normalizedData.indexOf("base64,", 0, Qt::CaseInsensitive);
    if (normalizedData.startsWith("data:", Qt::CaseInsensitive) && base64MarkerIndex >= 0)
    {
        const QString header = normalizedData.left(base64MarkerIndex);
        const int slashIndex = header.indexOf('/');
        const int semicolonIndex = header.indexOf(';');
        if (slashIndex >= 0 && semicolonIndex > slashIndex)
        {
            imageData.format = header.mid(slashIndex + 1, semicolonIndex - slashIndex - 1).toLatin1().toUpper();
        }
        normalizedData = normalizedData.mid(base64MarkerIndex + 7);
    }
    imageData.rawData = QByteArray::fromBase64(normalizedData.toLatin1());
    imageData.image.loadFromData(imageData.rawData);
    if (imageData.format.isEmpty())
    {
        imageData.format = imageData.image.format() == QImage::Format_Invalid ? QByteArray() : QByteArray("PNG");
    }
    return imageData;
}
