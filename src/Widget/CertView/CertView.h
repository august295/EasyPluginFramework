#ifndef __CERTVIEW_H__
#define __CERTVIEW_H__

#include "ui_CertView.h"

#include "CertX509Helper.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class CertViewClass;
};
QT_END_NAMESPACE

class CertView
    : public QWidget
{
    Q_OBJECT

public:
    enum TabType
    {
        TAB_BASE,
        TAB_DETAIL,
    };

public:
    CertView(QWidget* parent = nullptr);
    ~CertView();

    void Init();
    void InitWidget();
    void OpenCertDialog();
    void ShowTabBase();
    void ShowTabDetail();

public slots:

private:
    Ui::CertViewClass* ui;

    struct CertViewPrivate;
    CertViewPrivate* m_p;
};

#endif
