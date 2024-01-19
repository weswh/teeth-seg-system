#include "onnxruntime_cxx_api.h"

#include "ModelProcess.h"
#include "nei_id.h"

#include "vtkPolyDataNormals.h"
#include "vtkCenterOfMass.h"

#include "vtkCellCenters.h"
#include "vtkCellData.h"
#include "vtkThreshold.h"
#include "vtkGeometryFilter.h"
#include "vtkPointData.h"
#include "vtkCellData.h"

#include "TeethColor.h"
#include <vector>
#include <iostream>
#include "vtkOctreePointLocator.h"
#include <QDebug>

ModelProcess::ModelProcess():
    mEnv(ORT_LOGGING_LEVEL_WARNING, "AI3DSegment"),
    mSessionOptions(),
    mSession(nullptr),
    mAllocator()
{
}

ModelProcess::~ModelProcess()
{
    if(mSession)
        delete mSession;
}

void ModelProcess::preprocess(vtkSmartPointer<vtkPolyData> polydata,
                              Eigen::MatrixXf& normals_e,
                              RowMajorMatrixXf &input1,
                              RowMajorMatrixXf &input2,
                              RowMajorMatrixXf &input3)
{
    // vtk 计算法向量
    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputData(polydata);
    normals->ComputePointNormalsOff();
    normals->ComputeCellNormalsOn();
    normals->Update();


    //获取法向量数组
    auto normalsArray = normals->GetOutput()->GetCellData()->GetNormals();
    auto celln = polydata->GetNumberOfCells();
    auto points_n = polydata->GetNumberOfPoints();


    auto centerFilter = vtkSmartPointer<vtkCenterOfMass>::New();
    centerFilter->SetInputData(polydata);
    centerFilter->SetUseScalarsAsWeights(false);
    centerFilter->Update();

    double center[3];
    centerFilter->GetCenter(center);

    auto ecenter = Eigen::Vector3f((float)center[0], (float)center[1], (float)center[2]);

    // 获取cell的中心
    auto cellCenterFilter = vtkSmartPointer<vtkCellCenters>::New();
    cellCenterFilter->SetInputData(polydata);
    cellCenterFilter->Update();
    auto barycenters = cellCenterFilter->GetOutput();


    // 遍历barycenters,修改每个cell的中心
    double point1[3];
    for (int i = 0; i < celln; i++)
    {
        barycenters->GetPoint(i, point1);

        auto pp = Eigen::Vector3f((float)point1[0], (float)point1[1], (float)point1[2]) - ecenter;

        barycenters->GetPoints()->SetPoint(i, pp.x(), pp.y(), pp.z());
    }

    // 取出所有点并修改其中心，个数和cell不同，仅用于计算
    Eigen::MatrixXf epoints(points_n, 3);
    double point[3];
    for (int i = 0; i < points_n; i++)
    {
        polydata->GetPoint(i, point);

        auto p = Eigen::Vector3f((float)point[0], (float)point[1], (float)point[2]) - ecenter;
        epoints.row(i) << p.x(), p.y(), p.z();
    }

    // 遍历每个 cell 将所有点变为 n x 9 的矩阵
    Eigen::MatrixXf ecells(celln, 9);

    vtkSmartPointer<vtkCellArray> polys = polydata->GetPolys();
    for (vtkIdType i = 0; i < polys->GetNumberOfCells(); i++)
    {
        vtkSmartPointer<vtkIdList> faceIds = vtkSmartPointer<vtkIdList>::New();
        polys->GetNextCell(faceIds);

        vtkIdType ids[3] = { 0 };
        for (vtkIdType j = 0; j < faceIds->GetNumberOfIds(); j++)
        {
            ids[j] = faceIds->GetId(j);
        }
        ecells.row(i) << epoints.row(ids[0]), epoints.row(ids[1]), epoints.row(ids[2]);

    }


    // 遍历法向量
    for (int i = 0; i < normalsArray->GetNumberOfTuples(); i++)
    {
        double normal[3];
        normalsArray->GetTuple(i, normal);
        normals_e.row(i) << normal[0], normal[1], normal[2];
    }


    // 每一列最大值、最小值、平均值
    auto maxpoint = epoints.colwise().maxCoeff();
    auto minpoint = epoints.colwise().minCoeff();
    auto avgpoint = epoints.colwise().mean();

    // 每一列的标准差
    auto centered = epoints.array().rowwise() - avgpoint.array();
    auto variance = centered.array().square().colwise().mean();
    auto std_dev = variance.array().sqrt();

    // 计算法向量每个轴的均值
    auto avg_normals = normals_e.colwise().mean();

    // 计算法向量每个轴的标准差
    auto centered_normals = normals_e.array().rowwise() - avg_normals.array();
    auto variance_normals = centered_normals.array().square().colwise().mean();
    Eigen::VectorXf std_dev_normals = variance_normals.array().sqrt();


    // 对ecells中的每个点减去均值除以方差
    auto trible_avgs = Eigen::VectorXf(9);
    trible_avgs << avgpoint.x(), avgpoint.y(), avgpoint.z(),
        avgpoint.x(), avgpoint.y(), avgpoint.z(),
        avgpoint.x(), avgpoint.y(), avgpoint.z();

    auto trible_std = Eigen::VectorXf(9);
    trible_std << std_dev.x(), std_dev.y(), std_dev.z(),
        std_dev.x(), std_dev.y(), std_dev.z(),
        std_dev.x(), std_dev.y(), std_dev.z();

    ecells.rowwise() -= trible_avgs.transpose();

    for (int i = 0; i < ecells.cols(); i++)
    {
        ecells.col(i) /= trible_std(i);
    }


    // 修改barycenters中的每个点
    Eigen::MatrixXf barycenters_e(celln, 3);

    for (int i = 0; i < celln; i++)
    {
        barycenters->GetPoint(i, point);
        for (int j = 0; j < 3; j++)
        {
            point[j] = (point[j] - minpoint[j]) / (maxpoint[j] - minpoint[j]);
        }
        barycenters_e.row(i) << point[0], point[1], point[2];
    }


    normals_e.rowwise() -= avg_normals;
    for (int i = 0; i < normals_e.cols(); i++)
    {
        normals_e.col(i) /= std_dev_normals(i);
    }

    // 合并cells barycenters normals
    for (int i = 0; i < celln; i++)
    {
        input1.col(i) << ecells.row(i).transpose(), barycenters_e.row(i).transpose(), normals_e.row(i).transpose();
    }

    // 计算barycenters_e的距离矩阵
    int n = barycenters_e.rows();

#pragma omp parallel for
    for (int i = 0; i < n; ++i)
    {
        for (int j = i; j < n; ++j)
        {
            if (i == j)
            {
                input2(i, j) = 0.f;
                input3(i, j) = 0.f;
            }
            else
            {
                // 利用对称性
                float distance = (barycenters_e.row(i) - barycenters_e.row(j)).norm();

                input2(i, j) = distance < 0.1;
                input2(j, i) = distance < 0.1;
                input3(i, j) = distance < 0.2;
                input3(j, i) = distance < 0.2;

            }
        }
    }

    // 将 A_S 每一行相加
    Eigen::VectorXf A_S_sum = input2.rowwise().sum();
    Eigen::VectorXf A_L_sum = input3.rowwise().sum();

// 将A_S A_L每一列除以A_S_sum
#pragma omp parallel for
    for (int i = 0; i < input2.cols(); i++)
    {
        input2.col(i) /= A_S_sum(i);
        input3.col(i) /= A_L_sum(i);
    }
}

bool ModelProcess::loadOnnxModel(const wchar_t *wstr)
{
    try {
        mSessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    } catch (...) {
        std::cout << "SetGraphOptimizationLevel error" << std::endl;
        return false;
    }

    const auto& api = Ort::GetApi();

    OrtCUDAProviderOptions cudaOptions;
    OrtStatus* status = api.SessionOptionsAppendExecutionProvider_CUDA(static_cast<OrtSessionOptions*>(mSessionOptions),
                                                                       &cudaOptions);
    if(status) // nullptr for Status* indicates success
    {
        std::cout << "Append cuda provider failed" << std::endl;
        return false;
    }



    try
    {
        mSession = new Ort::Session(mEnv, wstr, mSessionOptions);
    }
    catch (...)
    {
        std::cout << "create Session error" << std::endl;
        return false;
    }
    return true;
}

Eigen::MatrixXf ModelProcess::predict(RowMajorMatrixXf &input1, RowMajorMatrixXf &input2, RowMajorMatrixXf &input3)
{
    auto numInputNodes = mSession->GetInputCount();
    auto numOutputNodes = mSession->GetOutputCount();

    int numOfCells = input2.rows();
    std::array<int64_t, 3> inputShape1 = { 1, 15, numOfCells };
    std::array<int64_t, 3> inputShape2 = { 1, numOfCells, numOfCells };
    std::array<int64_t, 3> inputShape3 = { 1, numOfCells, numOfCells };

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    Ort::Value inputTensor1 = Ort::Value::CreateTensor<float>(memory_info, input1.data(), input1.size(), inputShape1.data(), inputShape1.size());

    Ort::Value inputTensor2 = Ort::Value::CreateTensor<float>(memory_info, input2.data(), input2.size(), inputShape2.data(), inputShape2.size());

    Ort::Value inputTensor3 = Ort::Value::CreateTensor<float>(memory_info, input3.data(), input3.size(), inputShape3.data(), inputShape3.size());


    std::array<Ort::Value, 3> ort_inputs{ std::move(inputTensor1), std::move(inputTensor2), std::move(inputTensor3) };

    /********************************** 推理 *************************************/
    std::vector<const char*> inputNodeNames(numInputNodes);
    for (int i = 0; i < numInputNodes; i++)
    {
        Ort::AllocatedStringPtr inputName = mSession->GetInputNameAllocated(i, mAllocator);
        inputNodeNames[i] = inputName.get();
    }

    std::vector<const char*> outputNodeNames(numOutputNodes);
    for (int i = 0; i < numOutputNodes; i++)
    {
        Ort::AllocatedStringPtr outputName = mSession->GetOutputNameAllocated(i, mAllocator);
        outputNodeNames[i] = outputName.get();
    }

    std::vector<Ort::Value> outputTensors;
    try {
        outputTensors = mSession->Run(Ort::RunOptions{ nullptr }, inputNodeNames.data(), ort_inputs.data(), ort_inputs.size(), outputNodeNames.data(), 1);
    } catch (...) {
        std::cout << "onnx segment error" << std::endl;
        return Eigen::MatrixXf();
    }

    /********************************** 输出 *************************************/

    float* floatArray = outputTensors[0].GetTensorMutableData<float>();

    Eigen::Map<Eigen::MatrixXf> output(floatArray, 15, numOfCells);

    return output.transpose();
}

void ModelProcess::predict(vtkSmartPointer<vtkPolyData> polydata)
{
    auto celln = polydata->GetNumberOfCells();

    Eigen::MatrixXf normals_e(celln, 3);
    RowMajorMatrixXf input1(15, celln);
    RowMajorMatrixXf input2(celln, celln);
    RowMajorMatrixXf input3(celln, celln);

    ModelProcess::preprocess(polydata, normals_e, input1, input2, input3);

    mPolydata = polydata;
    mNormals_e = normals_e;
    Eigen::MatrixXf output = predict(input1, input2, input3);
    if(output.size() == 0)
    {
        return;
    }
    mOutput = output;
    ModelProcess::postprocess(mPolydata, mOutput, mNormals_e);
//     保存分割结果到 polydata

}

int num_classes = 15;
double eps = 1.0e-6;
// int lambda_c = 15;
int ModelProcess::mLambda = 15;
int ModelProcess::mRound = 80;

void ModelProcess::postprocess(vtkSmartPointer<vtkPolyData> polydata,
                               Eigen::MatrixXf& modelOutput,
                               Eigen::MatrixXf& normals_e)
{

    qDebug() << "postprocess called with mLambda =" << mLambda;
    int celln = polydata->GetNumberOfCells();
    // 遍历概率向量map_t，将小于eps的元素替换为eps
    for (int i = 0; i < modelOutput.size(); i++)
    {
        if (modelOutput(i) < eps)
        {
            modelOutput(i) = eps;
        }
    }

    // 一元势计算，形状为 面片数×num_classes 的矩阵
    Eigen::MatrixXi unaries = Eigen::MatrixXi::Zero(modelOutput.rows(), num_classes);
    for (int i = 0; i < modelOutput.rows(); i++)
    {
        for (int j = 0; j < num_classes; j++)
        {
            unaries(i, j) = static_cast<int>(-mRound * log10(modelOutput(i, j)));
        }
    }
    unaries.resize(modelOutput.rows(), num_classes);


    // 生成pairwise矩阵,求二元势能，形状为num_classes×num_classes
    Eigen::MatrixXi pairwise(num_classes, num_classes);
    pairwise.setOnes();  // 将所有元素赋值为 1
    pairwise = pairwise - Eigen::MatrixXi::Identity(num_classes, num_classes);

    //计算每个面片三个顶点索引，cell_ids存储每个面片的顶点索引
    vtkSmartPointer<vtkCellArray> cells = polydata->GetPolys();
    cells->InitTraversal();
    vtkIdType num_faces = cells->GetNumberOfCells();
    Eigen::Matrix<int, Eigen::Dynamic, 3> cell_ids(num_faces, 3);
    for (vtkIdType i = 0; i < num_faces; i++)
    {
        vtkSmartPointer<vtkIdList> cell = vtkSmartPointer<vtkIdList>::New();
        cells->GetNextCell(cell);
        for (int j = 0; j < 3; j++)
        {
            cell_ids(i, j) = static_cast<int>(cell->GetId(j));
        }
    }

    // 计算相邻面片数量及索引，neighbor_counts, neighbors
    std::vector<int> neighbor_counts(num_faces, 0);
    std::vector<std::vector<int>> neighbors(num_faces);

    compute_neighbors(cell_ids, neighbor_counts, neighbors);

    Eigen::MatrixXd normalsEigen = normals_e.cast<double>();

    // 重新计算barycenters_origian
    auto cellCenterFilter = vtkSmartPointer<vtkCellCenters>::New();
    cellCenterFilter->SetInputData(polydata);
    cellCenterFilter->Update();
    auto barycenters_origian = cellCenterFilter->GetOutput();

    // 将质心数据转换为Eigen::MatrixXd类型
    Eigen::MatrixXd barycentersEigen(barycenters_origian->GetNumberOfPoints(), 3);
    for (vtkIdType i = 0; i < barycenters_origian->GetNumberOfPoints(); ++i)
    {
        double* point = barycenters_origian->GetPoint(i);
        barycentersEigen.row(i) << point[0], point[1], point[2];
    }

    // 计算edges的权重代价
    std::vector<std::vector<double>> edges;
    compute_edges(neighbors, normalsEigen, barycentersEigen, edges);

    // 将计算的edges 转换为 Eigen::MatrixXd 类型
    Eigen::MatrixXd edges_matrix(edges.size(), 3);
    for (int i = 0; i < edges.size(); ++i)
    {
        edges_matrix(i, 0) = edges[i][0];
        edges_matrix(i, 1) = edges[i][1];
        edges_matrix(i, 2) = edges[i][2];
    }

    // 更新edges_matrix 最后一列数据
    edges_matrix.col(2) *= mLambda  * mRound;

    // 将 edges_matrix 中的数据转换为整型
    Eigen::MatrixXi edges_int = edges_matrix.cast<int>();


    // 调用 cut_from_graph 进行图割优化，前面的主要过程就是计算cut_from_graph图割算法的三个输入数据
    Eigen::MatrixXi result = cut_from_graph(edges_int, unaries, pairwise);


    Eigen::MatrixXi refinedLabels(celln, 1); // 定义一个 n_vertices 行 1 列的矩阵 refined_labels
    for (int i = 0; i < celln; ++i)
    {
        refinedLabels(i, 0) = result(i); // 将 result 数组中的值赋给 refined_labels 矩阵对应位置
    }

    vtkSmartPointer<vtkIntArray> labels = vtkSmartPointer<vtkIntArray>::New();
    labels->SetName("Label");
    labels->SetNumberOfComponents(1);
    labels->SetNumberOfTuples(celln);

    for (int i = 0; i < celln; ++i)
    {
        labels->SetTuple1(i, refinedLabels(i));
    }

    polydata->GetCellData()->AddArray(labels);
}

void ModelProcess::mapLabel(vtkPolyData *downsampledPolyData, vtkPolyData *originalPolyData)
{
    // 计算downsampledPolyData面片中心点
    vtkSmartPointer<vtkCellCenters> cellCenters = vtkSmartPointer<vtkCellCenters>::New();
    cellCenters->SetInputData(downsampledPolyData);
    cellCenters->Update();

    // 计算originalPolyData面片中心点
    vtkSmartPointer<vtkCellCenters> originalCellCenters = vtkSmartPointer<vtkCellCenters>::New();
    originalCellCenters->SetInputData(originalPolyData);
    originalCellCenters->Update();

    // 找到与下采样模型面片的中心点和法向量最相似的原始模型面片
    vtkSmartPointer<vtkOctreePointLocator> cellLocator = vtkSmartPointer<vtkOctreePointLocator>::New();
    cellLocator->SetDataSet(cellCenters->GetOutput());
    cellLocator->BuildLocator();

    // 遍历下采样模型面片的中心点和法向量，找到与之最相似的原始模型面片
    // 初始化
    vtkDataArray* originalLabels = originalPolyData->GetCellData()->GetArray("Label");
    if (originalLabels == nullptr)
    {
        std::cout << "none Label add label" << std::endl;
        vtkSmartPointer<vtkIntArray> labels = vtkSmartPointer<vtkIntArray>::New();
        labels->SetName("Label");
        labels->SetNumberOfComponents(1);
        labels->SetNumberOfTuples(originalPolyData->GetNumberOfCells());
        originalPolyData->GetCellData()->AddArray(labels);
        originalLabels = originalPolyData->GetCellData()->GetArray("Label");

        for (int i = 0; i < originalPolyData->GetNumberOfCells(); ++i)
        {
            originalLabels->SetTuple1(i, 0);
        }
    }

    vtkDataArray* downsampledLabels = downsampledPolyData->GetCellData()->GetArray("Label");
    vtkPoints* cellCenterPoints = originalCellCenters->GetOutput()->GetPoints();
    vtkIdType cellId;

    if(!downsampledLabels)
    {
        std::cout << "missing predict labels" << std::endl;
        return;
    }

    double cellCenterPoint[3];
    for (vtkIdType i = 0; i < originalPolyData->GetNumberOfCells(); i++)
    {
        cellCenterPoints->GetPoint(i, cellCenterPoint);
        cellId = cellLocator->FindClosestPoint(cellCenterPoint);
        originalLabels->SetTuple1(i, downsampledLabels->GetTuple1(cellId));
    }
}

std::vector<vtkSmartPointer<vtkPolyData> > ModelProcess::divideToothByLabel(vtkSmartPointer<vtkPolyData> dentalModel, bool deepCopy)
{std::cout << "divideToothByLabel function called." << std::endl;
    // 从 0 到 16 的标签值
    int minLabel = 0;
    int maxLabel = 16;

    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    if(deepCopy)
        polydata->DeepCopy(dentalModel);
    else
        polydata->ShallowCopy(dentalModel);

    polydata->GetPointData()->SetScalars(nullptr);
    polydata->GetPointData()->SetNormals(nullptr);

    // 保存每个标签对应的 vtkPolyData 对象
    std::vector<vtkSmartPointer<vtkPolyData>> segmentDatas(maxLabel - minLabel + 1);

    for (int label = minLabel; label <= maxLabel; ++label)
    {
        vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
        threshold->SetInputData(polydata);
        threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, "Label");
        threshold->SetLowerThreshold(label);
        threshold->SetUpperThreshold(label);
        threshold->Update();

        vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
        geometryFilter->SetInputConnection(threshold->GetOutputPort());

        geometryFilter->Update();

        vtkSmartPointer<vtkPolyData> segResult = geometryFilter->GetOutput();
        std::cout << "Number of cells for label " << label << ": " << segResult->GetNumberOfCells() << std::endl;

        if(segResult->GetNumberOfCells() < 1)
            continue;

        int colorsIndex = segResult->GetCellData()->HasArray("Colors");
        if(colorsIndex != -1)
        {
            segResult->GetCellData()->RemoveArray(colorsIndex);
        }

        segmentDatas[label] = segResult;
    }

    return segmentDatas;
}


std::vector<vtkSmartPointer<vtkPolyData>> ModelProcess::divideToothByColor(vtkSmartPointer<vtkPolyData> dentalModel)
{
    qDebug() << "divideToothByColor function called.";

    std::vector<vtkSmartPointer<vtkPolyData>> results;
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->DeepCopy(dentalModel);
    auto colorArray = polyData->GetCellData()->GetScalars("Colors");
    auto labelArray = polyData->GetCellData()->GetArray("Label");

    if(!colorArray)
    {
        qDebug() << "Color array is not present in the model.";
        return results;
    }

    if(!labelArray)
    {
        qDebug() << "Label array is not present in the model. Creating a new one.";
        vtkSmartPointer<vtkIntArray> labels = vtkSmartPointer<vtkIntArray>::New();
        labels->SetName("Label");
        labels->SetNumberOfComponents(1);
        labels->SetNumberOfTuples(polyData->GetNumberOfCells());
        polyData->GetCellData()->AddArray(labels);
        labelArray = polyData->GetCellData()->GetArray("Label");
    }

    auto numberOfColors = colorArray->GetNumberOfTuples();
    auto numberOfLabels = labelArray->GetNumberOfTuples();

    qDebug() << "Number of colors: " << numberOfColors;
    qDebug() << "Number of labels: " << numberOfLabels;

    if(numberOfColors != numberOfLabels)
    {
        qDebug() << "Number of colors does not equal number of labels.";
        return results;
    }

    for(vtkIdType i = 0; i < numberOfLabels; ++i)
    {
        double* color = colorArray->GetTuple3(i);
        std::vector<int> c({int(color[0]), int(color[1]), int(color[2])});
        try {
            auto label = TeethColor::reverseColorMap.at(c);
            labelArray->SetTuple1(i, label);
        } catch (const std::out_of_range& e) {
            qDebug() << "Color not found in reverseColorMap: " << c[0] << ", " << c[1] << ", " << c[2];
            continue;
        }
    }

    qDebug() << "divideToothByColor function finished.";
    return divideToothByLabel(polyData, false);
}

void ModelProcess::setLambda(int newLambda)
{
    qDebug() << "setLambda called with newLambda =" << newLambda;
    mLambda = newLambda;
    // 检查 mPolydata, mOutput, 和 mNormals_e 是否已经被初始化
//    if (mPolydata == nullptr || mOutput.size() == 0 || mNormals_e.size() == 0) {
//        // 如果没有，返回并可能输出错误信息
//        std::cout << "Error: predict must be called before setLambda." << std::endl;
//        return;
//    }
//    postprocess(mPolydata, mOutput, mNormals_e);
}

void ModelProcess::setRound(int newRound)
{
    qDebug() << "setLambda called with newLambda =" << newRound;
    mRound = newRound;
    // 检查 mPolydata, mOutput, 和 mNormals_e 是否已经被初始化
//    if (mPolydata == nullptr || mOutput.size() == 0 || mNormals_e.size() == 0) {
//        // 如果没有，返回并可能输出错误信息
//        std::cout << "Error: predict must be called before setRound." << std::endl;
//        return;
//    }

//    postprocess(mPolydata, mOutput, mNormals_e);
}

