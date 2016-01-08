#include "whitelist.h"
#include "ui_whitelist.h"

WhiteList::WhiteList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WhiteList)
{
    ui->setupUi(this);
}

WhiteList::~WhiteList()
{
    delete ui;
}

void WhiteList::on_buttonBox_accepted()
{
    QString str = ui->white_list_lineEdit->text();
    str = str.trimmed();
    str = str.toUpper();
    QStringList whiteList = str.split(",");

    emit sendWhiteList(whiteList);

    this->close();

}

void WhiteList::on_buttonBox_rejected()
{
   this->close();
}
