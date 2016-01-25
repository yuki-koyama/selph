#ifndef GOODNESSFUNCTION_H
#define GOODNESSFUNCTION_H

#include <memory>
#include <vector>
#include <Eigen/Core>

class Image;

class GoodnessFunction
{
public:
    GoodnessFunction();

    Eigen::VectorXd applyGradientAscent(const Eigen::VectorXd &x, const Eigen::VectorXd &f, double scale = 1.0) const;

    double getValue(const Eigen::VectorXd &j) const;
    double getValue(const Eigen::VectorXd &x, const Eigen::VectorXd& f) const;
    Eigen::VectorXd computeGradient(const Eigen::VectorXd& j) const;
    Eigen::VectorXd computeGradient(const Eigen::VectorXd& x, const Eigen::VectorXd& f) const;
    Eigen::VectorXd getBestParameterSet(const Eigen::VectorXd& f, bool inverse = false) const;
    Eigen::VectorXd getAverageParameterSet() const;

    const double alpha;   // for interactive optimization
    const double epsilon; // for covariance matrix regularization

    // getter
    const std::vector<Eigen::VectorXd>& getParameterList() const { return pList; }
    const std::vector<Eigen::VectorXd>& getFeatureList() const { return fList; }

    // these lists store the results of the users edits
    std::vector<Eigen::VectorXd> pList;
    std::vector<Eigen::VectorXd> fList;

    void computeCovariance();
    void regularizeCovariance();
    std::vector<Eigen::MatrixXd> SList;

private:
    static Eigen::VectorXd getJointVector(const Eigen::VectorXd& x, const Eigen::VectorXd &f);
    static Eigen::VectorXd getClippedParameters(Eigen::VectorXd x);
};

#endif // GOODNESSFUNCTION_H
