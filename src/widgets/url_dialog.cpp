#include "url_dialog.h"


DWIDGET_USE_NAMESPACE

namespace dmr {
UrlDialog::UrlDialog()
    :DDialog(nullptr)
{
    addButtons(QStringList() << QApplication::translate("UrlDialog", "Cancel")
                                << QApplication::translate("UrlDialog", "Confirm"));
    setOnButtonClickedClose(false);
    setDefaultButton(1);
    setIcon(QIcon(":/resources/icons/logo-big.svg"));
    setMessage(QApplication::translate("UrlDialog", "Please input the url of file to play"));

    _le = new DLineEdit;
    addContent(_le);

    connect(getButton(0), &QAbstractButton::clicked, this, [=] {
        done(QDialog::Rejected);
    });
    connect(getButton(1), &QAbstractButton::clicked, this, [=] {
        done(QDialog::Accepted);
    });
}

QUrl UrlDialog::url() const
{
    return QUrl(_le->text());
}

}

