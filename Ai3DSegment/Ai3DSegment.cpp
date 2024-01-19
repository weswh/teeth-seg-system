#include "Ai3DSegment.h"
#include "ModelProcess.h"
#include "VcgModel.h"

#include <chrono>

Ai3dSegment::Ai3dSegment()
{
}

Ai3dSegment::~Ai3dSegment()
{
    if(mProcess)
        delete mProcess;
}

bool Ai3dSegment::loadModel(const wchar_t *wstr)
{
    if(mProcess)
    {
        delete mProcess;
        mProcess = nullptr;
    }

    mProcess = new ModelProcess();

    return mProcess->loadOnnxModel(wstr);
}

vtkSmartPointer<vtkPolyData> Ai3dSegment:: segment(vtkSmartPointer<vtkPolyData> polydata)
{
//    assert(!mProcess);

    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "starting predict......";

    vtkSmartPointer<vtkPolyData> polydata2 = vtkSmartPointer<vtkPolyData>::New();
    if(polydata->GetNumberOfCells() > 10000)
    {
        std::cout << "\n cells =  " <<  polydata->GetNumberOfCells() << " start downsampling..." << std::endl;
        MyMesh vcgMesh;
        vtk2vcg(polydata, vcgMesh);
        downSampling(vcgMesh);

        vcg2vtk(vcgMesh, polydata2);

        std::cout << "done! celln = " << polydata2->GetNumberOfCells() << std::endl;
    }
    else
    {
        polydata2->DeepCopy(polydata);
    }

    mProcess->predict(polydata2);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "done! ";
    std::cout << "Time: " << duration.count() * 1e-6 << " seconds" << std::endl;

    if(polydata->GetNumberOfCells() > 10000)
    {
        ModelProcess::mapLabel(polydata2, polydata);
        std::cout << "mapped done!" << std::endl;
        return polydata;
    }

    std::cout << "without maping!" << std::endl;
    return polydata2;
}
