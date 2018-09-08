#include "imagemodifier.h"

#include <cmath>
#include <cassert>
#include <QImage>
#include <enhancer.hpp>
#include <parallel-util.hpp>

using std::vector;
using std::max;
using std::min;
using std::thread;
using Eigen::Vector3d;

namespace ImageModifier
{
    inline Vector3d qRgb2rgb(const QRgb& qRgb)
    {
        const int r = qRed(qRgb);
        const int g = qGreen(qRgb);
        const int b = qBlue(qRgb);
        Vector3d rgb(r, g, b);
        return rgb / 255.0;
    }
    
    QImage modifyImage(const QImage& image, const std::vector<double>& set)
    {
        assert (set.size() == 3 || set.size() == 6);
        
        std::vector<double> raw_parameters = set;
        raw_parameters.resize(6, 0.5);
        const Eigen::VectorXd parameters = Eigen::Map<const Eigen::VectorXd>(&raw_parameters[0], 6);
        
        const int w = image.rect().width();
        const int h = image.rect().height();
        
        QImage newImg = QImage(w, h, QImage::Format_RGB32);
        
        auto changePixelColor = [&](const int x, const int y)
        {
            const QRgb            original_rgb = image.pixel(x, y);
            const Eigen::Vector3d input_rgb    = qRgb2rgb(original_rgb);
            const Eigen::Vector3d output_rgb   = enhancer::enhance(input_rgb, parameters);
            const QRgb            new_rgb = qRgb(static_cast<int>(output_rgb(0) * 255.0),
                                                 static_cast<int>(output_rgb(1) * 255.0),
                                                 static_cast<int>(output_rgb(2) * 255.0));
            newImg.setPixel(x, y, new_rgb);
        };
        
        parallelutil::parallel_for_2d(w, h, changePixelColor);
        
        return newImg;
    }
}
