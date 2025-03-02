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

#include <Manager/DataManager.h>

#include "CertSM2Helper.h"
#include "PluginCertView.h"

struct PluginCertView::PluginCertViewPrivate
{
    QString             m_certFilePath;
    CertSM2Helper*      m_certSM2Helper;
    QStandardItemModel* m_modelField;
};

PluginCertView::PluginCertView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PluginCertViewClass())
    , m_p(new PluginCertViewPrivate)
{
    ui->setupUi(this);
    m_p->m_certSM2Helper = new CertSM2Helper;
    m_p->m_modelField    = new QStandardItemModel;
}

PluginCertView::~PluginCertView()
{
    if (m_p)
    {
        delete m_p->m_certSM2Helper;
        delete m_p->m_modelField;
    }
    delete m_p;
}

bool PluginCertView::Init()
{
    QScreen* screen         = QGuiApplication::primaryScreen();
    QRect    screenGeometry = screen->geometry();
    resize(screenGeometry.width() / 2, screenGeometry.height() / 2);
    setWindowTitle(tr("证书查看器"));
    setWindowIcon(QIcon(":/icons/cert.png"));

    this->InitWidget();
    return true;
}

bool PluginCertView::InitAppFinish()
{
    return true;
}

bool PluginCertView::Release()
{
    delete this;
    return true;
}

std::string PluginCertView::Version()
{
    return "0.0.1";
}

std::string PluginCertView::Description()
{
    return "证书查看器";
}

std::string PluginCertView::Icon()
{
    return "cert.png";
}

PluginLocation PluginCertView::Location()
{
    return PluginLocation(PluginType::WIDGET, "证书查看器", "证书", "工具");
}

void PluginCertView::WidgetShow()
{
    show();
}

void PluginCertView::InitWidget()
{
    QMenuBar* menubar    = new QMenuBar(this);
    QMenu*    fileMenu   = menubar->addMenu("文件");
    QAction*  openAction = new QAction("打开", this);
    connect(openAction, &QAction::triggered, this, &PluginCertView::OpenCertDialog);
    fileMenu->addAction(openAction);
    layout()->setMenuBar(menubar);

    ui->label_subject_CN->setText("");
    ui->label_subject_O->setText("");
    ui->label_subject_OU->setText("");
    ui->label_issuer_CN->setText("");
    ui->label_issuer_O->setText("");
    ui->label_issuer_OU->setText("");
    ui->label_validity_notbefore->setText("");
    ui->label_validity_notafter->setText("");
    ui->label_signature_name_algorithm->setText("未知");
    ui->label_signature_name_value->setText("未知");
    ui->label_signature_algorithm->setText("");
    ui->label_signature_value->setText("");

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [&](int index) {
        if (!m_p->m_certSM2Helper->IsParse())
        {
            return;
        }
        if (index == TAB_BASE)
        {
            ShowTabBase();
        }
        else if (index == TAB_DETAIL)
        {
            ShowTabDetail();
        }
    });
}

void PluginCertView::OpenCertDialog()
{
    m_p->m_certFilePath = QFileDialog::getOpenFileName(
        this,                            // 父窗口
        "选择文件",                      // 对话框标题
        "",                              // 初始目录（空字符串表示默认目录）
        "证书文件 (*.cer);;所有文件 (*)" // 文件过滤器
    );

    m_p->m_certSM2Helper->ParseCertSM2(m_p->m_certFilePath.toStdString());
    ShowTabBase();
}

void PluginCertView::ShowTabBase()
{
    CertSM2Helper* certSM2Helper = m_p->m_certSM2Helper;

    std::string subject_cn = certSM2Helper->GetCertInfo(SGD_CERT_SUBJECT_CN);
    std::string subject_o  = certSM2Helper->GetCertInfo(SGD_CERT_SUBJECT_O);
    std::string subject_ou = certSM2Helper->GetCertInfo(SGD_CERT_SUBJECT_OU);
    std::string issuer_cn  = certSM2Helper->GetCertInfo(SGD_CERT_ISSUER_CN);
    std::string issuer_o   = certSM2Helper->GetCertInfo(SGD_CERT_ISSUER_O);
    std::string issuer_ou  = certSM2Helper->GetCertInfo(SGD_CERT_ISSUER_OU);
    std::string not_before = certSM2Helper->GetCertInfo(SGD_CERT_NOTBEFORE_TIME);
    std::string not_after  = certSM2Helper->GetCertInfo(SGD_CERT_NOTAFTER_TIME);
    ui->label_subject_CN->setText(QString::fromStdString(subject_cn));
    ui->label_subject_O->setText(QString::fromStdString(subject_o));
    ui->label_subject_OU->setText(QString::fromStdString(subject_ou));
    ui->label_issuer_CN->setText(QString::fromStdString(issuer_cn));
    ui->label_issuer_O->setText(QString::fromStdString(issuer_o));
    ui->label_issuer_OU->setText(QString::fromStdString(issuer_ou));
    ui->label_validity_notbefore->setText(QString::fromStdString(not_before));
    ui->label_validity_notafter->setText(QString::fromStdString(not_after));
}

void PluginCertView::ShowTabDetail()
{
    m_p->m_modelField->clear();

    CertSM2Helper* certSM2Helper = m_p->m_certSM2Helper;

    std::string    subject_cn = certSM2Helper->GetCertInfo(SGD_CERT_SUBJECT_CN);
    QStandardItem* itemRoot   = m_p->m_modelField->invisibleRootItem();
    QStandardItem* itemCN     = new QStandardItem(QString::fromStdString(subject_cn));
    itemRoot->appendRow(itemCN);

    QStandardItem* itemCert = new QStandardItem(tr("证书"));
    itemCN->appendRow(itemCert);

    QStandardItem* itemVersion = new QStandardItem(tr("版本"));
    itemCert->appendRow(itemVersion);
    std::string version = certSM2Helper->GetCertInfo(SGD_CERT_VERSION);
    itemVersion->setData(QString::fromStdString(version), Qt::UserRole);

    QStandardItem* itemSerialNumber = new QStandardItem(tr("序列号"));
    itemCert->appendRow(itemSerialNumber);
    std::string serialNumber = certSM2Helper->GetCertInfo(SGD_CERT_SERIAL);
    itemSerialNumber->setData(QString::fromStdString(serialNumber), Qt::UserRole);

    QStandardItem* itemSignature = new QStandardItem(tr("签名算法"));
    itemCert->appendRow(itemSignature);

    QStandardItem* itemIssuer = new QStandardItem(tr("颁发者"));
    itemCert->appendRow(itemIssuer);
    std::string issuer  = certSM2Helper->GetCertInfo(SGD_CERT_ISSUER);
    QString     qIssuer = QString::fromStdString(issuer).replace(";", "\r\n");
    itemIssuer->setData(qIssuer, Qt::UserRole);

    QStandardItem* itemValidity = new QStandardItem(tr("有效期"));
    itemCert->appendRow(itemValidity);

    QStandardItem* itemSubject = new QStandardItem(tr("使用者"));
    itemCert->appendRow(itemSubject);
    std::string subject  = certSM2Helper->GetCertInfo(SGD_CERT_SUBJECT);
    QString     qSubject = QString::fromStdString(subject).replace(";", "\r\n");
    itemSubject->setData(qSubject, Qt::UserRole);

    ui->treeView_field->setModel(m_p->m_modelField);
    ui->treeView_field->setHeaderHidden(true);
    ui->treeView_field->expandAll();

    connect(ui->treeView_field, &QTreeView::clicked, this, [&](const QModelIndex& index) {
        QString value = index.data(Qt::UserRole).toString();
        ui->textEdit_fieldValue->setText(value);
    });
}

// 插件创建函数
extern "C" PLUGINCERTVIEW_API IPlugin* CreatePlugin()
{
    // 替换为你的插件类的实例化
    return new PluginCertView();
}
