#include "mds.h"

#include <vector>
#include <utility>
#include <Eigen/Eigenvalues>

using namespace Eigen;

namespace MDS
{

const double epsilon = 1e-16;

// This function extract the N-largest eigen values and eigen vectors
inline void extractNLargestEigen(unsigned n, VectorXd& S, MatrixXd& V)
{
    // Note: m is the original dimension
    const unsigned m = S.rows();

    // Copy the original matrix
    const MatrixXd origV = V;

    // Sort by eigenvalue
    std::vector<std::pair<double, unsigned>> sortS(m);
    for (unsigned i = 0; i < m; ++ i) sortS[i] = std::make_pair(std::max(S(i), epsilon), i);
    std::partial_sort(sortS.begin(), sortS.begin() + n, sortS.end(), std::greater<std::pair<double, unsigned>>());

    // Resize matrices
    S.resize(n);
    V.resize(m, n);

    // Set values
    for (unsigned i = 0; i < n; ++ i)
    {
        S(i)     = sortS[i].first;
        V.col(i) = origV.col(sortS[i].second);
    }
}

// This function computes element-wise sqrt for a vector
inline VectorXd sqrt(const VectorXd& v)
{
    VectorXd result(v.rows());
    for (unsigned i = 0; i < v.rows(); ++ i) result(i) = std::sqrt(std::max(v(i), epsilon));
    return result;
}

MatrixXd computeMDS(const MatrixXd &D, unsigned dim)
{
    assert(D.rows() == D.cols());
    assert(D.rows() >= dim);
    const unsigned n = D.rows();
    const MatrixXd H = MatrixXd::Identity(n, n) - (1.0 / static_cast<double>(n)) * VectorXd::Ones(n) * VectorXd::Ones(n).transpose();
    const MatrixXd K = - 0.5 * H * D * H;
    const EigenSolver<MatrixXd> solver(K);
    VectorXd S = solver.eigenvalues().real();
    MatrixXd V = solver.eigenvectors().real();
    extractNLargestEigen(dim, S, V);
    const MatrixXd X = DiagonalMatrix<double, Dynamic>(sqrt(S)) * V.transpose();
    return X;
}

}

