#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "list.h"
#include "littlebutton.h"
#include "patientcase.h"
#include "qlabel.h"
#include "regist.h"
#include "signin.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

#include <vtkSmartPointer.h>
#include "Ai3DSegment.h"

class VTKOpenGLWidget;
class ToolMenuWidget;
class vtkPolyData;
class QMessageBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString onnxModelPath;
private slots:
    void slotLoadOnnx(QString fileName);
    void slotLoadDentalModel(QString fileName);
    void slotStartSegment();
    void slotSegmentFinished();
    void slotEnableBrush(bool enable);
    void slotUndoBrush();
    void slotRedoBrush();
    void slotSetColorId(int id);
    void slotExportModel();
    void slotToTeethAlignStep();
    void slotSetLambda(int newLambda);
    void slotSetRound(int newRound);
    void slotDeleteBrushedCells();

    void on_sign_clicked();

protected:
    void resizeEvent(QResizeEvent* event);

private:
    void addDentalModel(vtkSmartPointer<vtkPolyData> model);
    void addTeethModel(std::vector<vtkSmartPointer<vtkPolyData>> model);
    Ui::MainWindow* ui;

    VTKOpenGLWidget* mOpenGLWidget;
    ToolMenuWidget* mToolWidget;
    patientcase* mpatientcase;
    list* mlist;
    signin* msignin;
    regist* mregist;
    littlebutton* mlittlebutton;
    Ai3dSegment* mSegment;
    vtkSmartPointer<vtkPolyData> mCurrentDental;
    QLabel *caselabel;
    QMessageBox *mMessage;
    ModelProcess*mModelPost;
};
#endif // MAINWINDOW_H
