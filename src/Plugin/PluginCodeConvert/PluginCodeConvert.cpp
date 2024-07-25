#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtGui/QIcon>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>

#include <Manager/DataManager.h>

#include "PluginCodeConvert.h"

struct PluginCodeConvert::PluginCodeConvertPrivate {
    QMap<QString, QTextCodec*> codec_map;
};

PluginCodeConvert::PluginCodeConvert(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PluginCodeConvertClass())
    , m_p(new PluginCodeConvertPrivate)
{
    ui->setupUi(this);
}

PluginCodeConvert::~PluginCodeConvert()
{
    m_p->codec_map.clear();
    delete m_p;
}

bool PluginCodeConvert::Init()
{
    setWindowTitle(tr("编码转换"));
    setWindowIcon(QIcon(":/icons/convert.png"));

    this->InitWidget();
    return true;
}

bool PluginCodeConvert::InitAppFinish()
{
    return true;
}

bool PluginCodeConvert::Release()
{
    delete this;
    return true;
}

std::string PluginCodeConvert::Version()
{
    return "0.0.1";
}

std::string PluginCodeConvert::Description()
{
    return "编码转换";
}

std::string PluginCodeConvert::Icon()
{
    return "convert.png";
}

PluginLocation PluginCodeConvert::Location()
{
    return PluginLocation(PluginType::WIDGET, "编码转换", "语言", "工具");
}

void PluginCodeConvert::WidgetShow()
{
    show();
}

void PluginCodeConvert::InitWidget()
{
    QStringList codec_list = {"GBK", "UTF-8"};
    ui->comboBox_lang_old->addItems(codec_list);
    ui->comboBox_lang_new->addItems(codec_list);

    connect(ui->pushButton_add, &QPushButton::clicked, this, &PluginCodeConvert::slot_pushButton_add);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &PluginCodeConvert::slot_pushButton_delete);
    connect(ui->pushButton_convert, &QPushButton::clicked, this, &PluginCodeConvert::slot_pushButton_convert);
}

void PluginCodeConvert::slot_pushButton_add()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more files to open");
    ui->listWidget->addItems(files);
}

void PluginCodeConvert::slot_pushButton_delete()
{
    QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    for (QListWidgetItem* item : selectedItems) {
        delete ui->listWidget->takeItem(ui->listWidget->row(item));
    }
}

void PluginCodeConvert::slot_pushButton_convert()
{
    QString     lang_old  = ui->comboBox_lang_old->currentText();
    QTextCodec* codec_old = nullptr;
    auto        iter_old  = m_p->codec_map.find(lang_old);
    if (iter_old != m_p->codec_map.end()) {
        codec_old = iter_old.value();
    } else {
        codec_old = QTextCodec::codecForName(lang_old.toUtf8());
        m_p->codec_map.insert(lang_old, codec_old);
    }

    QString     lang_new  = ui->comboBox_lang_new->currentText();
    QTextCodec* codec_new = nullptr;
    auto        iter_new  = m_p->codec_map.find(lang_new);
    if (iter_new != m_p->codec_map.end()) {
        codec_new = iter_new.value();
    } else {
        codec_new = QTextCodec::codecForName(lang_new.toUtf8());
        m_p->codec_map.insert(lang_new, codec_new);
    }

    DataManager::instance().Publish("log", QString("[%0 ==> %1]").arg(lang_old).arg(lang_new).toStdString());
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QString filename = ui->listWidget->item(i)->text();
        QFile   file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        QByteArray content    = file.readAll();
        QString    conten_old = codec_old->toUnicode(content);
        QByteArray conten_new = codec_new->fromUnicode(conten_old);

        QFile file_new(filename + "_convert");
        if (!file_new.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            continue;
        }
        file_new.write(conten_new);
        DataManager::instance().Publish("log", QString("\t[%0] ==> [%1]").arg(filename).arg(filename + "_convert").toStdString());
    }
}

// 插件创建函数
extern "C" PLUGINCODECONVERT_API IPlugin* CreatePlugin()
{
    // 替换为你的插件类的实例化
    return new PluginCodeConvert();
}
