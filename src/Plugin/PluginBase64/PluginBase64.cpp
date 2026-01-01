#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtGui/QScreen>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>

#include "Base64Helper.h"
#include "PluginBase64.h"

PluginBase64::PluginBase64(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PluginBase64Class())
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
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &PluginBase64::slot_pushButton_clear_clicked);
}

void PluginBase64::slot_pushButton_open_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择文件", "", "图片(*.img *.bmp *.jpg *.png)");
    if (!fileName.isEmpty())
    {
        ui->lineEdit_img->setText(fileName);
        QPixmap pix(fileName);
        pix = pix.scaled(ui->label_img->size() - QSize(4, 4), Qt::KeepAspectRatio);
        ui->label_img->setPixmap(pix);
    }
}

void PluginBase64::slot_pushButton_img_to_base64_clicked()
{
    QString fileName = ui->lineEdit_img->text().trimmed();
    if (fileName.isEmpty())
    {
        return;
    }

    QImage  image(fileName);
    QString text = Base64Helper::imageToBase64(image);
    ui->textEdit_base64->setText(text);

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

    QImage  image = Base64Helper::base64ToImage(text);
    QPixmap pix   = QPixmap::fromImage(image);
    pix           = pix.scaled(ui->label_img->size() - QSize(4, 4), Qt::KeepAspectRatio);
    ui->label_img->setPixmap(pix);

    auto record = QString("Base64 转换为图片").toStdString();
    m_eventBus->publish("log", new LogEvent(0, ET_LOG, Name(), __FILE__, __LINE__, __FUNCTION__, record));
}

void PluginBase64::slot_pushButton_clear_clicked()
{
    ui->lineEdit_img->clear();
    ui->label_img->clear();
    ui->textEdit_base64->clear();
}

// 插件创建函数
extern "C" PLUGINBASE64_API IPlugin* CreatePlugin()
{
    // 替换为你的插件类的实例化
    return new PluginBase64();
}
