#define MULTI_THREAD

#include "histogram.h"

#include <vector>
#include <chrono>
#include <iostream>
#include <cmath>
#ifdef MULTI_THREAD
#include <thread>
#endif
#include <QImage>
#include "colorutility.h"

using namespace Eigen;
using namespace std;

namespace
{
inline Vector3d qRgb2rgb(QRgb qRgb)
{
    const int r = qRed(qRgb);
    const int g = qGreen(qRgb);
    const int b = qBlue(qRgb);
    Vector3d rgb(r, g, b);
    return rgb / 255.0;
}

VectorXd computeSingleChannelHistogram(unsigned nBins, unsigned height, unsigned width, const vector<vector<double>>& pixels)
{
    VectorXd hist  = VectorXd::Zero(nBins);
    for (unsigned x = 0; x < width; ++ x) for (unsigned y = 0; y < height; ++ y)
    {
        const double color = pixels[x][y];
        for (unsigned i = 0; i < nBins; ++ i)
        {
            if (color <= (static_cast<double>(i + 1) / static_cast<double>(nBins)))
            {
                hist(i) += 1.0;
                break;
            }
        }
    }
    return hist / static_cast<double>(width * height);
}

VectorXd computeSingleChannelHistogram(unsigned nBins, unsigned height, unsigned width, unsigned channel, const vector<vector<Vector3d>>& pixels)
{
    VectorXd hist  = VectorXd::Zero(nBins);
    for (unsigned x = 0; x < width; ++ x) for (unsigned y = 0; y < height; ++ y)
    {
        const Vector3d& color = pixels[x][y];
        for (unsigned i = 0; i < nBins; ++ i)
        {
            if (color(channel) <= (static_cast<double>(i + 1) / static_cast<double>(nBins)))
            {
                hist(i) += 1.0;
                break;
            }
        }
    }
    return hist / static_cast<double>(width * height);
}
}

Histogram::Histogram(std::shared_ptr<QImage> image) : nBins(30), image(image)
{
#ifdef MULTI_THREAD
    // compute various histograms with multi-threading
    thread t1([this] () { computeRgbHistogram();       });
    thread t2([this] () { computeHslHistogram();       });
    thread t3([this] () { computeIntensityHistogram(); });
    thread t4([this] () { computeEdgeHistogram();      });
    t1.join();
    t2.join();
    t3.join();
    t4.join();
#else
    computeRgbHistogram();
    computeHslHistogram();
    computeIntensityHistogram();
    computeEdgeHistogram();
#endif
}

void Histogram::computeRgbHistogram()
{
    vector<vector<Vector3d>> pixels(image->width());
    for (int x = 0; x < image->width(); ++ x)
    {
        pixels[x].resize(image->height());
        for (int y = 0; y < image->height(); ++ y)
        {
            const Vector3d rgb = qRgb2rgb(image->pixel(x, y));
            pixels[x][y] = rgb;
        }
    }
    rgbHistogram.push_back(computeSingleChannelHistogram(nBins, image->height(), image->width(), 0, pixels));
    rgbHistogram.push_back(computeSingleChannelHistogram(nBins, image->height(), image->width(), 1, pixels));
    rgbHistogram.push_back(computeSingleChannelHistogram(nBins, image->height(), image->width(), 2, pixels));
}

void Histogram::computeHslHistogram()
{
    vector<vector<Vector3d>> pixels(image->width());
    for (int x = 0; x < image->width(); ++ x)
    {
        pixels[x].resize(image->height());
        for (int y = 0; y < image->height(); ++ y)
        {
            const Vector3d rgb = qRgb2rgb(image->pixel(x, y));
            pixels[x][y] = ColorUtility::rgb2hsl(rgb);
        }
    }
    hslHistogram.push_back(computeSingleChannelHistogram(nBins, image->height(), image->width(), 0, pixels));
    hslHistogram.push_back(computeSingleChannelHistogram(nBins, image->height(), image->width(), 1, pixels));
    hslHistogram.push_back(computeSingleChannelHistogram(nBins, image->height(), image->width(), 2, pixels));
}

void Histogram::computeIntensityHistogram()
{
    vector<vector<double>> pixels(image->width());
    for (int x = 0; x < image->width(); ++ x)
    {
        pixels[x].resize(image->height());
        for (int y = 0; y < image->height(); ++ y)
        {
            const Vector3d rgb = qRgb2rgb(image->pixel(x, y));
            pixels[x][y] = rgb.sum();
        }
    }
    intensityHistogram = computeSingleChannelHistogram(nBins, image->height(), image->width(), pixels);
}

void Histogram::computeEdgeHistogram()
{
    // Intensity
    vector<vector<double>> pixels(image->width());
    for (int x = 0; x < image->width(); ++ x)
    {
        pixels[x].resize(image->height());
        for (int y = 0; y < image->height(); ++ y)
        {
            const Vector3d rgb = qRgb2rgb(image->pixel(x, y));
            pixels[x][y] = rgb.sum();
        }
    }

    // Gradient (x)
    vector<vector<double>> gradX(image->width());
    for (int x = 0; x < image->width(); ++ x)
    {
        gradX[x].resize(image->height(), 0.0);
        for (int y = 0; y < image->height(); ++ y)
        {
            // Ignore the edges of the image
            if (x == 0 || x == image->width() - 1 || y == 0 || y == image->height()) continue;

            // Sobel filter
            double grad = - 1.0 * pixels[x - 1][y - 1] + 0.0 * pixels[x + 0][y - 1] + 1.0 * pixels[x + 1][y - 1]
                          - 2.0 * pixels[x - 1][y + 0] + 0.0 * pixels[x + 0][y + 0] + 2.0 * pixels[x + 1][y + 0]
                          - 1.0 * pixels[x - 1][y + 1] + 0.0 * pixels[x + 0][y + 1] + 1.0 * pixels[x + 1][y + 1];
            gradX[x][y] = (1.0 / 8.0) * grad;
        }
    }

    // Gradient (y)
    vector<vector<double>> gradY(image->width());
    for (int x = 0; x < image->width(); ++ x)
    {
        gradY[x].resize(image->height(), 0.0);
        for (int y = 0; y < image->height(); ++ y)
        {
            // Ignore the edges of the image
            if (x == 0 || x == image->width() - 1 || y == 0 || y == image->height()) continue;

            // Sobel filter
            double grad = + 1.0 * pixels[x - 1][y - 1] + 2.0 * pixels[x + 0][y - 1] + 1.0 * pixels[x + 1][y - 1]
                          + 0.0 * pixels[x - 1][y + 0] + 0.0 * pixels[x + 0][y + 0] + 0.0 * pixels[x + 1][y + 0]
                          - 1.0 * pixels[x - 1][y + 1] - 2.0 * pixels[x + 0][y + 1] - 1.0 * pixels[x + 1][y + 1];
            gradY[x][y] = (1.0 / 8.0) * grad;
        }
    }

    edgeHistogram.resize(2);
    edgeHistogram[0] = computeSingleChannelHistogram(nBins, image->height(), image->width(), gradX);
    edgeHistogram[1] = computeSingleChannelHistogram(nBins, image->height(), image->width(), gradY);
}

double Histogram::computeL2Distance(const VectorXd &a, const VectorXd &b)
{
    return (a - b).norm();
}

double Histogram::computeSmoothedL2Distance(const VectorXd &a, const VectorXd &b)
{
    unsigned nBins = a.rows();

    VectorXd smoothedA = a;
    VectorXd smoothedB = b;

    // laplacian smoothing
    auto smoothHistogram = [nBins] (const VectorXd& v) -> VectorXd
    {
        VectorXd next(nBins);
        next(0) = (v(0) + v(1)) / 2.0;
        for (unsigned elem = 1; elem < nBins - 1; ++ elem)
        {
            next(elem) = (v(elem - 1) + v(elem) + v(elem + 1)) / 3.0;
        }
        next(nBins - 1) = (v(nBins - 1) + v(nBins - 2)) / 2.0;
        return next;
    };

    for (unsigned i = 0; i < 5; ++ i) smoothedA = smoothHistogram(smoothedA);
    for (unsigned i = 0; i < 5; ++ i) smoothedB = smoothHistogram(smoothedB);

    return (smoothedA - smoothedB).norm();
}

double Histogram::computeSymmetricKLDivergenceDistance(const VectorXd &a, const VectorXd &b)
{
    auto computeKLDivergence = [] (const VectorXd& a, const VectorXd& b) -> double
    {
        const double epsilon = 1e-14;
        double sum = 0.0;
        for (unsigned i = 0; i < a.rows(); ++ i)
        {
            sum += a(i) * log(max(a(i) / max(b(i), epsilon), epsilon));
        }
        return sum;
    };

    return computeKLDivergence(a, b) + computeKLDivergence(b, a);
}

double Histogram::computeEntropyDistance(const VectorXd &a, const VectorXd &b)
{
    auto computeEntropy = [] (const VectorXd& v) -> double
    {
        const unsigned n = v.rows();
        const double   w = 1.0 / static_cast<double>(n);

        double sum = 0.0;
        for (unsigned i = 0; i < n; ++ i)
        {
            sum += - v(i) * log(max(v(i) / w, 1e-16));
        }
        return sum;
    };

    return std::abs(computeEntropy(a) - computeEntropy(b));
}
