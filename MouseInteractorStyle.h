
#ifndef MOUSEINTERACTORSTYLE_H
#define MOUSEINTERACTORSTYLE_H

#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSTLReader.h>
#include <vtkPropPicker.h>
#include <vtkProperty.h>

class VTKOpenGLWidget;

class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MouseInteractorStyle* New();
    vtkTypeMacro(MouseInteractorStyle, vtkInteractorStyleTrackballCamera);

    virtual void OnLeftButtonDown() override;
    virtual void OnRightButtonDown() override;
    virtual void OnMiddleButtonDown() override;
    virtual void OnMouseMove() override;
    virtual void OnLeftButtonUp() override;
    virtual void OnRightButtonUp() override;
    virtual void OnMiddleButtonUp() override;

private:
    MouseInteractorStyle() = default;
    ~MouseInteractorStyle() override = default;

    bool mLeftDown = false;
    bool mRightDown = false;
    bool mMiddleDown = false;
    int mLastPos[2] = {0,0};
};

#endif // MOUSEINTERACTORSTYLE_H
