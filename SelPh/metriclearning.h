#ifndef METRICLEARNING_H
#define METRICLEARNING_H

#include <Eigen/Core>
#include <vector>

namespace MetricLearning
{
Eigen::VectorXd computeMetricLearning(const std::vector<std::vector<std::vector<double>>> &D_images, const Eigen::MatrixXd& D_params, const Eigen::VectorXd& seed, unsigned nData);
}

#endif // METRICLEARNING_H
