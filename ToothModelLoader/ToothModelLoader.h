#ifndef TOOTHMODELLOADER_H
#define TOOTHMODELLOADER_H

#include "ToothModelLoader_global.h"

#include <vtkSmartPointer.h>
#include "vtkVector.h"

class vtkPolyData;

class TOOTHMODELLOADER_EXPORT ToothModelLoader
{
public:
    static ToothModelLoader& getInstance()
    {
        static ToothModelLoader instance;
        return instance;
    }

    ToothModelLoader(ToothModelLoader const&) = delete;
    void operator=(ToothModelLoader const&) = delete;

    /// STL/OBJ/PLY/XML
    vtkSmartPointer<vtkPolyData> load(const std::string& filename);
//    vtkSmartPointer<vtkPolyData> loadMyPly(const std::string& filename);
    void saveAsObj(const std::string& filePath, vtkSmartPointer<vtkPolyData> dentalModel);

    /// algorithmn
    vtkVector3f computeAverageNormal(vtkSmartPointer<vtkPolyData> dentalModel);
    void rotateMeshToStandartAngle(vtkSmartPointer<vtkPolyData> lowerModel, vtkSmartPointer<vtkPolyData> upperModel);

private:
    ToothModelLoader() {}
};

#endif // TOOTHMODELLOADER_H
