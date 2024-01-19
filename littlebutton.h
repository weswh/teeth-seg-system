#ifndef LITTLEBUTTON_H
#define LITTLEBUTTON_H

#include <QWidget>

namespace Ui {
class littlebutton;
}

class littlebutton : public QWidget
{
    Q_OBJECT

public:
    explicit littlebutton(QWidget *parent = nullptr);
    ~littlebutton();
signals:
    void signalnewcase();
    void signalcaselist();
    void signaltool();
    void signalsin();
    void signalsup();
private slots:
    void on_newcase_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_sin_clicked();


    void on_sup_clicked();

private:
    Ui::littlebutton *ui;
};

#endif // LITTLEBUTTON_H
