
#include "MouseInteractorStyle.h"

#include <QDebug>

vtkStandardNewMacro(MouseInteractorStyle);

void MouseInteractorStyle::OnLeftButtonDown()
{
    mLeftDown = true;
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

void MouseInteractorStyle::OnRightButtonDown()
{
    mRightDown = true;
    this->StartRotate();
    vtkInteractorStyleTrackballCamera::OnRightButtonDown();
}

void MouseInteractorStyle::OnMiddleButtonDown()
{
    mMiddleDown = true;
    vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();
}

void MouseInteractorStyle::OnMouseMove()
{
    if(mRightDown)
    {
        this->Rotate();
    }
    else
    {
        vtkInteractorStyleTrackballCamera::OnMouseMove();
    }
}

void MouseInteractorStyle::OnRightButtonUp()
{
    mRightDown = false;
    this->EndRotate();
    vtkInteractorStyleTrackballCamera::OnRightButtonUp();
}

void MouseInteractorStyle::OnMiddleButtonUp()
{
    mMiddleDown = false;
    vtkInteractorStyleTrackballCamera::OnMiddleButtonUp();
}

void MouseInteractorStyle::OnLeftButtonUp()
{
    mLeftDown = false;

    vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}
