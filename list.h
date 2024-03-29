#ifndef LIST_H
#define LIST_H

#include <QWidget>

namespace Ui {
class list;
}

class list : public QWidget
{
    Q_OBJECT

public:
    explicit list(QWidget *parent = nullptr);
    ~list();

private slots:
    void on_pushButton_clicked();

    void on_lineEdit_returnPressed();

private:
    Ui::list *ui;
};

#endif // LIST_H
