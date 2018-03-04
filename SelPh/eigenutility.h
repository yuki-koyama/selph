#ifndef EIGENUTILITY_H
#define EIGENUTILITY_H

#include <vector>
#include <Eigen/Core>

namespace EigenUtility {

inline Eigen::VectorXd diag2vec(const Eigen::MatrixXd& X)
{
    Eigen::VectorXd x(X.rows());
    for (unsigned i = 0; i < X.rows(); ++ i) {
        x(i) = X(i, i);
    }
    return x;
}

inline Eigen::MatrixXd vec2diag(const Eigen::VectorXd& x)
{
    Eigen::MatrixXd X = Eigen::MatrixXd::Zero(x.rows(), x.rows());
    for (unsigned i = 0; i < x.rows(); ++ i) {
        X(i, i) = x(i);
    }
    return X;
}

inline std::vector<double> eigen2std(const Eigen::VectorXd& x)
{
    std::vector<double> y(x.rows());
    Eigen::Map<Eigen::VectorXd>(&y[0], x.rows()) = x;
    return y;
}

inline Eigen::VectorXd std2eigen(const std::vector<double>& x)
{
    return Eigen::Map<const Eigen::VectorXd>(&x[0], x.size());
}

inline Eigen::VectorXd join(const Eigen::VectorXd& x, const Eigen::VectorXd& y)
{
    Eigen::VectorXd z(x.rows() + y.rows());
    z.block(       0, 0, x.rows(), 1) = x;
    z.block(x.rows(), 0, y.rows(), 1) = y;
    return z;
}

inline Eigen::VectorXd join(const std::vector<Eigen::VectorXd>& vecs)
{
    int dim = 0;
    for (const Eigen::VectorXd& v : vecs) {
        dim += v.rows();
    }
    Eigen::VectorXd res(dim);
    int index = 0;
    for (const Eigen::VectorXd& v : vecs) {
        res.block(index, 0, v.rows(), 1) = v;
        index += v.rows();
    }
    return res;
}

}

#endif // EIGENUTILITY_H
