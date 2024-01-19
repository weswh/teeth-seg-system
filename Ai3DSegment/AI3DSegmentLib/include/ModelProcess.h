#ifndef MODELPROCESS_H
#define MODELPROCESS_H

#include "onnxruntime_cxx_api.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <Eigen/Core>

#include "Ai3DSegment_global.h"
#include "qobjectdefs.h"

using RowMajorMatrixXf = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
class AI3DSEGMENTLIB_EXPORT ModelProcess
{
public:
    ModelProcess();
    ~ModelProcess();

    static void preprocess(vtkSmartPointer<vtkPolyData> polydata,
                           Eigen::MatrixXf &normals_e,
                           RowMajorMatrixXf &input1,
                           RowMajorMatrixXf &input2,
                           RowMajorMatrixXf &input3);

    bool loadOnnxModel(const wchar_t* wstr);
    void predict(vtkSmartPointer<vtkPolyData> polydata);

    static void postprocess(vtkSmartPointer<vtkPolyData> polydata,
                            Eigen::MatrixXf& modelOutput, Eigen::MatrixXf &normals_e);

    static void mapLabel(vtkPolyData* downsampledPolyData, vtkPolyData* originalPolyData);

    static std::vector<vtkSmartPointer<vtkPolyData>> divideToothByLabel(vtkSmartPointer<vtkPolyData> dentalModel, bool deepCopy = true);
    static std::vector<vtkSmartPointer<vtkPolyData>> divideToothByColor(vtkSmartPointer<vtkPolyData> dentalModel);

public slots:
    void setLambda(int newLambda);
    void setRound(int newRound);

private:
    Eigen::MatrixXf predict(RowMajorMatrixXf &input1,
                            RowMajorMatrixXf &input2,
                            RowMajorMatrixXf &input3 );

    Ort::Env mEnv;
    Ort::SessionOptions mSessionOptions;
    Ort::Session* mSession;
    Ort::AllocatorWithDefaultOptions mAllocator;

    vtkSmartPointer<vtkPolyData> mPolydata;
    Eigen::MatrixXf mOutput;
    Eigen::MatrixXf mNormals_e;
    static  int mLambda;
    static  int mRound;
};

#endif // MODELPROCESS_H
