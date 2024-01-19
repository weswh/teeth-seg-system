#ifndef PATIENTCASE_H
#define PATIENTCASE_H

#include <QWidget>

namespace Ui {
class patientcase;
}

class patientcase : public QWidget
{
    Q_OBJECT

public:
    explicit patientcase(QWidget *parent = nullptr);
    ~patientcase();

private slots:
    void on_cancelpatient_clicked();

    void on_savepatient_clicked();

    void on_pushButton_clicked();

private:
    Ui::patientcase *ui;
};

#endif // PATIENTCASE_H
