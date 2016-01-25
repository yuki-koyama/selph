#ifndef IMAGEGENERATOR_H
#define IMAGEGENERATOR_H

#include <vector>
#include <memory>
#include <Eigen/Core>

class QImage;

namespace ImageModifier {

QImage modifyImage(const QImage& image, const std::vector<double>& set);

}

#endif // IMAGEGENERATOR_H
