
#ifndef VTKOPENGLWIDGET_H
#define VTKOPENGLWIDGET_H

#include "QVTKOpenGLNativeWidget.h"

#include "BrushCallBack.h"
#include "vtkInteractorStyleTrackballCamera.h"

#include <QLabel>
#include <QImage>

class vtkOrientationMarkerWidget;
class MouseInteractorStyle;

class VTKOpenGLWidget : public QVTKOpenGLNativeWidget
{
public:
    VTKOpenGLWidget(QWidget *parent = nullptr);

    vtkRenderer* getRenderer();

    BrushCallback* getBrush();

    MouseInteractorStyle* getInteractorStyle();

    void enableDefaltStyle(bool useDefault);
    void undoBrush();
    void redoBrush();
    void refreshRenderWindow();
private:
    void initRenderWindow();

    vtkSmartPointer<vtkRenderer> mRenderer;
    vtkSmartPointer<vtkOrientationMarkerWidget> mMarkerWidget;
    vtkSmartPointer<BrushCallback> brushCallback;
    vtkSmartPointer<MouseInteractorStyle> mInteractorStyle;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> mRenderWindow;
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> mDefaultStyle;
};

#endif // VTKOPENGLWIDGET_H
