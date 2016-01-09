#ifndef WHITELIST_H
#define WHITELIST_H

#include <QDialog>
#include <QString>
#include <QStringList>


namespace Ui {
class WhiteList;
}

class WhiteList : public QDialog
{
    Q_OBJECT

public:
    explicit WhiteList(QWidget *parent = 0);
    ~WhiteList();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::WhiteList *ui;

signals:
    void sendWhiteList(const QStringList &controlWhiteList,const QStringList &experimentWhiteList);
};

#endif // WHITELIST_H
