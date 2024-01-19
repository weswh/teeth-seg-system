#include "ToolMenuWidget.h"

#include "ui_ToolMenuWidget.h"
#include <QDebug>
#include <QFileDialog>
#include <QButtonGroup>
#include "TeethColor.h"

ToolMenuWidget::ToolMenuWidget(QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::ToolMenuWidget)
    , mSettings("MyCompany", "MyApp")
{
    ui->setupUi(this);

    initWidget();

    connect(ui->chooseOnnxBtn, &QToolButton::clicked, this, &ToolMenuWidget::slotchooseOnnxBtn);
    connect(ui->chooseDentalBtn, &QToolButton::clicked, this, &ToolMenuWidget::slotChooseDentalBtn);
    connect(ui->loadOnnxBtn, &QPushButton::clicked, this, [&](){
        emit signalLoadOnnxModel(mOnnxPathList.at(ui->onnxPathCombo->currentIndex()));
    });
    connect(ui->dentalModelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&]() {
        emit signalLoadDentalModel(mDentalPathList.at(ui->dentalModelCombo->currentIndex()));
    });
    connect(ui->segmentBtn,&QPushButton::clicked, this, &ToolMenuWidget::signalStartSegment);
    connect(ui->fillColorBtn,&QToolButton::clicked, this, &ToolMenuWidget::signalEnableBrush);
    connect(ui->undoBtn,&QToolButton::clicked, this, &ToolMenuWidget::signalUndoBrush);
    connect(ui->redoBtn,&QToolButton::clicked, this, &ToolMenuWidget::signalRedoBrush);


    connect(ui->exportModelBtn,&QToolButton::clicked, this, &ToolMenuWidget::signalExport);
    connect(ui->toTeethAlignBtn,&QToolButton::clicked, this, &ToolMenuWidget::signalToTeethAlignStep);
    connect(ui->lambda, &QLineEdit::returnPressed, this, &ToolMenuWidget::lambdaPressed);
    connect(ui->round, &QLineEdit::returnPressed, this, &ToolMenuWidget::roundpressed);
    connect(ui->adjust,&QPushButton::clicked, this, &ToolMenuWidget::signalStartSegment);
    connect(ui->clipping,&QPushButton::clicked, this, &ToolMenuWidget::signalDeleteBrushedCells);
}

ToolMenuWidget::~ToolMenuWidget()
{
    delete ui;
}

void ToolMenuWidget::setSegmentEnable(bool enable)
{
    ui->segmentBtn->setEnabled(enable);
}

void ToolMenuWidget::setTeethAlignEnable(bool enable)
{
    ui->toTeethAlignBtn->setEnabled(enable);
}

void ToolMenuWidget::slotchooseOnnxBtn()
{
    QString lastDir = mOnnxPathList.isEmpty() ? "" : QFileInfo(mOnnxPathList.last()).absolutePath();
    QString lastOpenedPath = mSettings.value("lastOpenedPath", lastDir).toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), lastOpenedPath, tr("ONNX Files (*.onnx)"));

    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        QString newLastOpenedPath = QFileInfo(fileName).absolutePath();
        mSettings.setValue("lastOpenedPath", newLastOpenedPath);
        mSettings.setValue("lastOpenedName", QFileInfo(fileName).fileName());

        updateComboBox(ui->onnxPathCombo, fileName);
    }
}

void ToolMenuWidget::slotChooseDentalBtn()
{
    QString lastDir = mDentalPathList.isEmpty() ? "" : QFileInfo(mDentalPathList.last()).absolutePath();
    QString lastOpenedPath = mSettings.value("lastOpenedDentalPath", lastDir).toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), lastOpenedPath);

    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        QString newLastOpenedPath = QFileInfo(fileName).absolutePath();
        mSettings.setValue("lastOpenedDentalPath", newLastOpenedPath);
        mSettings.setValue("lastOpenedDentalName", QFileInfo(fileName).fileName());

        updateComboBox(ui->dentalModelCombo, fileName);
    }
}

void ToolMenuWidget::slotRadioBtn(QAbstractButton *)
{
    QButtonGroup* group = qobject_cast<QButtonGroup*>(sender());
    if(group == nullptr)
        return;

    QString name = group->checkedButton()->objectName();
    qDebug() << name;

    if(name == "radioButton11")
        emit signalSetBrushColor(1);
    else if(name == "radioButton12")
        emit signalSetBrushColor(2);
    else if(name == "radioButton13")
        emit signalSetBrushColor(3);
    else if(name == "radioButton14")
        emit signalSetBrushColor(4);
    else if(name == "radioButton15")
        emit signalSetBrushColor(5);
    else if(name == "radioButton16")
        emit signalSetBrushColor(6);
    else if(name == "radioButton17")
        emit signalSetBrushColor(7);
    else if(name == "radioButton18")
        emit signalSetBrushColor(8);
    else if(name == "radioButton21")
        emit signalSetBrushColor(9);
    else if(name == "radioButton22")
        emit signalSetBrushColor(10);
    else if(name == "radioButton23")
        emit signalSetBrushColor(11);
    else if(name == "radioButton24")
        emit signalSetBrushColor(12);
    else if(name == "radioButton25")
        emit signalSetBrushColor(13);
    else if(name == "radioButton26")
        emit signalSetBrushColor(14);
    else if(name == "radioButton27")
        emit signalSetBrushColor(15);
    else if(name == "eraserBtn")
        emit signalSetBrushColor(0);
    else if(name == "radioButton28")
        emit signalSetBrushColor(16);
        ui->clipping->setEnabled(false);


}

void ToolMenuWidget::initWidget()
{
    QString lastOpenedOnnxPath = mSettings.value("lastOpenedPath").toString();
    QString lastOpenedOnnxName = mSettings.value("lastOpenedName").toString();

    if (!lastOpenedOnnxPath.isEmpty() && !lastOpenedOnnxName.isEmpty())
    {
        updateComboBox(ui->onnxPathCombo, lastOpenedOnnxPath + "/" + lastOpenedOnnxName);
    }

    QButtonGroup* radioGroup = new QButtonGroup(this);
    radioGroup->exclusive();
    radioGroup->addButton(ui->radioButton11);
    radioGroup->addButton(ui->radioButton12);
    radioGroup->addButton(ui->radioButton13);
    radioGroup->addButton(ui->radioButton14);
    radioGroup->addButton(ui->radioButton15);
    radioGroup->addButton(ui->radioButton16);
    radioGroup->addButton(ui->radioButton17);
    radioGroup->addButton(ui->radioButton18);
    radioGroup->addButton(ui->radioButton21);
    radioGroup->addButton(ui->radioButton22);
    radioGroup->addButton(ui->radioButton23);
    radioGroup->addButton(ui->radioButton24);
    radioGroup->addButton(ui->radioButton25);
    radioGroup->addButton(ui->radioButton26);
    radioGroup->addButton(ui->radioButton27);
    radioGroup->addButton(ui->radioButton28);
    radioGroup->addButton(ui->eraserBtn);


    auto c = TeethColor::COLORS2.at(1);
    ui->radioButton11->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(2);
    ui->radioButton12->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(3);
    ui->radioButton13->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(4);
    ui->radioButton14->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(5);
    ui->radioButton15->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(6);
    ui->radioButton16->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(7);
    ui->radioButton17->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(8);
    ui->radioButton18->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(9);
    ui->radioButton21->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(10);
    ui->radioButton22->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(11);
    ui->radioButton23->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(12);
    ui->radioButton24->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(13);
    ui->radioButton25->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(14);
    ui->radioButton26->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(15);
    ui->radioButton27->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));
    c = TeethColor::COLORS2.at(16);
    ui->radioButton28->setStyleSheet(QString("background-color:rgb(%1,%2,%3);border:0px;").arg(c[0]).arg(c[1]).arg(c[2]));

    connect(radioGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &ToolMenuWidget::slotRadioBtn);
}

void ToolMenuWidget::updateComboBox(QComboBox *comboBox, const QString &fileName)
{
    if(comboBox == ui->onnxPathCombo)
    {
        mOnnxPathList.append(fileName);
    }
    else if(comboBox == ui->dentalModelCombo)
    {
        mDentalPathList.append(fileName);
    }

    QString fileNameOnly = QFileInfo(fileName).fileName();
    QString parentDirName = QFileInfo(fileName).dir().dirName();
    QString displayText = parentDirName + "/" + fileNameOnly;

    comboBox->addItem(displayText);
    comboBox->setCurrentIndex(comboBox->count()-1);

    if(!mDentalPathList.isEmpty())
        qDebug() << comboBox->currentIndex() << comboBox->currentText() << mDentalPathList.at(ui->dentalModelCombo->currentIndex());
}
void ToolMenuWidget::lambdaPressed()
{
    // 获取输入的 lambda 值
    int newLambda = ui->lambda->text().toInt();
    // 发射信号
    emit signallambdaChanged(newLambda);
}

void ToolMenuWidget::roundpressed()
{
    // 获取输入的 round 值
    int newRound = ui->round->text().toInt();
    // 发射信号
    emit signalRoundChanged(newRound);
}

void ToolMenuWidget::setAdjustEnable(bool enable)
{
    ui->adjust->setEnabled(enable);
}

