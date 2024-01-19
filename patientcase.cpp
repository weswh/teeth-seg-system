#include "patientcase.h"
#include "ui_patientcase.h"

patientcase::patientcase(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::patientcase)
{
    ui->setupUi(this);
    ui->widget->setStyleSheet("border-radius: 10px;");
//    this->setStyleSheet("border-radius: 10px;");
//    ui->lineEdit_2->setFixedHeight(45);
//    ui->lineEdit_2->setStyleSheet("background-color: ##E0EEEE;border-radius: 10px;font-size: 30px;");
}

patientcase::~patientcase()
{
    delete ui;
}

void patientcase::on_cancelpatient_clicked()
{
    this->close();
}


void patientcase::on_savepatient_clicked()
{
    this->close();
}


void patientcase::on_pushButton_clicked()
{
       this->close();
}

