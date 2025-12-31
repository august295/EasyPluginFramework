#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtGui/QScreen>
#include <QFileDialog>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>
#include <QStandardItemModel>
#include <QStandardItem>

#include "CertSM2Helper.h"
#include "PluginASN1View.h"

struct PluginASN1View::PluginASN1ViewPrivate
{
    QString             m_certFilePath;
    CertSM2Helper*      m_certSM2Helper;
    QStandardItemModel* m_modelField;
};

PluginASN1View::PluginASN1View(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PluginASN1ViewClass())
    , m_p(new PluginASN1ViewPrivate)
{
    ui->setupUi(this);
}

PluginASN1View::~PluginASN1View()
{
    if (m_p)
    {
        delete m_p->m_certSM2Helper;
        delete m_p->m_modelField;
    }
    delete m_p;
}

bool PluginASN1View::Init()
{
    QScreen* screen         = QGuiApplication::primaryScreen();
    QRect    screenGeometry = screen->geometry();
    resize(800, 1000);
    setWindowTitle(tr("ASN.1查看器"));
    setWindowIcon(QIcon(":/icons/tree.png"));

    this->InitWidget();
    return true;
}

bool PluginASN1View::InitAppFinish()
{
    return true;
}

bool PluginASN1View::Release()
{
    delete this;
    return true;
}

std::string PluginASN1View::Version()
{
    return "0.0.1";
}

std::string PluginASN1View::Description()
{
    return "ASN.1查看器";
}

std::string PluginASN1View::Icon()
{
    return "tree.png";
}

PluginLocation PluginASN1View::Location()
{
    return PluginLocation(PluginType::WIDGET, "ASN.1查看器", "证书", "工具");
}

void PluginASN1View::WidgetShow()
{
    show();
}

void PluginASN1View::InitWidget()
{
    
}

// 插件创建函数
extern "C" PluginASN1View_API IPlugin* CreatePlugin()
{
    // 替换为你的插件类的实例化
    return new PluginASN1View();
}
