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

#include <iostream>
#include <openssl/applink.c> // qt 需要

#include "CertView.h"

struct CertView::CertViewPrivate
{
    QString             m_certFilePath;
    CertX509Helper*     m_certX509Helper;
    QStandardItemModel* m_modelRoute;
    QStandardItemModel* m_modelField;
    bool                m_chainBuild;
};

CertView::CertView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CertViewClass())
    , m_p(new CertViewPrivate)
{
    ui->setupUi(this);
    m_p->m_certX509Helper = new CertX509Helper;
    m_p->m_modelRoute     = new QStandardItemModel;
    m_p->m_modelField     = new QStandardItemModel;
    m_p->m_chainBuild     = true;

    Init();
}

CertView::~CertView()
{
    if (m_p)
    {
        delete m_p->m_certX509Helper;
        delete m_p->m_modelRoute;
        delete m_p->m_modelField;
    }
    delete m_p;
}

void CertView::Init()
{
    QScreen* screen         = QGuiApplication::primaryScreen();
    QRect    screenGeometry = screen->geometry();
    resize(540, 660);
    setWindowTitle(tr("证书查看器"));
    setWindowIcon(QIcon(":/icons/cert.png"));

    this->InitWidget();
}

void CertView::InitWidget()
{
    QMenuBar* menubar    = new QMenuBar(this);
    QMenu*    fileMenu   = menubar->addMenu("文件");
    QAction*  openAction = new QAction("打开", this);
    connect(openAction, &QAction::triggered, this, &CertView::OpenCertDialog);
    fileMenu->addAction(openAction);
    layout()->setMenuBar(menubar);

    // 颁发对象
    ui->label_subject_CN->setText("");
    ui->label_subject_O->setText("");
    ui->label_subject_OU->setText("");

    // 颁发者
    ui->label_issuer_CN->setText("");
    ui->label_issuer_O->setText("");
    ui->label_issuer_OU->setText("");

    // 有效期
    ui->label_validity_notbefore->setText("");
    ui->label_validity_notafter->setText("");

    // 指纹
    ui->groupBox_signature->hide();
    ui->label_signature_name_algorithm->setText("未知");
    ui->label_signature_name_value->setText("未知");
    ui->label_signature_algorithm->setText("");
    ui->label_signature_value->setText("");

    ui->tabWidget->setCurrentIndex(TAB_BASE);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [&](int index) {
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

void CertView::OpenCertDialog()
{
    m_p->m_certFilePath = QFileDialog::getOpenFileName(
        this,                            // 父窗口
        "选择文件",                      // 对话框标题
        "",                              // 初始目录（空字符串表示默认目录）
        "证书文件 (*.cer);;所有文件 (*)" // 文件过滤器
    );
    m_p->m_certX509Helper->loadFromFile(m_p->m_certFilePath.toStdString());
    m_p->m_chainBuild = true;
    m_p->m_modelRoute->clear();
    m_p->m_modelField->clear();
    ShowTabBase();
    ShowTabDetail();
}

void CertView::ShowTabBase()
{
    CertX509Helper* certX509Helper = m_p->m_certX509Helper;

    std::string subject    = certX509Helper->getSubject();
    std::string subject_cn = certX509Helper->Capture(subject, "CN =", ",");
    std::string subject_o  = certX509Helper->Capture(subject, "O =", ",");
    std::string subject_ou = certX509Helper->Capture(subject, "OU =", ",");
    std::string issuer     = certX509Helper->getIssuer();
    std::string issuer_cn  = certX509Helper->Capture(issuer, "CN =", ",");
    std::string issuer_o   = certX509Helper->Capture(issuer, "O =", ",");
    std::string issuer_ou  = certX509Helper->Capture(issuer, "OU =", ",");
    std::string not_before = certX509Helper->getNotBefore();
    std::string not_after  = certX509Helper->getNotAfter();
    ui->label_subject_CN->setText(QString::fromStdString(subject_cn));
    ui->label_subject_O->setText(QString::fromStdString(subject_o));
    ui->label_subject_OU->setText(QString::fromStdString(subject_ou));
    ui->label_issuer_CN->setText(QString::fromStdString(issuer_cn));
    ui->label_issuer_O->setText(QString::fromStdString(issuer_o));
    ui->label_issuer_OU->setText(QString::fromStdString(issuer_ou));
    ui->label_validity_notbefore->setText(QString::fromStdString(not_before));
    ui->label_validity_notafter->setText(QString::fromStdString(not_after));
}

void CertView::ShowTabDetail()
{
    CertX509Helper* certX509Helper = m_p->m_certX509Helper;
    if (!certX509Helper->isLoadCert())
    {
        return;
    }

    std::string subject    = certX509Helper->getSubject();
    std::string subject_cn = certX509Helper->Capture(subject, "CN =", ",");
    // 证书结构
    if (m_p->m_chainBuild)
    {
        m_p->m_modelRoute->clear();
        m_p->m_chainBuild = false;

        QStandardItem* routeRoot = m_p->m_modelRoute->invisibleRootItem();
        if (!certX509Helper->buildChain())
        {
            QStandardItem* routeLeaf = new QStandardItem(QString::fromStdString(subject_cn));
            routeRoot->appendRow(routeLeaf);
        }
        else
        {
            QStandardItem* parent = nullptr;
            QStandardItem* child  = nullptr;
            auto           chain  = certX509Helper->getChain();
            for (auto iter = chain.begin(); iter != chain.end(); iter++)
            {
                std::string subjectChain   = certX509Helper->getSubject(*iter);
                std::string subjectChainCn = certX509Helper->Capture(subjectChain, "CN =", ",");
                parent                     = new QStandardItem(QString::fromStdString(subjectChainCn));
                if (child)
                {
                    parent->appendRow(child);
                }
                child = parent;
            }
            routeRoot->appendRow(parent);
        }
        ui->treeView_route->setModel(m_p->m_modelRoute);
        ui->treeView_route->setHeaderHidden(true);
        ui->treeView_route->expandAll();
    }

    // 证书字段
    m_p->m_modelField->clear();
    QStandardItem* itemRoot = m_p->m_modelField->invisibleRootItem();
    QStandardItem* itemCN   = new QStandardItem(QString::fromStdString(subject_cn));
    itemRoot->appendRow(itemCN);

    {
        QStandardItem* itemCert = new QStandardItem(tr("证书"));
        itemCN->appendRow(itemCert);

        QStandardItem* itemVersion = new QStandardItem(tr("版本"));
        itemCert->appendRow(itemVersion);
        std::string version = certX509Helper->getVersion();
        itemVersion->setData(QString::fromStdString(version), Qt::UserRole);

        QStandardItem* itemSerialNumber = new QStandardItem(tr("序列号"));
        itemCert->appendRow(itemSerialNumber);
        std::string serialNumber = certX509Helper->getSerialNumber();
        itemSerialNumber->setData(QString::fromStdString(serialNumber), Qt::UserRole);

        QStandardItem* itemSignature      = new QStandardItem(tr("签名算法"));
        std::string    signatureAlgorithm = certX509Helper->getSignatureAlgorithm();
        itemSignature->setData(QString::fromStdString(signatureAlgorithm), Qt::UserRole);
        itemCert->appendRow(itemSignature);

        QStandardItem* itemIssuer = new QStandardItem(tr("颁发者"));
        itemCert->appendRow(itemIssuer);
        std::string issuer  = certX509Helper->getIssuer();
        QString     qIssuer = QString::fromStdString(issuer).replace(", ", "\r\n");
        itemIssuer->setData(qIssuer, Qt::UserRole);

        QStandardItem* itemValidity = new QStandardItem(tr("有效期"));
        itemCert->appendRow(itemValidity);
        QStandardItem* itemNotBefore = new QStandardItem(tr("不早于"));
        std::string    notBefore     = certX509Helper->getNotAfter();
        QString        qNotBefore    = QString::fromStdString(notBefore);
        itemNotBefore->setData(qNotBefore, Qt::UserRole);
        itemValidity->appendRow(itemNotBefore);
        QStandardItem* itemNotAfter = new QStandardItem(tr("不晚于"));
        std::string    notAfter     = certX509Helper->getNotBefore();
        QString        qNotAfter    = QString::fromStdString(notAfter);
        itemNotAfter->setData(qNotAfter, Qt::UserRole);
        itemValidity->appendRow(itemNotAfter);

        QStandardItem* itemSubject = new QStandardItem(tr("使用者"));
        itemCert->appendRow(itemSubject);
        QString qSubject = QString::fromStdString(subject).replace(", ", "\r\n");
        itemSubject->setData(qSubject, Qt::UserRole);

        QStandardItem* itemSubjectPublicKeyInfo = new QStandardItem(tr("使用者公钥信息"));
        itemCert->appendRow(itemSubjectPublicKeyInfo);
        QStandardItem* itemSubjectPublicKeyAlgorithm = new QStandardItem(tr("使用者公钥算法"));
        QStandardItem* itemSubjectPublicKey          = new QStandardItem(tr("使用者公钥"));
        QString        qSubjectPublicKeyAlgorithm    = QString::fromStdString(certX509Helper->getPublicKeyAlgorithm());
        QString        qSubjectPublicKey             = QString::fromStdString(certX509Helper->getPublicKeyValue());
        itemSubjectPublicKeyAlgorithm->setData(qSubjectPublicKeyAlgorithm, Qt::UserRole);
        itemSubjectPublicKey->setData(qSubjectPublicKey, Qt::UserRole);
        itemSubjectPublicKeyInfo->appendRow(itemSubjectPublicKeyAlgorithm);
        itemSubjectPublicKeyInfo->appendRow(itemSubjectPublicKey);

        QStandardItem* itemExtensions = new QStandardItem(tr("扩展"));
        itemCert->appendRow(itemExtensions);
        auto exts = certX509Helper->parseExtensions();
        for (auto ext : exts)
        {
            std::string    key     = ext.first;
            std::string    value   = ext.second;
            QStandardItem* itemExt = new QStandardItem(QString::fromStdString(key));
            itemExt->setData(QString::fromStdString(value), Qt::UserRole);
            itemExtensions->appendRow(itemExt);
        }
    }

    QStandardItem* itemOutSignature      = new QStandardItem(tr("签名算法"));
    std::string    outSignatureAlgorithm = certX509Helper->getSignatureAlgorithm();
    itemOutSignature->setData(QString::fromStdString(outSignatureAlgorithm), Qt::UserRole);
    itemCN->appendRow(itemOutSignature);
    QStandardItem* itemOutSignatureValue = new QStandardItem(tr("签名值"));
    std::string    outSignatureValue     = certX509Helper->getSignatureValue();
    itemOutSignatureValue->setData(QString::fromStdString(outSignatureValue), Qt::UserRole);
    itemCN->appendRow(itemOutSignatureValue);

    ui->treeView_field->setModel(m_p->m_modelField);
    ui->treeView_field->setHeaderHidden(true);
    ui->treeView_field->expandAll();

    connect(ui->treeView_field, &QTreeView::clicked, this, [&](const QModelIndex& index) {
        QString value = index.data(Qt::UserRole).toString();
        ui->textEdit_fieldValue->setText(value);
    });
}
