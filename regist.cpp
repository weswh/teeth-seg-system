#include "regist.h"
#include "ui_regist.h"
#include <QMessageBox>

regist::regist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::regist)
{
    ui->setupUi(this);
}

regist::~regist()
{
    delete ui;
}

void regist::on_pushButton_clicked()
{
  QMessageBox::information(this, "regist", "success");
}


void regist::on_pushButton_2_clicked()
{
  this->close();
}

