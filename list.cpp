#include "list.h"
#include "ui_list.h"

list::list(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::list)
{
    ui->setupUi(this);
    // 设置水平表头的高度
    ui->tableWidget->horizontalHeader()->setFixedHeight(50);
    ui->tableWidget->verticalHeader()->setFixedWidth(35);
    // 设置水平表头的背景颜色
    QString horizontalHeaderStyle = "QHeaderView::section { background-color: #B0E2FF; }";
    ui->tableWidget->horizontalHeader()->setStyleSheet(horizontalHeaderStyle);

    // 设置垂直表头的背景颜色
    QString verticalHeaderStyle = "QHeaderView::section { background-color: #B0E2FF; }";
    ui->tableWidget->verticalHeader()->setStyleSheet(verticalHeaderStyle);
    // 设置行表头和列表头交叉点的背景颜色
    QString cornerButtonStyle = "QTableWidget::corner-button { background-color: red; }";
    ui->tableWidget->setStyleSheet(cornerButtonStyle);
    QFont font;
    font.setFamily("黑体"); // 设置字体为黑体
    font.setPointSize(12); // 设置字体大小
    font.setBold(true); // 设置字体为粗体

    // 设置表头字体
    ui->tableWidget->horizontalHeader()->setFont(font);
    ui->tableWidget->verticalHeader()->setFont(font);

}

list::~list()
{
    delete ui;
}

void list::on_pushButton_clicked()
{
    this->close();
}


void list::on_lineEdit_returnPressed()
{
    QString arg1 = ui->lineEdit->text();
    QList<QTableWidgetItem*> items = ui->tableWidget->findItems(arg1, Qt::MatchExactly);
    if(!items.empty())
    {
        // Clear previous selection
        ui->tableWidget->clearSelection();

        // Set selection mode and behavior
        ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

        // Select the row
        ui->tableWidget->selectRow(items.first()->row());

        // Highlight the row
        ui->tableWidget->setStyleSheet("QTableView::item:selected{background-color: yellow;}");
    }
}

