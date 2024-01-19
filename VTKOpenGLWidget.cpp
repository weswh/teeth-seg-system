
#include "VTKOpenGLWidget.h"

#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkProperty.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkCellData.h>
#include <vtkThreshold.h>
#include <vtkGeometryFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkAxesActor.h>
#include <vtkLine.h>
#include <vtkLineSource.h>

#include <vtkArrowSource.h>
#include <vtkTransformPolyDataFilter.h>

#include <vtkCaptionActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextActor.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRegularPolygonSource.h>

#include <QPainter>

#include "MouseInteractorStyle.h"
#include <QCoreApplication>


VTKOpenGLWidget::VTKOpenGLWidget(QWidget *parent):
    QVTKOpenGLNativeWidget(parent)
{
    initRenderWindow();
}

vtkRenderer *VTKOpenGLWidget::getRenderer()
{
    return mRenderer.Get();
}

BrushCallback *VTKOpenGLWidget::getBrush()
{
    return brushCallback.Get();
}

void VTKOpenGLWidget::undoBrush()
{
    brushCallback->UndoBrush();
}

void VTKOpenGLWidget::redoBrush()
{
    brushCallback->RedoBrush();
}


void VTKOpenGLWidget::refreshRenderWindow()
{
      mRenderWindow->Render();
}

void VTKOpenGLWidget::enableDefaltStyle(bool useDefault)
{
    if(useDefault)
    {
        mRenderWindow->GetInteractor()->SetInteractorStyle(mDefaultStyle);
    }
    else
    {
        mRenderWindow->GetInteractor()->SetInteractorStyle(mInteractorStyle);
    }
}

void VTKOpenGLWidget::initRenderWindow()
{
    vtkNew<vtkNamedColors> colors;
    colors->SetColor("myColor", 245/255.0, 255/255.0, 250/255.0);
    mRenderer = vtkSmartPointer<vtkRenderer>::New();

    //    mRenderer->SetBackground(colors->GetColor3d("white").GetData());
    mRenderer->SetBackground(colors->GetColor3d("myColor").GetData());

    mRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    mRenderWindow->SetAlphaBitPlanes(1);
    mRenderWindow->AddRenderer(mRenderer);


    this->setRenderWindow(mRenderWindow);

    vtkSmartPointer<vtkAxesActor> axesActor = vtkSmartPointer<vtkAxesActor>::New();


    mDefaultStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();

    mInteractorStyle = vtkSmartPointer<MouseInteractorStyle>::New();
    mRenderWindow->GetInteractor()->SetInteractorStyle(mDefaultStyle);

    // 设置渲染窗口坐下脚坐标轴显示
    mMarkerWidget =  vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    mMarkerWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
    mMarkerWidget->SetOrientationMarker(axesActor);
    mMarkerWidget->SetInteractor(mRenderWindow->GetInteractor());
    mMarkerWidget->SetViewport(0.0, 0.0, 0.2, 0.2);
    mMarkerWidget->SetEnabled(1);


    brushCallback = vtkSmartPointer<BrushCallback>::New();
    mInteractorStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, brushCallback);
    mInteractorStyle->AddObserver(vtkCommand::LeftButtonPressEvent, brushCallback);
    mInteractorStyle->AddObserver(vtkCommand::MouseMoveEvent, brushCallback);
    mInteractorStyle->AddObserver(vtkCommand::MouseWheelBackwardEvent, brushCallback);
    mInteractorStyle->AddObserver(vtkCommand::MouseWheelForwardEvent, brushCallback);

    this->update();
}


