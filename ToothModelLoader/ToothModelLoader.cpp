#include "ToothModelLoader.h"

#include <fstream>

#include <vtkSTLReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>

#include <vtkOBJWriter.h>

#include <vtkXMLPolyDataReader.h>
#include <vtkSmartPointer.h>

#include <vtkPolyDataNormals.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include <vtkTransform.h>
#include <vtkMath.h>
#include <vtkTransformPolyDataFilter.h>

static vtkSmartPointer<vtkPolyData> readSTL(const std::string &filename)
{
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(filename.c_str());
    reader->Update();
    return reader->GetOutput();
}

static vtkSmartPointer<vtkPolyData> readVTP(const std::string &filename)
{
    vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName(filename.c_str());
    reader->Update();
    return reader->GetOutput();
}

static vtkSmartPointer<vtkPolyData> readOBJ(const std::string &filename)
{
    vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName(filename.c_str());
    reader->Update();
    return reader->GetOutput();
}

static vtkSmartPointer<vtkPolyData> readPLY(const std::string &filename)
{
    vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName(filename.c_str());
    reader->Update();
    return reader->GetOutput();
}


vtkSmartPointer<vtkPolyData> ToothModelLoader::load(const std::string &filename)
{
    // 判断文件是否存在
    std::ifstream file(filename);
    if (!file.good())
    {
        std::cerr << "File not exists: " << filename << std::endl;
        return nullptr;
    }

    // 判断文件是否为stl文件，不区分大小写
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c){ return std::tolower(c); });
    if (extension == "stl")
    {
        return readSTL(filename);
    }
    else if(extension == "ply")
    {
        return readPLY(filename);
    }
    else if(extension == "vtp")
    {
        return readVTP(filename);
    }
    else if(extension == "obj")
    {
        return readOBJ(filename);
    }
    else
    {
        std::cerr << "Unsupported file type: " << extension << std::endl;
        return nullptr;
    }
}

//vtkSmartPointer<vtkPolyData> ToothModelLoader::loadMyPly(const std::string &filename)
//{

//}

void ToothModelLoader::saveAsObj(const std::string &filePath, vtkSmartPointer<vtkPolyData> dentalModel)
{
    vtkSmartPointer<vtkOBJWriter> writer = vtkSmartPointer<vtkOBJWriter>::New();
    writer->SetFileName(filePath.c_str());
    writer->SetInputData(dentalModel);
    writer->Write();
}

vtkVector3f ToothModelLoader::computeAverageNormal(vtkSmartPointer<vtkPolyData> dentalModel)
{
    vtkSmartPointer<vtkPolyDataNormals> normalsFilter = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalsFilter->SetInputData(dentalModel);
    normalsFilter->SetComputeCellNormals(false);
    normalsFilter->SetComputePointNormals(true);
    normalsFilter->SetFeatureAngle(0.0);
    normalsFilter->Update();

    vtkPointData *pointData = normalsFilter->GetOutput()->GetPointData();
    vtkDataArray *normalsArray = pointData->GetNormals();


    vtkVector3f averageNormal;
    averageNormal.Set(0,0,0);
    auto tupleNumber = normalsArray->GetNumberOfTuples();
    double tuple[3] = {0,0,0};
    for (int i = 0; i < tupleNumber; i++) {

        normalsArray->GetTuple(i, tuple);

        float* aNormal = averageNormal.GetData();
        averageNormal.Set(aNormal[0] + tuple[0], aNormal[1] + tuple[1], aNormal[2] + tuple[2]);
    }
    float* aNormal = averageNormal.GetData();
    averageNormal.Set(aNormal[0] / tupleNumber, aNormal[1] / tupleNumber, aNormal[2] / tupleNumber);

    averageNormal = averageNormal.Normalized();

    std::cout << "avg normal: (" << averageNormal[0] << ", " << averageNormal[1] << ", " << averageNormal[2] << ")" << std::endl;

    // 返回平均法线向量
    return averageNormal;
}

void ToothModelLoader::rotateMeshToStandartAngle(vtkSmartPointer<vtkPolyData> lowerModel, vtkSmartPointer<vtkPolyData> upperModel)
{
    double massCenter[3] = {0, 0, 0};
    int lowerNumPoints = lowerModel->GetNumberOfPoints();
    for (int i = 0; i < lowerNumPoints; i++) {
        double point[3];
        lowerModel->GetPoint(i, point);
        massCenter[0] += point[0];
        massCenter[1] += point[1];
        massCenter[2] += point[2];
    }

    int upperNumPoints = upperModel->GetNumberOfPoints();
    for (int i = 0; i < upperNumPoints; i++) {
        double point[3];
        upperModel->GetPoint(i, point);
        massCenter[0] += point[0];
        massCenter[1] += point[1];
        massCenter[2] += point[2];
    }

    int totalNumPoints = lowerNumPoints + upperNumPoints;

    massCenter[0] /= totalNumPoints;
    massCenter[1] /= totalNumPoints;
    massCenter[2] /= totalNumPoints;

    /* 1. 使咬合面z轴上颌的面朝屏幕*/
    vtkVector3f uN = computeAverageNormal(upperModel);
    vtkVector3f lN = vtkVector3f(-uN.GetX(), -uN.GetY(), -uN.GetZ());

    // Compute the rotation angle and axis to rotate the upper jaw and lower jaw to the standard normal
    vtkVector3d standardUN(0, 0, 1);
    vtkVector3d standardLN(0, 0, -1);

    double lNd[3] = {lN[0], lN[1], lN[2]};

    double angleLN = vtkMath::AngleBetweenVectors(lNd, standardLN.GetData());
    vtkVector3d axisLN;
    vtkMath::Cross(lNd, standardUN.GetData(), axisLN.GetData());

    // Apply the computed rotation to the upper jaw and lower jaw
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Translate(0, 0, 0);
    transform->RotateWXYZ(angleLN * 180.0 / vtkMath::Pi(), axisLN.GetData());

    vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    filter->SetTransform(transform);
    filter->SetInputData(lowerModel);
    filter->Update();
    lowerModel->DeepCopy(filter->GetOutput());

    filter->SetInputData(upperModel);
    filter->Update();
    upperModel->DeepCopy(filter->GetOutput());


    /*  */

}
