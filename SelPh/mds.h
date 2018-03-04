#ifndef MDS_H
#define MDS_H

#include <Eigen/Core>

namespace MDS
{
// This function computes low-dimensional embedding by using multi-dimensional scaling (MDS)
// - Input:  A distance (dissimilarity) matrix and a target dimension for embedding
// - Output: A coordinate matrix whose i-th column corresponds to the embedded coordinates of the i-th entry
Eigen::MatrixXd computeMDS(const Eigen::MatrixXd& D, unsigned dim);
}

#endif // MDS_H
