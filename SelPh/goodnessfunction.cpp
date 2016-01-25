#include "goodnessfunction.h"

#include <QImage>
#include <Eigen/Dense>
#include "eigenutility.h"
#include "image.h"

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;

namespace
{
double computeGaussian(const VectorXd& x, const VectorXd& c, const MatrixXd& S_inv)
{
    const unsigned n = x.rows();
    const VectorXd r = x - c;
    const double   a = 1.0 / sqrt(pow(2.0 * M_PI, static_cast<double>(n)) * (1.0 / S_inv.determinant()));
    const double   b = exp(- 0.5 * r.transpose() * S_inv * r);
    return a * b;
}

template<typename Scalar> Scalar clamp(Scalar x, Scalar m, Scalar M) { return std::max(std::min(x, M), m); }

double computeVar(const MatrixXd &w_mean, const double &o_mean, const vector<MatrixXd> &w, const VectorXd &o, unsigned s, unsigned t, unsigned N)
{
    double t1 = 0.0;
    for (unsigned i = 0; i < N; ++ i)
    {
        t1 += (o[i] * w[i](s, t) - o_mean * w_mean(s, t)) * (o[i] * w[i](s, t) - o_mean * w_mean(s, t));
    }

    double t2 = 0.0;
    for (unsigned i = 0; i < N; ++ i)
    {
        t2 += (o[i] - o_mean) * (o[i] * w[i](s, t) - o_mean * w_mean(s, t));
    }

    double t3 = 0.0;
    for (unsigned i = 0; i < N; ++ i)
    {
        t3 += (o[i] - o_mean) * (o[i] - o_mean);
    }

    return (static_cast<double>(N) / static_cast<double>(N - 1)) * (t1 - 2.0 * w_mean(s, t) * t2 + w_mean(s, t) * w_mean(s, t) * t3);
}
}

GoodnessFunction::GoodnessFunction() :
    alpha(0.0020),
    epsilon(0.020)
{

}

// A pure implementation of the adaptive kernel density estimation algorithms described in [Talton et al. 2009] (Section 5)
void GoodnessFunction::computeCovariance()
{
    const unsigned N = getFeatureList().size();
    const unsigned n = getFeatureList()[0].rows() + getParameterList()[0].rows();

    SList.clear();
    SList.resize(N);

    // Exception
    assert (N != 0);
    if (N == 1)
    {
        SList[0] = MatrixXd::Identity(n, n);
        return;
    }

    // Prepare the data points
    vector<VectorXd> x(N);
    for (unsigned i = 0; i < N; ++ i)
    {
        x[i] = getJointVector(getParameterList()[i], getFeatureList()[i]);
    }

    // Compute the distances from the k-nearest neighborhoods
    const unsigned k = std::min(N - 1, n);
    VectorXd squaredDistance_k = VectorXd::Zero(N);
    for (unsigned i = 0; i < N; ++ i)
    {
        vector<double> d(N);
        for (unsigned j = 0; j < N; ++ j)
        {
            d[j] = (x[i] - x[j]).squaredNorm();
        }
        partial_sort(d.begin(), d.begin() + (k + 1), d.end());  // Note: d[0] is always 0.0
        squaredDistance_k(i) = d[k];
    }

    // For each Gaussian center, ...
    for (unsigned center = 0; center < N; ++ center)
    {
        // Compute weights (omega in the paper) for this data point
        VectorXd o = VectorXd::Zero(N);
        for (unsigned j = 0; j < N; ++ j)
        {
            const double   alpha = 1.0;
            const MatrixXd S_inv = (1.0 / (alpha * squaredDistance_k[center])) * MatrixXd::Identity(n, n);
            o(j) = computeGaussian(x[j], x[center], S_inv);
        }
        o /= o.sum();

        // Compute weights (w in the paper) for this data point
        vector<MatrixXd> w(N);
        for (unsigned j = 0; j < N; ++ j)
        {
            w[j] = MatrixXd::Zero(n, n);
            for (unsigned s = 0; s < n; ++ s) for (unsigned t = s; t < n; ++ t)
            {
                w[j](s, t) = (x[j].row(s) - x[center].row(s)) * (x[j].row(t) - x[center].row(t));
                w[j](t, s) = (x[j].row(s) - x[center].row(s)) * (x[j].row(t) - x[center].row(t));
            }
        }

        // Compute a covariance matrix for this data point
        SList[center] = MatrixXd::Zero(n, n);
        for (unsigned s = 0; s < n; ++ s) for (unsigned t = s; t < n; ++ t)
        {
            double elem = 0.0;
            for (unsigned j = 0; j < N; ++ j)
            {
                elem += o[j] * w[j](s, t);
            }
            SList[center](s, t) = elem;
            SList[center](t, s) = elem;
        }

        // Compute the bandwidth shrinkage algorithm
        const MatrixXd& Sigma = SList[center];

        MatrixXd Phi = MatrixXd::Zero(n, n);
        for (unsigned j = 0; j < n; ++ j) Phi(j, j) = Sigma(j, j);

        const MatrixXd& w_mean = Sigma;
        const double    o_mean = o.sum() / static_cast<double>(N);

        double t1 = 0.0;
        for (unsigned s = 0; s < n; ++ s) for (unsigned t = 0; t < n; ++ t)
        {
            if (s == t) continue;
            t1 += computeVar(w_mean, o_mean, w, o, s, t, N);
        }

        double t2 = 0.0;
        for (unsigned s = 0; s < n; ++ s) for (unsigned t = 0; t < n; ++ t)
        {
            if (s == t) continue;
            t2 += w_mean(s, t) * w_mean(s, t);
        }

        double lambda = t1 / t2;
        lambda = isnan(lambda) ? 0.0 : clamp(lambda, 0.0, 1.0);

        SList[center] = lambda * Phi + (1.0 - lambda) * Sigma;
    }
}

void GoodnessFunction::regularizeCovariance()
{
    for (MatrixXd &Sigma : SList)
    {
        const unsigned n = Sigma.rows();
        for (unsigned i = 0; i < n; ++ i)
        {
            Sigma(i, i) = max(Sigma(i, i), epsilon);
        }
    }
}

VectorXd GoodnessFunction::applyGradientAscent(const VectorXd& x, const VectorXd& f, double scale) const
{
    const VectorXd grad = computeGradient(x, f).block(0, 0, x.rows(), 1);
    const VectorXd xNew = x + scale * alpha * grad;

    return getClippedParameters(xNew);
}

double GoodnessFunction::getValue(const VectorXd& j) const
{
#if 0
    const double   sig = 0.10;
    const unsigned N   = getFeatureList().size();

    double sum = 0.0;
    for (unsigned i = 0; i < N; ++ i) {
        const VectorXd& f_i = getFeatureList()[i];
        const VectorXd& x_i = getParameterList()[i];
        const VectorXd  j_i = getJointVector(x_i, f_i);
        const double    d_i = (j - j_i).squaredNorm();
        sum += exp(- d_i / sig);
    }
    return sum / static_cast<double>(N);
#else
    const unsigned N = getFeatureList().size();
    double sum = 0.0;
    for (unsigned i = 0; i < N; ++ i) {
        const MatrixXd& S     = SList[i];
        const MatrixXd  S_inv = S.inverse();
        const VectorXd& f_i   = getFeatureList()[i];
        const VectorXd& x_i   = getParameterList()[i];
        const VectorXd  j_i   = getJointVector(x_i, f_i);
        sum += computeGaussian(j, j_i, S_inv);
    }
    return sum / static_cast<double>(N);
#endif
}

double GoodnessFunction::getValue(const VectorXd& x, const VectorXd &f) const
{
    const VectorXd j = getJointVector(x, f);
    return getValue(j);
}

VectorXd GoodnessFunction::computeGradient(const VectorXd &j) const
{
    const unsigned n = j.rows();
    const unsigned N = getFeatureList().size();
    VectorXd sum = VectorXd::Zero(n);
    for (unsigned i = 0; i < N; ++ i)
    {
        const MatrixXd& S     = SList[i];
        const MatrixXd  S_inv = S.inverse();
        const VectorXd& f_i   = getFeatureList()[i];
        const VectorXd& x_i   = getParameterList()[i];
        const VectorXd  j_i   = getJointVector(x_i, f_i);
        sum += - computeGaussian(j, j_i, S_inv) * S_inv * (j - j_i);
    }

    // NaN check
    for (unsigned i = 0; i < sum.rows(); ++ i) sum(i) = std::isnan(sum(i)) ? 0.0 : sum(i);

    return sum / static_cast<double>(N);
}

VectorXd GoodnessFunction::computeGradient(const VectorXd &x, const VectorXd &f) const
{
    const VectorXd j = getJointVector(x, f);
    return computeGradient(j);
}

VectorXd GoodnessFunction::getClippedParameters(VectorXd x)
{
    for (int i = 0; i < x.rows(); ++ i)
    {
        if (x(i) < 0.0) x(i) = 0.0;
        if (x(i) > 1.0) x(i) = 1.0;
    }
    return x;
}

VectorXd GoodnessFunction::getJointVector(const VectorXd &x, const Eigen::VectorXd& f)
{
    return EigenUtility::join(x, f);
}

VectorXd GoodnessFunction::getAverageParameterSet() const
{
    assert(!pList.empty());

    VectorXd ave = VectorXd::Zero(pList[0].rows());
    for (const VectorXd& v : pList)
    {
        ave += v;
    }
    return (1.0 / static_cast<double>(pList.size())) * ave;
}
