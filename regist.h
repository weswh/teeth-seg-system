#ifndef REGIST_H
#define REGIST_H

#include "qmessagebox.h"
#include <QWidget>

namespace Ui {
class regist;
}

class regist : public QWidget
{
    Q_OBJECT

public:
    explicit regist(QWidget *parent = nullptr);
    ~regist();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::regist *ui;

};

#endif // REGIST_H
