#pragma once

#include<vcg/math/base.h>
#include <vcg/complex/complex.h>
#include <vcg/complex/algorithms/create/platonic.h>
#include <vcg/complex/algorithms/update/bounding.h>
#include <vcg/complex/algorithms/update/normal.h>
#include <vcg/complex/algorithms/clean.h>
#include <wrap/io_trimesh/import.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>
#include <vcg/complex/algorithms/local_optimization/tri_edge_collapse_quadric.h>
#include <vcg/container/simple_temporary_data.h>

#define MESHLAB_SCALAR double

typedef MESHLAB_SCALAR Scalarm;
typedef vcg::Point2<MESHLAB_SCALAR>     Point2m;
typedef vcg::Point3<MESHLAB_SCALAR>     Point3m;
typedef vcg::Point4<MESHLAB_SCALAR>     Point4m;
typedef vcg::Plane3<MESHLAB_SCALAR>     Plane3m;
typedef vcg::Segment2<MESHLAB_SCALAR>   Segment2m;
typedef vcg::Segment3<MESHLAB_SCALAR>   Segment3m;
typedef vcg::Box3<MESHLAB_SCALAR>       Box3m;
typedef vcg::Matrix44<MESHLAB_SCALAR>   Matrix44m;
typedef vcg::Matrix33<MESHLAB_SCALAR>   Matrix33m;
typedef vcg::Shot<MESHLAB_SCALAR>       Shotm;
typedef vcg::Similarity<MESHLAB_SCALAR> Similaritym;

using namespace std;

namespace vcg {
	namespace vertex
	{
		template <class T> class Coord3m : public Coord<vcg::Point3<Scalarm>, T> {
		public:	static void Name(std::vector<std::string>& name) { name.push_back(std::string("Coord3m")); T::Name(name); }
		};

		template <class T> class Normal3m : public Normal<vcg::Point3<Scalarm>, T> {
		public:	static void Name(std::vector<std::string>& name) { name.push_back(std::string("Normal3m")); T::Name(name); }
		};

		template <class T> class Qualitym : public Quality<Scalarm, T> {
		public: static void Name(std::vector<std::string>& name) { name.push_back(std::string("Qualitym")); T::Name(name); }
		};


		template <class T> class RadiusmOcf : public RadiusOcf<Scalarm, T> {
		public:	static void Name(std::vector<std::string>& name) { name.push_back(std::string("RadiusmOcf")); T::Name(name); }
		};

	}//end namespace vertex

	namespace face
	{
		template <class T> class Normal3m : public NormalAbs<vcg::Point3<Scalarm>, T> {
		public:  static void Name(std::vector<std::string>& name) { name.push_back(std::string("Normal3m")); T::Name(name); }
		};

		template <class T> class Qualitym : public Quality<Scalarm, T> {
		public:  static void Name(std::vector<std::string>& name) { name.push_back(std::string("QualitymOcf")); T::Name(name); }
		};


	}//end namespace face
}


class MyVertex; 
class MyEdge; 
class MyFace;

class MyUsedTypes : public vcg::UsedTypes<	vcg::Use<MyVertex>   ::AsVertexType,
    vcg::Use<MyEdge>     ::AsEdgeType,
    vcg::Use<MyFace>     ::AsFaceType> {};

class MyVertex : public vcg::Vertex<MyUsedTypes,
        vcg::vertex::Coord3m,
        vcg::vertex::Normal3m,
        vcg::vertex::Qualitym,
        vcg::vertex::BitFlags,
        vcg::vertex::Qualitym,
        vcg::vertex::VFAdj,          /*  0b */
	    vcg::vertex::Mark          /*  0b */
> {
};

class MyEdge : public vcg::Edge<MyUsedTypes, vcg::edge::EVAdj, vcg::edge::EEAdj> {};
class MyFace : public vcg::Face<MyUsedTypes,
    vcg::face::VertexRef, 
    vcg::face::BitFlags, 
    vcg::face::Normal3m,
    vcg::face::Qualitym,
    vcg::face::FFAdj, 
    vcg::face::VFAdj> {
};

class MyMesh : public vcg::tri::TriMesh<vector<MyVertex>, 
                                        vector<MyFace>> {
};




namespace vcg {
	namespace tri {
		typedef	SimpleTempData<MyMesh::VertContainer, math::Quadric<double>> QuadricTemp;

		class QHelper
		{
		public:
			QHelper() {}
			static void Init() {}
			static math::Quadric<double>& Qd(MyVertex& v) { return TD()[v]; }
			static math::Quadric<double>& Qd(MyVertex* v) { return TD()[*v]; }
			static MyVertex::ScalarType W(MyVertex* /*v*/) { return 1.0; }
			static MyVertex::ScalarType W(MyVertex& /*v*/) { return 1.0; }
			static void Merge(MyVertex& /*v_dest*/, MyVertex const& /*v_del*/) {}
			static QuadricTemp*& TDp() { static QuadricTemp* td; return td; }
			static QuadricTemp& TD() { return *TDp(); }
		};
		typedef BasicVertexPair<MyVertex> VertexPair;

		class MyTriEdgeCollapse : public vcg::tri::TriEdgeCollapseQuadric< MyMesh, VertexPair, MyTriEdgeCollapse, QHelper > {
		public:
			typedef  vcg::tri::TriEdgeCollapseQuadric< MyMesh, VertexPair, MyTriEdgeCollapse, QHelper> TECQ;
			inline MyTriEdgeCollapse(const VertexPair& p, int i, BaseParameterClass* pp) :TECQ(p, i, pp) {}
		};
	}
}

void vtk2vcg(vtkSmartPointer<vtkPolyData> vtkMesh, MyMesh& vcgMesh);
void vcg2vtk(MyMesh& vcgMesh, vtkSmartPointer<vtkPolyData> vtkMesh);

void QuadricSimplification(MyMesh & vcgMesh, int  TargetFaceNum, vcg::tri::TriEdgeCollapseQuadricParameter& pp, vcg::CallBackPos* cb);

void downSampling(MyMesh& vcgMesh);





