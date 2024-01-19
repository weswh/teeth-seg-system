#ifndef AI3DSEGMENT_H
#define AI3DSEGMENT_H

#include "Ai3DSegment_global.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

class ModelProcess;

class AI3DSEGMENTLIB_EXPORT Ai3dSegment
{
public:
    Ai3dSegment();
    ~Ai3dSegment();

    bool loadModel(const wchar_t *wstr);

    vtkSmartPointer<vtkPolyData> segment(vtkSmartPointer<vtkPolyData> polydata);

private:
    ModelProcess* mProcess = nullptr;
};

#endif // AI3DSEGMENT_H
