#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <memory>
#include <vector>
#include <Eigen/Core>

class QImage;

class Histogram
{
public:
    Histogram(std::shared_ptr<QImage> image);

    static double computeL2Distance                   (const Eigen::VectorXd& a, const Eigen::VectorXd& b);
    static double computeSmoothedL2Distance           (const Eigen::VectorXd& a, const Eigen::VectorXd& b);
    static double computeSymmetricKLDivergenceDistance(const Eigen::VectorXd& a, const Eigen::VectorXd& b);
    static double computeEntropyDistance              (const Eigen::VectorXd& a, const Eigen::VectorXd& b);

    std::vector<Eigen::VectorXd> rgbHistogram;
    std::vector<Eigen::VectorXd> hslHistogram;
    Eigen::VectorXd              intensityHistogram;
    std::vector<Eigen::VectorXd> edgeHistogram;

private:
    const unsigned                nBins;
    const std::shared_ptr<QImage> image;

    // These methods will be called only once
    void computeRgbHistogram();
    void computeHslHistogram();
    void computeIntensityHistogram();
    void computeEdgeHistogram();
};

#endif // HISTOGRAM_H
