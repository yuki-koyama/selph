#include "image.h"

#include <QImage>
#include "imagemodifier.h"
#include "eigenutility.h"
#include "histogram.h"
#include "core.h"

using namespace std;
using namespace Eigen;

namespace
{
Core& core = Core::getInstance();
const unsigned previewHeight = 720;
}

Image::Image(const string &fileName) : fileName(fileName)
{
    originalQImage = make_shared<QImage>(fileName.c_str());
    scaledQImage   = make_shared<QImage>(originalQImage->scaledToHeight(min<unsigned>(previewHeight, originalQImage->height()), Qt::SmoothTransformation));
    assert (!originalQImage->isNull());

    // feature computation
    histogram   = make_shared<Histogram>(scaledQImage);
    aspectRatio = static_cast<double>(originalQImage->height()) / static_cast<double>(originalQImage->width());
    size        = static_cast<double>(originalQImage->height() * originalQImage->width()) / static_cast<double>(previewHeight * previewHeight);

    // temporary dummy vector
    featureVector = VectorXd::Zero(core.featureDim);
}

shared_ptr<QImage> Image::getModifiedScaledQImage(const VectorXd& x) const
{
    const QImage img = ImageModifier::modifyImage(*scaledQImage, EigenUtility::eigen2std(x));
    return make_shared<QImage>(img);
}

shared_ptr<QImage> Image::getModifiedOriginalQImage(const VectorXd& x) const
{
    const QImage img = ImageModifier::modifyImage(*originalQImage, EigenUtility::eigen2std(x));
    return make_shared<QImage>(img);
}
