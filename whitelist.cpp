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
    QString controlStr = ui->white_list_control_lineEdit->text();
    controlStr = controlStr.trimmed();
    controlStr = controlStr.toUpper();

    QString experimentStr = ui->white_list_experiment_lineEdit->text();
    experimentStr = experimentStr.trimmed();
    experimentStr = experimentStr.toUpper();

    QStringList controlWhiteList = controlStr.split(",");
    QStringList experimentWhiteList = experimentStr.split(",");

    emit sendWhiteList(controlWhiteList,experimentWhiteList);

    this->close();

}

void WhiteList::on_buttonBox_rejected()
{
   this->close();
}
