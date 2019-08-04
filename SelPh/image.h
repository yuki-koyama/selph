#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <memory>
#include <Eigen/Core>

class QImage;
namespace imagedistance { class HistogramManager; }

class Image
{
public:
    Image(const std::string& fileName);

    std::shared_ptr<QImage> getModifiedScaledQImage  (const Eigen::VectorXd& x) const;
    std::shared_ptr<QImage> getModifiedOriginalQImage(const Eigen::VectorXd& x) const;

    // setter
    void setFeatureVector(const Eigen::VectorXd& f) { featureVector = f; }

    // getter
    const Eigen::VectorXd&                           getFeatureVector()  const { assert(featureVector.rows() != 0); return featureVector; }
    std::shared_ptr<QImage>                          getScaledQImage()   const { return scaledQImage; }
    std::shared_ptr<QImage>                          getOriginalQImage() const { return originalQImage; }
    const std::string&                               getFileName()       const { return fileName; }
    std::shared_ptr<imagedistance::HistogramManager> getHistogram()      const { return histogram; }
    double                                           getAspectRatio()    const { return aspectRatio; }
    double                                           getSize()           const { return size; }

private:

    // Additional features other than histogram-related ones
    double aspectRatio;
    double size;

    std::shared_ptr<imagedistance::HistogramManager> histogram;
    std::shared_ptr<QImage>                          scaledQImage;
    std::shared_ptr<QImage>                          originalQImage;
    std::string                                      fileName;
    Eigen::VectorXd                                  featureVector;   // transformed by MSD
};

#endif // IMAGE_H
