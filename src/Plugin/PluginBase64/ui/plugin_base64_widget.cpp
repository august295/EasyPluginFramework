#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtGui/QImageReader>
#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <QtGui/QScreen>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

#include "plugin_base64_widget.h"

#include "../core/base64_helper.h"

PluginBase64::PluginBase64(QWidget* parent)
    : QWidget(parent)
    , m_currentImage()
    , m_currentImageBytes()
    , m_currentImageFormat()
    , m_currentFileName()
    , ui(new Ui::PluginBase64WidgetClass())
{
    m_eventBus = nullptr;
    ui->setupUi(this);
}

PluginBase64::~PluginBase64()
{
}

bool PluginBase64::Init()
{
    m_eventBus = GetEventBus();

    QScreen* screen         = QGuiApplication::primaryScreen();
    QRect    screenGeometry = screen->geometry();
    resize(screenGeometry.width() / 2, screenGeometry.height() / 2);
    setWindowTitle(tr("图片Base64转换"));
    setWindowIcon(QIcon(":/icons/base64.png"));

    this->InitWidget();
    return true;
}

bool PluginBase64::InitAppFinish()
{
    std::string record = "插件初始化完成";
    m_eventBus->publish("log", new LogEvent(0, ET_LOG, Name(), __FILE__, __LINE__, __FUNCTION__, record));
    return true;
}

bool PluginBase64::Release()
{
    delete this;
    return true;
}

std::string PluginBase64::Version()
{
    return "0.0.1";
}

std::string PluginBase64::Name()
{
    return "PluginBase64";
}

std::string PluginBase64::Description()
{
    return "图片Base64转换";
}

std::string PluginBase64::Icon()
{
    return "base64.png";
}

PluginLocation PluginBase64::Location()
{
    return PluginLocation(PluginType::WIDGET, "图片Base64转换", "图片", "工具");
}

void PluginBase64::WidgetShow()
{
    show();
}

void PluginBase64::OnEvent(const Event* event)
{
    switch (event->type)
    {
    case ET_MESSAGE: {
        const MessageEvent* me = dynamic_cast<const MessageEvent*>(event);
        printf("%s\n", me->message.c_str());
    }
    break;
    default:
        break;
    }
}

void PluginBase64::InitWidget()
{
    connect(ui->pushButton_open, &QPushButton::clicked, this, &PluginBase64::slot_pushButton_open_clicked);
    connect(ui->pushButton_img_to_base64, &QPushButton::clicked, this, &PluginBase64::slot_pushButton_img_to_base64_clicked);
    connect(ui->pushButton_base64_to_img, &QPushButton::clicked, this, &PluginBase64::slot_pushButton_base64_to_img_clicked);
    connect(ui->pushButton_save, &QPushButton::clicked, this, &PluginBase64::slot_pushButton_save_clicked);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &PluginBase64::slot_pushButton_clear_clicked);
    ui->label_img->setMinimumSize(240, 240);
    ui->label_img->setStyleSheet("QLabel { border: 1px solid #c8c8c8; background: #fafafa; }");
    UpdateInfoText("图片信息：\n- 暂无", "Base64信息：\n- 暂无");
}

void PluginBase64::slot_pushButton_open_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择文件", "", "图片(*.bmp *.jpg *.jpeg *.png *.gif *.webp)");
    if (!fileName.isEmpty())
    {
        QImage image(fileName);
        if (image.isNull())
        {
            QMessageBox::warning(this, tr("提示"), tr("图片加载失败"));
            return;
        }
        ui->lineEdit_img->setText(fileName);
        m_currentFileName = fileName;
        m_currentImage = image;
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
            m_currentImageBytes = file.readAll();
        }
        m_currentImageFormat = DetectImageFormat(fileName);
        UpdatePreviewImage(m_currentImage);
        UpdateInfoText(BuildImageInfo(m_currentImage, m_currentImageFormat, m_currentFileName), BuildBase64Info(m_currentImageBytes, ui->textEdit_base64->toPlainText()));
    }
}

void PluginBase64::slot_pushButton_img_to_base64_clicked()
{
    QString fileName = ui->lineEdit_img->text().trimmed();
    if (fileName.isEmpty())
    {
        return;
    }

    if (m_currentImage.isNull())
    {
        QImage image(fileName);
        if (image.isNull())
        {
            QMessageBox::warning(this, tr("提示"), tr("图片加载失败"));
            return;
        }
        m_currentImage = image;
        m_currentFileName = fileName;
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
            m_currentImageBytes = file.readAll();
        }
        m_currentImageFormat = DetectImageFormat(fileName);
    }
    QString text;
    if (!m_currentImageBytes.isEmpty())
    {
        text = QString::fromLatin1(m_currentImageBytes.toBase64());
    }
    else
    {
        text = Base64Helper::imageToBase64(m_currentImage, m_currentImageFormat.isEmpty() ? QByteArray("PNG") : m_currentImageFormat);
    }
    ui->textEdit_base64->setText(text);
    UpdatePreviewImage(m_currentImage);
    UpdateInfoText(BuildImageInfo(m_currentImage, m_currentImageFormat, m_currentFileName), BuildBase64Info(m_currentImageBytes, text));

    auto record = QString("%1 转换为 Base64").arg(fileName).toStdString();
    m_eventBus->publish("log", new LogEvent(0, ET_LOG, Name(), __FILE__, __LINE__, __FUNCTION__, record));
}

void PluginBase64::slot_pushButton_base64_to_img_clicked()
{
    QString text = ui->textEdit_base64->toPlainText().trimmed();
    if (text.isEmpty())
    {
        return;
    }
    const Base64ImageData imageData = Base64Helper::parseBase64Image(text);
    if (imageData.image.isNull())
    {
        QMessageBox::warning(this, tr("提示"), tr("Base64 数据不是有效图片"));
        return;
    }
    m_currentImage = imageData.image;
    m_currentImageBytes = imageData.rawData;
    m_currentImageFormat = imageData.format;
    m_currentFileName.clear();
    UpdatePreviewImage(m_currentImage);
    UpdateInfoText(BuildImageInfo(m_currentImage, m_currentImageFormat), BuildBase64Info(m_currentImageBytes, text));

    auto record = QString("Base64 转换为图片").toStdString();
    m_eventBus->publish("log", new LogEvent(0, ET_LOG, Name(), __FILE__, __LINE__, __FUNCTION__, record));
}

void PluginBase64::slot_pushButton_save_clicked()
{
    if (m_currentImage.isNull())
    {
        QMessageBox::information(this, tr("提示"), tr("当前没有可保存的图片"));
        return;
    }
    const QString defaultSuffix = m_currentImageFormat.isEmpty() ? "png" : QString::fromLatin1(m_currentImageFormat).toLower();
    const QString defaultName = m_currentFileName.isEmpty() ? tr("base64_image.%1").arg(defaultSuffix) : QFileInfo(m_currentFileName).completeBaseName() + "_decoded." + defaultSuffix;
    const QString fileName = QFileDialog::getSaveFileName(this, tr("保存图片"), defaultName, tr("图片(*.png *.jpg *.jpeg *.bmp *.gif *.webp)"));
    if (fileName.isEmpty())
    {
        return;
    }
    const QByteArray targetFormat = DetectImageFormat(fileName);
    bool isSaved = false;
    if (!m_currentImageBytes.isEmpty() && !targetFormat.isEmpty() && targetFormat == m_currentImageFormat)
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
            isSaved = file.write(m_currentImageBytes) == m_currentImageBytes.size();
        }
    }
    if (!isSaved)
    {
        isSaved = m_currentImage.save(fileName, targetFormat.isEmpty() ? "PNG" : targetFormat.constData());
    }
    if (!isSaved)
    {
        QMessageBox::warning(this, tr("提示"), tr("保存图片失败"));
        return;
    }
    auto record = QString("图片已保存到 %1").arg(fileName).toStdString();
    m_eventBus->publish("log", new LogEvent(0, ET_LOG, Name(), __FILE__, __LINE__, __FUNCTION__, record));
}

void PluginBase64::slot_pushButton_clear_clicked()
{
    ClearData();
}

void PluginBase64::UpdatePreviewImage(const QImage& image)
{
    if (image.isNull())
    {
        ui->label_img->setText("+");
        ui->label_img->setPixmap(QPixmap());
        return;
    }
    QPixmap pix = QPixmap::fromImage(image);
    pix = pix.scaled(ui->label_img->size() - QSize(4, 4), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_img->setText("");
    ui->label_img->setPixmap(pix);
}

void PluginBase64::UpdateInfoText(const QString& imageInfo, const QString& base64Info)
{
    ui->label_image_info->setText(imageInfo);
    ui->label_base64_info->setText(base64Info);
}

void PluginBase64::ClearData()
{
    m_currentImage = QImage();
    m_currentImageBytes.clear();
    m_currentImageFormat.clear();
    m_currentFileName.clear();
    ui->lineEdit_img->clear();
    ui->textEdit_base64->clear();
    UpdatePreviewImage(QImage());
    UpdateInfoText("图片信息：\n- 暂无", "Base64信息：\n- 暂无");
}

QString PluginBase64::BuildImageInfo(const QImage& image, const QByteArray& format, const QString& fileName) const
{
    if (image.isNull())
    {
        return "图片信息：\n- 暂无";
    }
    QStringList lines;
    lines << "图片信息：";
    if (!fileName.isEmpty())
    {
        lines << QString("- 文件：%1").arg(fileName);
    }
    lines << QString("- 尺寸：%1 x %2").arg(image.width()).arg(image.height());
    lines << QString("- 格式：%1").arg(format.isEmpty() ? "未知" : QString::fromLatin1(format).toUpper());
    lines << QString("- 深度：%1 bit").arg(image.depth());
    lines << QString("- 字节数：%1").arg(m_currentImageBytes.isEmpty() ? image.sizeInBytes() : m_currentImageBytes.size());
    return lines.join('\n');
}

QString PluginBase64::BuildBase64Info(const QByteArray& rawData, const QString& base64Text) const
{
    QStringList lines;
    lines << "Base64信息：";
    lines << QString("- 原始字节数：%1").arg(rawData.size());
    lines << QString("- Base64长度：%1").arg(base64Text.size());
    if (rawData.isEmpty())
    {
        lines << "- 压缩率：暂无";
    }
    else
    {
        const double ratio = static_cast<double>(base64Text.size()) / static_cast<double>(rawData.size());
        lines << QString("- 长度倍率：%1").arg(QString::number(ratio, 'f', 2));
    }
    return lines.join('\n');
}

QByteArray PluginBase64::DetectImageFormat(const QString& fileName) const
{
    const QByteArray format = QFileInfo(fileName).suffix().toLatin1().toUpper();
    return format == "JPEG" ? QByteArray("JPG") : format;
}

// 插件创建函数
extern "C" PLUGINBASE64_API IPlugin* CreatePlugin()
{
    // 替换为你的插件类的实例化
    return new PluginBase64();
}
