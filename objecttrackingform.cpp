#include "objecttrackingform.h"
#include "ui_objecttrackingform.h"

ObjectTrackingForm::ObjectTrackingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ObjectTrackingForm)
{
    ui->setupUi(this);
}

ObjectTrackingForm::~ObjectTrackingForm()
{
    delete ui;
}

void ObjectTrackingForm::requestObjectTrackingParameters()
{
    objectTrackingParameters params;
    params.thresholdNoMove = ui->no_move_spinBox->value();
    params.thresholdLoitering = ui->loitering_move_spinBox->value();
    params.thresholdDirection = ui->direction_spinBox->value();
    params.segmentSize = ui->segment_size_direction_spinBox->value();
    emit setObjectTrackingParameters(params);
}

void ObjectTrackingForm::updateUI(const objectTrackingParameters &params)
{
    ui->no_move_spinBox->setValue(params.thresholdNoMove);
    ui->loitering_move_spinBox->setValue(params.thresholdLoitering);
    ui->direction_spinBox->setValue(params.thresholdDirection);
    ui->segment_size_direction_spinBox->setValue(params.segmentSize);
}

void ObjectTrackingForm::on_buttonBox_accepted()
{
    objectTrackingParameters params;
    params.thresholdNoMove = ui->no_move_spinBox->value();
    params.thresholdLoitering = ui->loitering_move_spinBox->value();
    params.thresholdDirection = ui->direction_spinBox->value();
    params.segmentSize = ui->segment_size_direction_spinBox->value();
    emit setObjectTrackingParameters(params);

    this->close();
}

void ObjectTrackingForm::on_buttonBox_rejected()
{
    this->close();
}
