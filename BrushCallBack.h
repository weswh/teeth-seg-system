#ifndef BRUSHCALLBACK_H
#define BRUSHCALLBACK_H

#include <stack>
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCellPicker.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkRendererCollection.h>
#include <vtkActorCollection.h>
#include <QDebug>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkCellData.h>
#include <vtkCellIterator.h>
#include <unordered_set>

class QLabel;

class BrushCallback : public vtkCommand
{
public:
    static BrushCallback* New()
    {
        return new BrushCallback;
    }

    void setDentalModel(vtkSmartPointer<vtkPolyData> pickSource);

    void setBrush(QLabel* brush);
    void enableBrush(bool enable);
    void setBrushColor(int colorId);
    void UndoBrush();
    void RedoBrush();
    virtual void Execute(vtkObject* caller, unsigned long eventId, void* callData);
    void DeleteBrushedCells(int colorId);
    vtkSmartPointer<vtkRenderWindowInteractor> interactor;
private:
    vtkSmartPointer<vtkPolyData> mPickSource;
    vtkSmartPointer<vtkDataArray> mColorArray;
    vtkDataArray* mColorArrayCopy;
    QLabel* mBrush;
    float mBrushRadius = 2.2;
    int mBrushColorId = -1;
    bool mIsEnableBrush = false;
    bool mLeftBtnPressed = false;
    std::stack<vtkSmartPointer<vtkDataArray>> mColorStack;
    std::stack<vtkSmartPointer<vtkUnsignedCharArray>> mRedoColorStack;
    std::unordered_set<vtkIdType> mBrushedCells;  // 新添加的成员变量
};

#endif // BRUSHCALLBACK_H
