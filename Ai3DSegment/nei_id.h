#pragma once
#define _USE_MATH_DEFINES

#include <string>
#include <vector>

#include <Eigen/Dense>
#include <Eigen/Core>

#include <unordered_map>  // 用于使用哈希表
#include <unordered_set>  // 用于使用哈希集合

#include "cgco.h"
#include <GCoptimization.h>


using namespace std;
using Eigen::MatrixXi;

#define M_PI 3.14159265358979323846

//计算每个面片的相邻面片
void compute_neighbors(const MatrixXi& cell_ids, std::vector<int>& neighbor_counts, std::vector<std::vector<int>>& neighbors) {
    int num_faces = cell_ids.rows();
    neighbor_counts.resize(num_faces, 0);
    neighbors.resize(num_faces);

    // 使用数组存储每个面的顶点
    std::vector<std::unordered_set<int>> face_vertices(num_faces);
    for (int i = 0; i < num_faces; i++) {
        for (int k = 0; k < 3; k++) {
            face_vertices[i].insert(cell_ids(i, k));
        }
    }

    // 并行计算邻居关系
#pragma omp parallel for
    for (int i = 0; i < num_faces; i++) {
        std::vector<int>& local_neighbors = neighbors[i];
        for (int j = i + 1; j < num_faces; j++) { // 优化循环顺序，只计算一半的邻居关系
            int count = 0;
            for (int k = 0; k < 3; k++) {
                int v = cell_ids(i, k);
                if (face_vertices[j].count(v) > 0)
                {
                    count++;
                }
            }
            if (count == 2) {
                local_neighbors.push_back(j);
#pragma omp atomic
                neighbor_counts[i]++; // 使用原子操作进行计数，避免锁竞争
#pragma omp atomic
                neighbor_counts[j]++; // 使用原子操作进行计数，避免锁竞争
            }
        }
    }
}

// 计算edges
void compute_edges(const std::vector<std::vector<int>>& neighbors,
    const Eigen::MatrixXd& normals,
    const Eigen::MatrixXd& barycenters,
    std::vector<std::vector<double>>& edges) {

    int num_faces = neighbors.size();
    edges.clear(); // 清空 edges 容器

    for (int i = 0; i < num_faces; i++) {
        for (int j = 0; j < neighbors[i].size(); j++) {
            int i_nei = neighbors[i][j];  // 获取相邻面片索引

            if (i < i_nei) { // 避免重复计算
                auto normal = normals.row(i);
                auto normal_nei = normals.row(i_nei);
                // 计算 cos_theta
                double cos_theta = normal.dot(normal_nei) /
                    normal.norm() /
                    normal_nei.norm();

                if (cos_theta >= 1.0) {
                    cos_theta = 0.9999;
                }

                double theta = acos(cos_theta); // 计算 theta
                double phi = (barycenters.row(i) - barycenters.row(i_nei)).norm(); // 计算 phi

                if (theta > M_PI / 2.0) {
                    // 计算 edges 中的值并添加到容器中
                    std::vector<double> edge = { static_cast<double>(i), static_cast<double>(i_nei), -log10(theta / M_PI) * phi };
                    edges.push_back(edge);
                }
                else {
                    double beta = 1 + normals.row(i).head(3).dot(normals.row(i_nei).head(3));
                    // 计算 edges 中的值并添加到容器中
                    std::vector<double> edge = { static_cast<double>(i), static_cast<double>(i_nei), -beta * log10(theta / M_PI) * phi };
                    edges.push_back(edge);
                }
            }
        }
    }
}

// 图割优化算法
Eigen::MatrixXi cut_from_graph(const Eigen::MatrixXi& edges,
    const Eigen::MatrixXi& unary_cost,
    const Eigen::MatrixXi& pairwise_cost,
    int n_iter = 5,
    const std::string& algorithm = "expansion") {
    // 检查 pairwise_cost 是否对称
    if (!pairwise_cost.transpose().isApprox(pairwise_cost)) {
        throw std::runtime_error("pairwise_cost must be symmetric.");
    }

    // 检查 unary_cost 和 pairwise_cost 的形状是否兼容
    if (unary_cost.cols() != pairwise_cost.rows()) {
        throw std::runtime_error("unary_cost and pairwise_cost have incompatible shapes.\n"
            "unary_cost must be n_vertices x n_labels, pairwise_cost must be n_labels x n_labels.");
    }

    // 检查 pairwise_cost 是否为方阵
    if (pairwise_cost.rows() != pairwise_cost.cols()) {
        throw std::runtime_error("pairwise_cost must be a square matrix.");
    }

    int n_vertices = unary_cost.rows();
    int n_labels = pairwise_cost.rows();

    GCoptimizationGeneralGraph* gc = new GCoptimizationGeneralGraph(n_vertices, n_labels);

    // 设置边缘信息
    for (int i = 0; i < edges.rows(); ++i) {
        if (edges.cols() == 3) {
            gc->setNeighbors(edges(i, 0), edges(i, 1), edges(i, 2));
        }
        else {
            gc->setNeighbors(edges(i, 0), edges(i, 1));
        }
    }

    // 设置一元势能
    Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> unary_cost_int = unary_cost.cast<int>();
    gc->setDataCost(unary_cost_int.data());


    // 设置二元势能
    Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> pairwise_cost_int = pairwise_cost.cast<int>();
    gc->setSmoothCost(pairwise_cost_int.data());

    // 根据选择的算法进行优化
    if (algorithm == "swap") {
        gc->swap(n_iter);
    }
    else if (algorithm == "expansion") {
        gc->expansion(n_iter);
    }
    else {
        throw std::runtime_error("algorithm should be either `swap` or `expansion`.");
    }

    Eigen::MatrixXi refine_labels(n_vertices, 1);

    // 获取最终的标签结果
    for (int i = 0; i < n_vertices; ++i) {
        refine_labels(i, 0) = gc->whatLabel(i);
    }

    /*delete gc;*/

    return refine_labels;
}
