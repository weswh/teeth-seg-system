#include "mainwindow.h"

#include <QResizeEvent>

#include "ui_MainWindow.h"

#include "ToolMenuWidget.h"
#include "patientcase.h"
#include "VTKOpenGLWidget.h"
#include "list.h"
#include <QDebug>

#include <QtConcurrent>
#include <QFuture>


#include <vtkOBJReader.h>
#include "vtkPointData.h"
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkProperty.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkCellData.h>
#include <vtkThreshold.h>
#include <vtkGeometryFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include "vtkUnsignedCharArray.h"

#include "TeethColor.h"
#include "ToothModelLoader.h"
#include "ModelProcess.h"

#include "QMessageBox"

using vtkData = vtkSmartPointer<vtkPolyData>;

int mapNumber(int n) {
    if(n ==0) return 0;

    if (1 <= n && n <= 7) {
        return 18 - n;
    } else if (8 <= n && n <= 16) {
        return n + 13;
    } else {
        throw std::invalid_argument("Invalid input");
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mToolWidget = new ToolMenuWidget(this);
    mpatientcase = new patientcase(this);
    msignin = new signin(this);
    mregist = new regist(this);
    mlist = new list(this);
    mlist->setVisible(false);
    mlist->move(600,100);
//    mpatientcase->setVisible(false);
    mpatientcase->lower();
    mlittlebutton = new littlebutton(this);
    mlittlebutton->move(800,20);
    mlittlebutton->setVisible(false);
    mpatientcase->move(950,150);
    msignin->move(800,200);
    mregist->move(800,200);
    ui->dockWidget->setWidget(mToolWidget);
//    ui->dockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    ui->dockWidget->setVisible(false);
    mregist->setVisible(false);
    mOpenGLWidget = new VTKOpenGLWidget(ui->centralwidget);
    mSegment = new Ai3dSegment();
    mMessage = new QMessageBox(this);
    mMessage->setWindowTitle("Information");

    connect(mToolWidget, &ToolMenuWidget::signalLoadDentalModel,this, &MainWindow::slotLoadDentalModel);

    connect(mToolWidget, &ToolMenuWidget::signalLoadOnnxModel,this, &MainWindow::slotLoadOnnx);
    connect(mToolWidget, &ToolMenuWidget::signalStartSegment,this, &MainWindow::slotStartSegment);
    connect(mToolWidget, &ToolMenuWidget::signalEnableBrush,this, &MainWindow::slotEnableBrush);
    connect(mToolWidget, &ToolMenuWidget::signalUndoBrush,this, &MainWindow::slotUndoBrush);
    connect(mToolWidget, &ToolMenuWidget::signalRedoBrush,this, &MainWindow::slotRedoBrush);

    connect(mToolWidget, &ToolMenuWidget::signalSetBrushColor,this, &MainWindow::slotSetColorId);
    connect(mToolWidget, &ToolMenuWidget::signalExport,this, &MainWindow::slotExportModel);
    connect(mToolWidget, &ToolMenuWidget::signalToTeethAlignStep,this, &MainWindow::slotToTeethAlignStep);

    connect(mToolWidget, &ToolMenuWidget::signallambdaChanged, this, &MainWindow::slotSetLambda);
    connect(mToolWidget, &ToolMenuWidget::signalRoundChanged, this, &MainWindow::slotSetRound);
    connect(mToolWidget, &ToolMenuWidget::signalDeleteBrushedCells, this, &MainWindow::slotDeleteBrushedCells);
    connect(mlittlebutton, &littlebutton::signalnewcase, this,[=](){mpatientcase->raise();mpatientcase->show();});
    connect(mlittlebutton, &littlebutton::signaltool,this,[=](){ui->dockWidget->setVisible(true);});
    connect(mlittlebutton, &littlebutton::signalcaselist, this,[=](){mlist->raise();mlist->show();});
    connect(msignin, &signin::signalsignin, this,[=](){mlittlebutton->setVisible(true);msignin->close();});
    connect(msignin, &signin::signalsignup, this,[=](){mregist->setVisible(true);msignin->close();});
    connect(mlittlebutton, &littlebutton::signalsin, this,[=](){msignin->raise();msignin->show();msignin->setVisible(true);});
    connect(mlittlebutton, &littlebutton::signalsup, this,[=](){msignin->close();mregist->show();mregist->setVisible(true);});

}

MainWindow::~MainWindow()
{
    delete ui;
    delete mSegment;
}

void MainWindow::slotLoadOnnx(QString fileName)
{

    onnxModelPath = fileName;
    if(!mSegment->loadModel(fileName.toStdWString().c_str()))
    {
        qDebug() << "load onnx failed";
        return;
    }
    mToolWidget->setSegmentEnable(true);
}

//void MainWindow::slotLoadDentalModel(QString fileName)
//{
//    mCurrentDental = ToothModelLoader::getInstance().load(fileName.toStdString());

//    addDentalModel(mCurrentDental);

//    mToolWidget->setTeethAlignEnable(false);
//}
void MainWindow::slotLoadDentalModel(QString fileName)
{
    mCurrentDental = ToothModelLoader::getInstance().load(fileName.toStdString());

    addDentalModel(mCurrentDental);

    mToolWidget->setTeethAlignEnable(false);

//    mOpenGLWidget->setVisible(true);
}

void MainWindow::slotStartSegment()
{
    this->slotLoadOnnx(onnxModelPath);
    if(mCurrentDental)
    {
        qDebug() << Q_FUNC_INFO;

        mMessage->setText("Start segment...");
        mMessage->setStandardButtons(QMessageBox::NoButton);
        mMessage->setFixedSize(200,100);
        mMessage->show();

        QFutureWatcher<vtkData> *watcher = new QFutureWatcher<vtkData>();
        QFuture<vtkData> future = QtConcurrent::run(mSegment, &Ai3dSegment::segment, mCurrentDental);
        connect(watcher, &QFutureWatcher<vtkData>::finished, this, &MainWindow::slotSegmentFinished);
        watcher->setFuture(future);
    }
}

void MainWindow::slotSegmentFinished()
{
    qDebug() << Q_FUNC_INFO;

    QFutureWatcher<vtkData> *watcher =
        static_cast<QFutureWatcher<vtkData> *>(sender());

    if (watcher)
    {
        vtkData polydata = watcher->result();
        watcher->deleteLater();
        qDebug() << "Is the same polydata? " << (polydata == mCurrentDental);
        if(polydata != mCurrentDental)
        {
            mCurrentDental->DeepCopy(polydata);
        }

        addDentalModel(mCurrentDental);

        mMessage->setStandardButtons(QMessageBox::Close);
        mMessage->setText("Segment finished...");
        mToolWidget->setTeethAlignEnable(true);
    }
}

void MainWindow::slotEnableBrush(bool enable)
{
    mOpenGLWidget->enableDefaltStyle(!enable);
    mOpenGLWidget->getBrush()->enableBrush(enable);
    qDebug() << "brush enable"<< enable;
}

void MainWindow::slotUndoBrush()
{
    mOpenGLWidget->undoBrush();
    mOpenGLWidget->refreshRenderWindow();
}

void MainWindow::slotRedoBrush()
{
    mOpenGLWidget->redoBrush();
    mOpenGLWidget->refreshRenderWindow();
}

void MainWindow::slotSetColorId(int id)
{
    mOpenGLWidget->getBrush()->setBrushColor(id);
}

void MainWindow::slotExportModel()
{
    QDir dir;
    if(!dir.exists("output"))
    {
        dir.mkdir("output");
    }

    if(mCurrentDental)
    {
        auto divdeddList = ModelProcess::divideToothByColor(mCurrentDental);
        if(divdeddList.size() == 0){
            QMessageBox::warning(this, "Export", "Export failed: there are no division");
            return;
        }

        for(int i = 0; i < divdeddList.size(); i++)
        {
            if(divdeddList.at(i))
            {
                QString fileName = "output/" + QString::number(mapNumber(i)) + ".obj";
                ToothModelLoader::getInstance().saveAsObj(fileName.toStdString(), divdeddList.at(i));
            }
        }
    }
    else
    {
        QMessageBox::warning(this, "Export", "Export failed: No model is loaded");
        return;
    }

    QMessageBox::information(this, "Export", "Export finished...");

}

//void MainWindow::slotToTeethAlignStep()
//{
//    auto devidedList = ModelProcess::divideToothByColor(mCurrentDental);
//    if(devidedList.size() == 0)
//    {
//        QMessageBox::warning(this, "Step", "There are no teeth division");
//        return;
//    }
//    addTeethModel(devidedList);
//}



void MainWindow::slotToTeethAlignStep()
{
    qDebug() << "slotToTeethAlignStep function called.";

    auto colorArray = mCurrentDental->GetCellData()->GetScalars("Colors");
    if (colorArray == nullptr) {
        QMessageBox::warning(this, "Step", "Color array is not present in the model.");
        return;
    }

    auto labelArray = mCurrentDental->GetCellData()->GetArray("Label");
    if (labelArray == nullptr) {
        QMessageBox::warning(this, "Step", "Label array is not present in the model.");
        return;
    }

    auto numberOfColors = colorArray->GetNumberOfTuples();
    auto numberOfLabels = labelArray->GetNumberOfTuples();

    qDebug() << "Number of colors: " << numberOfColors;
    qDebug() << "Number of labels: " << numberOfLabels;

    if (numberOfColors != numberOfLabels) {
        QMessageBox::warning(this, "Step", "Number of colors does not equal number of labels.");
        return;
    }

    auto devidedList = ModelProcess::divideToothByColor(mCurrentDental);

    qDebug() << "Number of items in devidedList: " << devidedList.size();

    if(devidedList.size() == 0)
    {
        QMessageBox::warning(this, "Step", "There are no teeth division");
        return;
    }

    addTeethModel(devidedList);

    qDebug() << "slotToTeethAlignStep function finished.";
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    mOpenGLWidget->resize(ui->centralwidget->size());
}

void MainWindow::addDentalModel(vtkSmartPointer<vtkPolyData> model)
{
    qDebug() << Q_FUNC_INFO;
    mOpenGLWidget->getRenderer()->RemoveAllViewProps();

    model->GetPointData()->SetScalars(nullptr);
    model->GetPointData()->SetNormals(nullptr);

    // 判断面片是否存在Label标签,若存在则遍历面片，将面片颜色设置为对应的颜色
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    if (model->GetCellData()->HasArray("Label"))
    {
        vtkSmartPointer<vtkDataArray> labels = model->GetCellData()->GetArray("Label");
        for (vtkIdType i = 0; i < labels->GetNumberOfTuples(); i++)
        {
            int label = labels->GetTuple1(i);
            auto c = TeethColor::COLORS.at(label);

            colors->InsertNextTuple3(c[0], c[1], c[2]);
        }
        model->GetCellData()->SetScalars(colors);
    }
    else
    {
        for (vtkIdType i = 0; i < model->GetNumberOfCells(); i++)
        {
            auto c = TeethColor::COLORS.at(0);

            colors->InsertNextTuple3(c[0], c[1], c[2]);
        }
        model->GetCellData()->SetScalars(colors);
    }

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(model);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->PickableOn();

    mOpenGLWidget->getRenderer()->AddActor(actor);
    mOpenGLWidget->getRenderer()->ResetCamera();
    mOpenGLWidget->getRenderer()->ResetCameraClippingRange();

    mOpenGLWidget->getBrush()->setDentalModel(model);

    mOpenGLWidget->renderWindow()->Render();

    mOpenGLWidget->move(-300,0);

    mOpenGLWidget->update();
}

void MainWindow::addTeethModel(std::vector<vtkSmartPointer<vtkPolyData> > teethList)
{
    qDebug() << Q_FUNC_INFO;
    if(teethList.size() == 0)
    {
        return;
    }

    mOpenGLWidget->getRenderer()->RemoveAllViewProps();

    for(int i = 0; i < teethList.size(); ++i)
    {
        auto model = teethList.at(i);
        if(!model || i == 0)
            continue;

        model->GetPointData()->SetScalars(nullptr);
        model->GetPointData()->SetNormals(nullptr);

        auto id = model->GetCellData()->HasArray("Colors");
        if(id != -1)
        {
            qDebug() << "remove colors";
            model->GetCellData()->RemoveArray(id);
        }

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(model);

        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);

        mOpenGLWidget->getRenderer()->AddActor(actor);
    }

    mOpenGLWidget->renderWindow()->Render();

    mOpenGLWidget->update();
}

void MainWindow::slotSetLambda(int newLambda)
{
    mModelPost->setLambda(newLambda);
    mMessage->setStandardButtons(QMessageBox::Close);
    mMessage->setText("lambda changed...");
    mToolWidget->setAdjustEnable(true);
}

void MainWindow::slotSetRound(int newRound)
{
    mModelPost->setRound(newRound);
    mMessage->setStandardButtons(QMessageBox::Close);
    mMessage->setText("round changed...");
    mToolWidget->setAdjustEnable(true);
}

void MainWindow::slotDeleteBrushedCells()
{

    int colorId = 16; // Set the colorId to 16
    qDebug() << "Entering MainWindow::slotDeleteBrushedCells.";
    qDebug() << "About to call DeleteBrushedCells with colorId: " << colorId;
    mOpenGLWidget->getBrush()->DeleteBrushedCells(colorId);
    qDebug() << "Returned from DeleteBrushedCells.";
//    mOpenGLWidget->renderWindow()->Render();
//    mOpenGLWidget->update();
    qDebug() << "flashed";
}

void MainWindow::on_sign_clicked()
{
    msignin->setVisible(true);
}

