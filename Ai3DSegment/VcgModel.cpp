#include "VcgModel.h"

using namespace vcg;

static bool filterCallBack(const int pos, const char* str)
{
    int static lastPos = -1;
    if (pos == lastPos) return true;
    lastPos = pos;
    std::cout << str << std::endl;
    return true;
}

void vtk2vcg(vtkSmartPointer<vtkPolyData> vtkMesh, MyMesh& vcgMesh)
{
    // 将vtkMesh转换为vcgMesh
    tri::Allocator<MyMesh>::AddVertices(vcgMesh, vtkMesh->GetNumberOfPoints());
    tri::Allocator<MyMesh>::AddFaces(vcgMesh, vtkMesh->GetNumberOfCells());
    // 1. 顶点
    for (vtkIdType i = 0; i < vtkMesh->GetNumberOfPoints(); i++)
    {
        double* p = vtkMesh->GetPoint(i);
        vcgMesh.vert[i].P() = MyMesh::CoordType(p[0], p[1], p[2]);
    }

    // 2. 面片
    MyMesh::FaceIterator fi = vcgMesh.face.begin();
    for (vtkIdType i = 0; i < vtkMesh->GetNumberOfCells(); i++)
    {
        vtkCell* cell = vtkMesh->GetCell(i);
        if (cell->GetCellType() != VTK_TRIANGLE)
        {
            std::cout << "Error: cell type is not triangle!" << std::endl;
            exit(-1);
        }
        vtkIdList* ids = cell->GetPointIds();
        for (int j = 0; j < 3; j++)
        {
            int index = ids->GetId(j);
            fi->V(j) = &vcgMesh.vert[index];
        }
        ++fi;
    }


    tri::UpdateTopology<MyMesh>::FaceFace(vcgMesh);

    // 更新VCG中的边界和法向量
    tri::UpdateBounding<MyMesh>::Box(vcgMesh);
    tri::UpdateNormal<MyMesh>::PerVertexNormalized(vcgMesh);

    // 去除重复点和非流形面
    tri::Clean<MyMesh>::RemoveDuplicateVertex(vcgMesh);
    tri::Clean<MyMesh>::RemoveNonManifoldFace(vcgMesh);
    tri::Clean<MyMesh>::RemoveNonManifoldVertex(vcgMesh);
}

void vcg2vtk(MyMesh& vcgMesh, vtkSmartPointer<vtkPolyData> vtkMesh)
{
    // 将vcgMesh转换为vtkMesh
    // 1. 顶点
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (int i = 0; i < vcgMesh.VN(); i++)
    {
        MyVertex& v = vcgMesh.vert[i];
        points->InsertNextPoint(v.P()[0], v.P()[1], v.P()[2]);
    }
    vtkMesh->SetPoints(points);

    // 2. 面片
    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    for (int i = 0; i < vcgMesh.face.size(); i++)
    {
        MyFace& f = vcgMesh.face[i];
        vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
        triangle->GetPointIds()->SetId(0, vcg::tri::Index(vcgMesh, f.V(0)));
        triangle->GetPointIds()->SetId(1, vcg::tri::Index(vcgMesh, f.V(1)));
        triangle->GetPointIds()->SetId(2, vcg::tri::Index(vcgMesh, f.V(2)));
        polys->InsertNextCell(triangle);
    }
    vtkMesh->SetPolys(polys);
}

typedef	SimpleTempData<MyMesh::VertContainer, math::Quadric<double> > QuadricTemp;
void QuadricSimplification(MyMesh &vcgMesh, int TargetFaceNum, vcg::tri::TriEdgeCollapseQuadricParameter &pp, vcg::CallBackPos *cb)
{
    tri::UpdateTopology<MyMesh>::FaceFace(vcgMesh);

    tri::UpdateTopology<MyMesh>::VertexFace(vcgMesh);

    tri::UpdateFlags<MyMesh>::FaceBorderFromVF(vcgMesh);

    math::Quadric<double> QZero;
    QZero.SetZero();
    QuadricTemp TD(vcgMesh.vert, QZero);
    tri::QHelper::TDp() = &TD;

    if (pp.PreserveBoundary)
    {
        pp.FastPreserveBoundary = true;
        pp.PreserveBoundary = false;
    }

    if (pp.NormalCheck) pp.NormalThrRad = M_PI / 4.0;

    vcg::LocalOptimization<MyMesh> DeciSession(vcgMesh, &pp);
    cb(1, "Initializing simplification");
    DeciSession.Init<tri::MyTriEdgeCollapse >();


    DeciSession.SetTargetSimplices(TargetFaceNum);
    int faceToDel = vcgMesh.fn - TargetFaceNum;
    std::cout << "vcgMesh.fn > TargetFaceNum " << (vcgMesh.fn > TargetFaceNum) << std::endl;
    while (DeciSession.DoOptimization() && vcgMesh.fn > TargetFaceNum)
    {
        cb(100 - 100 * (vcgMesh.fn - TargetFaceNum) / (faceToDel), "Simplifying...");
    };
    DeciSession.Finalize<tri::MyTriEdgeCollapse >();

    tri::Allocator<MyMesh>::CompactVertexVector(vcgMesh);
    tri::Allocator<MyMesh>::CompactFaceVector(vcgMesh);

    tri::UpdateBounding<MyMesh>::Box(vcgMesh);
    tri::UpdateNormal<MyMesh>::PerFaceNormalized(vcgMesh);
    tri::UpdateNormal<MyMesh>::PerVertexAngleWeighted(vcgMesh);
    tri::UpdateNormal<MyMesh>::NormalizePerFace(vcgMesh);
    tri::UpdateNormal<MyMesh>::PerVertexFromCurrentFaceNormal(vcgMesh);
    tri::UpdateNormal<MyMesh>::NormalizePerVertex(vcgMesh);
}

void downSampling(MyMesh &vcgMesh)
{
    int TargetFaceNum = 10000;
    tri::TriEdgeCollapseQuadricParameter pp;
    pp.QualityThr = 0.3;
    pp.PreserveBoundary = false;
    pp.NormalCheck = true;
    pp.PreserveTopology = true;
    pp.OptimalPlacement = true;
    pp.QualityQuadric = false;
    pp.QualityQuadricWeight = 0.001;

    QuadricSimplification(vcgMesh, TargetFaceNum, pp, filterCallBack);


    // 输出结果网格信息
    printf("Vertices: %d\n", vcgMesh.VN());
    printf("Faces: %d\n", vcgMesh.FN());
}
