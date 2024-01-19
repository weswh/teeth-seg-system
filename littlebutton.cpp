#include "littlebutton.h"
#include "ui_littlebutton.h"

littlebutton::littlebutton(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::littlebutton)
{
    ui->setupUi(this);
}

littlebutton::~littlebutton()
{
    delete ui;
}
void littlebutton::on_newcase_clicked()
{
    emit signalnewcase();
}


void littlebutton::on_pushButton_2_clicked()
{
    emit signalcaselist();
}


void littlebutton::on_pushButton_clicked()
{
    emit signaltool();
}


void littlebutton::on_sin_clicked()
{
    emit signalsin();
}



void littlebutton::on_sup_clicked()
{
    emit signalsup();
}

