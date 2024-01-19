#include "signin.h"
#include "ui_signin.h"
#include <QMessageBox>

signin::signin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::signin)
{
    ui->setupUi(this);
}

signin::~signin()
{
    delete ui;
}

void signin::on_pushButton_clicked()
{
    emit signalsignin();
    QMessageBox::information(this, "login", "success");
}


void signin::on_pushButton_3_clicked()
{
    emit signalsignup();

}

