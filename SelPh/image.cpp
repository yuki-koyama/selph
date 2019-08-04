#include "image.h"

#include <enhancer/enhancer.hpp>
#include <imagedistance.hpp>
#include <QImage>
#include "imagemodifier.h"
#include "eigenutility.h"
#include "core.h"

using namespace std;
using namespace Eigen;

namespace
{
Core& core = Core::getInstance();
const unsigned previewHeight = 720;

void convertQImageToRgbChannels(const QImage& image,
                                Eigen::MatrixXd* r,
                                Eigen::MatrixXd* g,
                                Eigen::MatrixXd* b)
{
    const int w = image.width();
    const int h = image.height();

    r->resize(h, w);
    g->resize(h, w);
    b->resize(h, w);

    for (int x = 0; x < w; ++ x)
    {
        for (int y = 0; y < h; ++ y)
        {
            const QRgb qrgb_color = image.pixel(x, y);
            r->coeffRef(y, x) = static_cast<double>(qRed(qrgb_color)) / 255.0;
            g->coeffRef(y, x) = static_cast<double>(qGreen(qrgb_color)) / 255.0;
            b->coeffRef(y, x) = static_cast<double>(qBlue(qrgb_color)) / 255.0;
        }
    }
}
}

Image::Image(const string &fileName) : fileName(fileName)
{
    originalQImage = make_shared<QImage>(fileName.c_str());
    scaledQImage   = make_shared<QImage>(originalQImage->scaledToHeight(min<unsigned>(previewHeight, originalQImage->height()), Qt::SmoothTransformation));
    assert (!originalQImage->isNull());

    // channel extraction
    MatrixXd r, g, b;
    convertQImageToRgbChannels(*scaledQImage, &r, &g, &b);

    // feature computation
    histogram   = make_shared<imagedistance::HistogramManager>(r, g, b, enhancer::internal::rgb2hsl);
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
