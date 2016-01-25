// #define TIME

#include "metriclearning.h"

#ifdef TIME
#include <chrono>
#endif
#include <nlopt.hpp>
#include "core.h"
#include "eigenutility.h"

using namespace Eigen;
using namespace std;

namespace
{
Core& core = Core::getInstance();
}

namespace MetricLearning
{

struct Arg
{
    Arg(const Core::Distance* D_images, const MatrixXd* D_params, unsigned nData) : D_images(D_images), D_params(D_params), nData(nData) {}
    const Core::Distance* D_images;
    const MatrixXd*       D_params;
    const unsigned        nData;
};

///////////////////////////////////////////////////////////////////////////////////////////////
// \grad(C) = \sum_{i, j} \grad { \| D_images_{i, j}^T \alpha - D_params_{i, j} \|^2 }
//          = - 2 \sum_{i, j} D_images_{i, j} ( D_params_{i, j} - D_images_{i, j}^T \alpha )
///////////////////////////////////////////////////////////////////////////////////////////////
Eigen::VectorXd computeGradient(const VectorXd& alpha, const Arg* data, const double weight)
{
    VectorXd grad = VectorXd::Zero(alpha.rows());
    for (unsigned i = 0; i < data->nData; ++ i) for (unsigned j = i + 1; j < data->nData; ++ j)
    {
        const VectorXd D_ij = EigenUtility::std2eigen((*data->D_images)[i][j]);
        grad += D_ij * (weight * data->D_params->coeff(i, j) - D_ij.transpose() * alpha);
    }
    grad = - 2.0 * grad;
    return grad;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// C = \sum_{i, j} { \| D_images_{i, j}^T \alpha - D_params_{i, j} \|^2 }
///////////////////////////////////////////////////////////////////////////////////////////////
double objectiveFunction(const vector<double> &x, vector<double>& grad, void* argData)
{
    const Arg*     data   = static_cast<const Arg*>(argData);
    const VectorXd alpha  = EigenUtility::std2eigen(x);
    const double   weight = 5.0;

    // Compute the function value
    double cost = 0.0;
    for (unsigned i = 0; i < data->nData; ++ i) for (unsigned j = i + 1; j < data->nData; ++ j)
    {
        const double d = Core::computeDistance(alpha, *(data->D_images), i, j) - weight * data->D_params->coeff(i, j);
        cost += d * d;
    }

    // Compute the gradient
    grad = EigenUtility::eigen2std(computeGradient(alpha, data, weight));

    return cost;
}

VectorXd computeMetricLearning(const vector<vector<vector<double>>> &D_images, const MatrixXd &D_params, const VectorXd &seed, unsigned nData)
{
    const unsigned dim = seed.rows();

    vector<double> x = EigenUtility::eigen2std(seed);
    double value;

    const Arg argData(&D_images, &D_params, nData);

    // Compute local optimization
    nlopt::opt localOpt(nlopt::LD_LBFGS, dim);
    localOpt.set_min_objective(objectiveFunction, (void*) &argData);
    localOpt.set_lower_bounds(0.0);
    try
    {
#ifdef TIME
        const auto t1 = chrono::system_clock::now();
#endif
        localOpt.optimize(x, value);
#ifdef TIME
        const auto t2 = chrono::system_clock::now();
        cout << "Metric learning: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << " [ms]" << endl;
#endif
    }
    catch (const nlopt::roundoff_limited e)
    {
        cerr << e.what() << endl;
    }
    catch (const std::runtime_error e)
    {
        cerr << e.what() << endl;
    }

    return EigenUtility::std2eigen(x);
}
}
