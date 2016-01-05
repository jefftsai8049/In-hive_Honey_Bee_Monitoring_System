#ifndef OBJECTTRACKINGFORM_H
#define OBJECTTRACKINGFORM_H

#include <QWidget>
#include <object_tracking.h>

namespace Ui {
class ObjectTrackingForm;
}

class ObjectTrackingForm : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectTrackingForm(QWidget *parent = 0);
    ~ObjectTrackingForm();

    void requestObjectTrackingParameters();

    void updateUI(const objectTrackingParameters &params);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::ObjectTrackingForm *ui;
signals:
    void setObjectTrackingParameters(const objectTrackingParameters &params);
};

#endif // OBJECTTRACKINGFORM_H
