#include "BrushCallBack.h"

#include "TeethColor.h"
#include "MouseInteractorStyle.h"
#include "vtkPointData.h"

#include <Qlabel>
#include <QList>

#include <vtkThreshold.h>
#include <vtkGeometryFilter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellData.h>
#include <vtkExtractUnstructuredGrid.h>

static bool checkSize(float r)
{
    return (r > 0.1 && r < 6.5);
}

static void ComputeCellCenter(vtkCell* myCell, double c[3])
{
    double pCenter[3];
    double* weights = new double[myCell->GetNumberOfPoints()];
    int subId = myCell->GetParametricCenter(pCenter);
    myCell->EvaluateLocation(subId, pCenter, c, weights);
    delete[] weights;
}

void BrushCallback::Execute(vtkObject *caller, unsigned long eventId, void *callData)
{
    vtkInteractorStyleTrackballCamera* trackballStyle = static_cast<vtkInteractorStyleTrackballCamera*>(caller);
    auto interac = trackballStyle->GetInteractor();

    static float factor = 0.1;

    if(eventId == vtkCommand::MouseWheelForwardEvent)
    {
        auto key = interac->GetControlKey();
        if(key)
        {
            auto r = mBrushRadius + factor;
            if(checkSize(r))
                mBrushRadius = r;
        }
        else
        {
            trackballStyle->OnMouseWheelForward();
            return;
        }
    }

    if(eventId == vtkCommand::MouseWheelBackwardEvent)
    {
        auto key = interac->GetControlKey();
        if(key)
        {
            auto r = mBrushRadius - factor;
            if(checkSize(r))
                mBrushRadius = r;
        }
        else
        {
            trackballStyle->OnMouseWheelBackward();
            return;
        }
    }

    if(eventId == vtkCommand::MouseMoveEvent)
    {
        trackballStyle->OnMouseMove();
    }



    if(eventId == vtkCommand::LeftButtonReleaseEvent)
    {
        mLeftBtnPressed = false;
        // Save the final color state at the end of each brush
        auto finalColorArray = mPickSource->GetCellData()->GetScalars("Colors")->NewInstance();
        finalColorArray->DeepCopy(mPickSource->GetCellData()->GetScalars("Colors"));
        mColorStack.push(finalColorArray);

        while(!mRedoColorStack.empty())
        {
            mRedoColorStack.pop();
        }
        qDebug() << "Redo stack cleared.";

        // 添加以下代码，当松开左键时，清空被涂刷选中的单元格的集合
        mBrushedCells.clear();

        return;
    }



    if(eventId == vtkCommand::LeftButtonPressEvent)
    {
        auto currentColorArray = vtkSmartPointer<vtkUnsignedCharArray>::New();
        currentColorArray->DeepCopy(mColorArray);
        mColorStack.push(currentColorArray);

        qDebug() << "Current color state saved. Stack size: " << mColorStack.size();

        mLeftBtnPressed = true;
        // Save the initial color state at the beginning of each brush
        auto initialColorArray = mPickSource->GetCellData()->GetScalars("Colors")->NewInstance();
        initialColorArray->DeepCopy(mPickSource->GetCellData()->GetScalars("Colors"));
        mColorStack.push(initialColorArray);
    }

    if(!mIsEnableBrush || mBrushColorId < 0)
    {
        return;
    }





    MouseInteractorStyle* interactorStyle = static_cast<MouseInteractorStyle*>(caller);
    auto interactor = interactorStyle->GetInteractor();
    this->interactor = trackballStyle->GetInteractor();
    int* clickPos = interactor->GetEventPosition();

    vtkActorCollection* ac = interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActors();
    if(ac->GetNumberOfItems() == 0)
        return;


    vtkNew<vtkCellPicker> picker;
    picker->SetTolerance(0.0005);

    picker->Pick(clickPos[0], clickPos[1], 0, interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

    double* worldPosition = picker->GetPickPosition();

    auto id = picker->GetCellId();

//    if(!mPickSource)

//        return;
    if(!mPickSource) {
        qDebug() << "mPickSource is null.";
        return;
    }
    if(!mPickSource->GetCellData()) {
        qDebug() << "mPickSource->GetCellData() is null.";
        return;
    }

    if(!mPickSource->GetCellData()->GetScalars("Colors")) {
        qDebug() << "mPickSource->GetCellData()->GetScalars(\"Colors\") is null.";
        return;
    }

    if(!mColorArray) {
        qDebug() << "mColorArray is null.";
        return;
    }
    mPickSource->GetCellData()->GetScalars("Colors")->DeepCopy(mColorArray);
    QList<vtkIdType> idList;
    if (id != -1)
    {
        vtkSmartPointer<vtkCellIterator> cellIterator = mPickSource->NewCellIterator();
        vtkNew<vtkGenericCell> cell;
        double center[3];

        for(cellIterator->InitTraversal(); !cellIterator->IsDoneWithTraversal(); cellIterator->GoToNextCell())
        {
            cellIterator->GetCell(cell);

            ComputeCellCenter(cell, center);

            double distance = sqrt(pow(worldPosition[0] - center[0], 2) + pow(worldPosition[1] - center[1], 2)
                                   + pow(worldPosition[2] - center[2], 2));

            if(distance < mBrushRadius)
            {
//                idList.append(cellIterator->GetCellId());

                auto cellId = cellIterator->GetCellId();
                idList.append(cellId);

                // 添加以下代码，将被涂刷选中的单元格的ID添加到集合中
                mBrushedCells.insert(cellId);
            }
        }

        auto c = TeethColor::COLORS2.at(mBrushColorId);
        float color[3] = {(float)c[0], (float)c[1], (float)c[2]};

        if(mBrushColorId == 0) // 如果是橡皮擦
        {
            c = TeethColor::COLORS2.back();
            color[0] = (float)c[0];
            color[1] = (float)c[1];
            color[2] = (float)c[2];
        }

        for(auto cId : idList)
        {
            mPickSource->GetCellData()->GetScalars("Colors")->SetTuple(cId, color);
            if(mLeftBtnPressed){
                auto c = TeethColor::COLORS2.at(mBrushColorId);
                float color[3] = {(float)c[0], (float)c[1], (float)c[2]};
                mColorArray->SetTuple(cId, color);
            }
        }
        mPickSource->Modified();
        interactor->GetRenderWindow()->Render();

    }
}


void BrushCallback::DeleteBrushedCells(int colorId) {
    // Output initial info
    qDebug() << "==============================";
    qDebug() << "Original Data";
    qDebug() << "Points: " << mPickSource->GetPoints()->GetNumberOfPoints();
    qDebug() << "Cells: " << mPickSource->GetNumberOfCells();
    auto c = TeethColor::COLORS2.at(colorId);
    float color[3] = {(float)c[0], (float)c[1], (float)c[2]};

    qDebug() << "Number of cells in mPickSource: " << mPickSource->GetNumberOfCells();
    // 1. 获取要保留的单元格
    vtkSmartPointer<vtkPoints> pointsToKeep = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cellsToKeep = vtkSmartPointer<vtkCellArray>::New();
    std::unordered_set<vtkIdType> processedPoints;  // Track processed points

    for (vtkIdType i = 0; i < mPickSource->GetNumberOfCells(); ++i) {
        double* cellColor = mPickSource->GetCellData()->GetScalars("Colors")->GetTuple(i);

        if (!(cellColor[0] == color[0] && cellColor[1] == color[1] && cellColor[2] == color[2])) {
            cellsToKeep->InsertNextCell(mPickSource->GetCell(i));

            // Get the point IDs of the current cell
            vtkIdList* cellPointIds = mPickSource->GetCell(i)->GetPointIds();

            // Insert points of the current cell into pointsToKeep (if not processed already)
            for (vtkIdType j = 0; j < cellPointIds->GetNumberOfIds(); ++j) {
                vtkIdType pointId = cellPointIds->GetId(j);

                // Check if the point has already been processed
                if (processedPoints.find(pointId) == processedPoints.end()) {
                    double point[3];
                    mPickSource->GetPoints()->GetPoint(pointId, point);
                    pointsToKeep->InsertNextPoint(point);

                    // Mark the point as processed
                    processedPoints.insert(pointId);
                }
            }
        }
    }
    // Create a new PolyData object and set points and cells
    vtkSmartPointer<vtkPolyData> newPolyData = vtkSmartPointer<vtkPolyData>::New();
    // 清空原始 mPickSource 的数据
//    mPickSource->GetPoints()->Reset();
//    mPickSource->SetPolys(nullptr);
    // 设置点和单元格
    newPolyData->SetPoints(pointsToKeep);
    newPolyData->SetPolys(cellsToKeep);



    // 深拷贝其他数组
    newPolyData->GetPointData()->DeepCopy(mPickSource->GetPointData());
    newPolyData->GetCellData()->DeepCopy(mPickSource->GetCellData());

    qDebug() << "==============================";
    qDebug() << "New PolyData";
    qDebug() << "Points: " << newPolyData->GetPoints()->GetNumberOfPoints();
    qDebug() << "Cells: " << newPolyData->GetNumberOfCells();

    // 4. 设置新polydata
    // 4. 设置新 polydata 到 mPickSource
//    mPickSource->GetPoints()->Reset();
//    mPickSource->SetPolys(nullptr);
    mPickSource->SetPoints(pointsToKeep);
    mPickSource->SetPolys(cellsToKeep);
//    mPickSource->ShallowCopy(newPolyData);
    mPickSource = newPolyData;
//     interactor->GetRenderWindow()->Render();



    // 7. 更新 mColorArray
    mColorArray->DeepCopy(mPickSource->GetCellData()->GetScalars("Colors"));

    // 8. 清空 mColorStack
    while (!mColorStack.empty()) {
        mColorStack.pop();
    }

    // 9. 推入当前状态
    mColorStack.push(mColorArray);

    // 10. 标记 mPickSource 已修改
    mPickSource->Modified();
    setDentalModel(mPickSource);
        qDebug() << "===========sbsbsbsbs==========";
    interactor->GetRenderWindow()->Render();

}

void BrushCallback::UndoBrush()
{
    qDebug() << "UndoBrush called. Stack size: " << mColorStack.size();

    // Check if the stack is empty
    if(!mColorStack.empty())
    {

        // Save the current color state to the redo stack
        auto currentColorArray = vtkSmartPointer<vtkUnsignedCharArray>::New();
        currentColorArray->DeepCopy(mColorArray);
        mRedoColorStack.push(currentColorArray);

        qDebug() << "Current color state saved to redo stack. Redo stack size: " << mRedoColorStack.size();


        // Restore the color state to the top of the stack
        mColorArray->DeepCopy(mColorStack.top());
        mPickSource->GetCellData()->GetScalars("Colors")->DeepCopy(mColorArray);
        mPickSource->Modified();

        // Remove the top element from the stack
        mColorStack.pop();

        qDebug() << "Color state restored. New stack size: " << mColorStack.size();
    }
    else
    {
        qDebug() << "No more steps to undo";
    }
}

void BrushCallback::RedoBrush()
{
    qDebug() << "RedoBrush called. Redo stack size: " << mRedoColorStack.size();

    if(!mRedoColorStack.empty())
    {
        // Pop the top color state from the redo stack
        auto redoColorArray = mRedoColorStack.top();
        mRedoColorStack.pop();

        qDebug() << "Color state popped from redo stack. New redo stack size: " << mRedoColorStack.size();

        // Restore the color state
        mColorArray->DeepCopy(redoColorArray);
        mPickSource->GetCellData()->GetScalars("Colors")->DeepCopy(mColorArray);
        mPickSource->Modified();

        // Save the restored color state to the undo stack
        auto currentColorArray = vtkSmartPointer<vtkUnsignedCharArray>::New();
        currentColorArray->DeepCopy(mColorArray);
        mColorStack.push(currentColorArray);

        qDebug() << "Restored color state saved to undo stack. Undo stack size: " << mColorStack.size();
    }
    else
    {
        qDebug() << "No more steps to redo";
    }
}


void BrushCallback::setDentalModel(vtkSmartPointer<vtkPolyData> pickSource)
{
    mPickSource = pickSource;

    auto dataArray = mPickSource->GetCellData()->GetScalars("Colors");
    mColorArray = dataArray->NewInstance();
    mColorArray->DeepCopy(dataArray);
}

void BrushCallback::setBrush(QLabel *brush)
{
    mBrush = brush;
}

void BrushCallback::enableBrush(bool enable)
{
    mIsEnableBrush = enable;
    if(!enable)
    {
        mPickSource->GetCellData()->GetScalars("Colors")->DeepCopy(mColorArray);
        mPickSource->Modified();
    }
}

void BrushCallback::setBrushColor(int colorId)
{
    if(colorId >= TeethColor::COLORS2.size())
    {
        qDebug() << "invalid color ID" << colorId;
        return;
    }
    mBrushColorId = colorId;
}
