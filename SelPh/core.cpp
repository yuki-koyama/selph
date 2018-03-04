// #define TIME

#include "core.h"

#include <cstdlib>
#include <ctime>
#include <Eigen/SVD>
#include "mainwindow.h"
#include "image.h"
#include "previewwidget.h"
#include "eigenutility.h"
#include "mds.h"
#include "histogram.h"
#include "metriclearning.h"

using std::vector;
using std::pair;
using std::string;
using std::shared_ptr;
using Eigen::VectorXd;
using Eigen::MatrixXd;

Core::Core() :
    featureDim(5),
    nReferencePhotos(10),
    currentIndex(0),
    isBaselineMode(false),
    uiSize(0),
    useVisualization(true),
    useOptimization(false),
    useInitialOptimization(false),
    useSortingPhotos(false),
    nIterations(1),
    gradationResolution(40)
{
    srand(time(NULL));
}

void Core::setReferencePhotos()
{
    vector<shared_ptr<QImage>> referenceImages;

    // If this is the baseline mode, show reference images in the original order
    if (isBaselineMode)
    {
        for (int i = enhancedImages.size() - 1; i >= 0; -- i) referenceImages.push_back(enhancedImages[i]);
        mainWindow->setReferencePhotos(referenceImages);
        return;
    }

    // If this is the standard mode, sort the images by using the distances
    vector<pair<double, unsigned>> distances(currentIndex);
    const unsigned n = std::min<unsigned>(nReferencePhotos, enhancedImages.size());
    for (unsigned i = 0; i < currentIndex; ++ i)
    {
        const double d = (images[i]->getFeatureVector() - images[currentIndex]->getFeatureVector()).squaredNorm();
        distances[i] = pair<double, unsigned>(d, i);
    }
    partial_sort(distances.begin(), distances.begin() + n, distances.end());
    for (unsigned i = 0; i < n; ++ i)
    {
        referenceImages.push_back(enhancedImages[distances[i].second]);
    }
    mainWindow->setReferencePhotos(referenceImages);
}

namespace
{

inline vector<double> computeDistanceBetweenImages(const shared_ptr<Image> a, const shared_ptr<Image> b)
{
    vector<double> d;

    // Histogram-based distances
    for (unsigned k = 0; k < 3; ++ k)
    {
        d.push_back(Histogram::computeL2Distance(a->getHistogram()->rgbHistogram[k], b->getHistogram()->rgbHistogram[k]));
        d.push_back(Histogram::computeSmoothedL2Distance(a->getHistogram()->rgbHistogram[k], b->getHistogram()->rgbHistogram[k]));
        d.push_back(Histogram::computeSymmetricKLDivergenceDistance(a->getHistogram()->rgbHistogram[k], b->getHistogram()->rgbHistogram[k]));
        d.push_back(Histogram::computeEntropyDistance(a->getHistogram()->rgbHistogram[k], b->getHistogram()->rgbHistogram[k]));
    }
    for (unsigned k = 0; k < 3; ++ k)
    {
        d.push_back(Histogram::computeL2Distance(a->getHistogram()->hslHistogram[k], b->getHistogram()->hslHistogram[k]));
        d.push_back(Histogram::computeSmoothedL2Distance(a->getHistogram()->hslHistogram[k], b->getHistogram()->hslHistogram[k]));
        d.push_back(Histogram::computeSymmetricKLDivergenceDistance(a->getHistogram()->hslHistogram[k], b->getHistogram()->hslHistogram[k]));
        d.push_back(Histogram::computeEntropyDistance(a->getHistogram()->hslHistogram[k], b->getHistogram()->hslHistogram[k]));
    }
    {
        d.push_back(Histogram::computeL2Distance(a->getHistogram()->intensityHistogram, b->getHistogram()->intensityHistogram));
        d.push_back(Histogram::computeSmoothedL2Distance(a->getHistogram()->intensityHistogram, b->getHistogram()->intensityHistogram));
        d.push_back(Histogram::computeSymmetricKLDivergenceDistance(a->getHistogram()->intensityHistogram, b->getHistogram()->intensityHistogram));
        d.push_back(Histogram::computeEntropyDistance(a->getHistogram()->intensityHistogram, b->getHistogram()->intensityHistogram));
    }
    for (unsigned k = 0; k < 2; ++ k)
    {
        d.push_back(Histogram::computeL2Distance(a->getHistogram()->edgeHistogram[k], b->getHistogram()->edgeHistogram[k]));
        d.push_back(Histogram::computeSmoothedL2Distance(a->getHistogram()->edgeHistogram[k], b->getHistogram()->edgeHistogram[k]));
        d.push_back(Histogram::computeSymmetricKLDivergenceDistance(a->getHistogram()->edgeHistogram[k], b->getHistogram()->edgeHistogram[k]));
        d.push_back(Histogram::computeEntropyDistance(a->getHistogram()->edgeHistogram[k], b->getHistogram()->edgeHistogram[k]));
    }

    // Other distances
    d.push_back(std::abs(a->getAspectRatio() - b->getAspectRatio()));
    d.push_back(std::abs(a->getSize() - b->getSize()));

    return d;
}

inline Core::Distance computeImageDistances(const vector<shared_ptr<Image>>& images)
{
    const unsigned n = images.size();
    Core::Distance D_images(n);
    for (unsigned i = 0; i < n; ++ i)
    {
        D_images[i].resize(n);
        for (unsigned j = 0; j < n; ++ j)
        {
            D_images[i][j] = computeDistanceBetweenImages(images[i], images[j]);
        }
    }
    return D_images;
}

inline MatrixXd computeParameterDistances(const vector<VectorXd>& params)
{
    const unsigned n = params.size();
    MatrixXd D(n, n);
    for (unsigned i = 0; i < n; ++ i) for (unsigned j = i; j < n; ++ j)
    {
        const double d = (params[i] - params[j]).norm();
        D(i, j) = d;
        D(j, i) = d;
    }
    return D;
}

MatrixXd correctRotation(const MatrixXd& X, const vector<shared_ptr<Image>>& images)
{
    // Find the optimal rotation
    MatrixXd A = MatrixXd::Zero(X.rows(), X.rows());
    for (unsigned i = 0; i < X.cols() - 1; ++ i)
    {
        VectorXd p = images[i]->getFeatureVector();
        VectorXd q = X.col(i);
        A += p * q.transpose();
    }
    Eigen::JacobiSVD<MatrixXd> svd(A, Eigen::ComputeFullU);
    MatrixXd R = svd.matrixU();
    return R * X;
}

}

void Core::computeMDS()
{
    // set the distance matrix
    unsigned n = currentIndex + 1;
    D.resize(n, n);
    for (unsigned i = 0; i < n; ++ i)
    {
        for (unsigned j = i; j < n; ++ j)
        {
            const double d  = computeDistance(alpha, distance, i, j);
            const double d2 = d * d;

            // Note: D needs to have the squared norms for MDS
            D(i, j) = d2;
            D(j, i) = d2;
        }
    }

    // Compute the MDS algorithm
#ifdef TIME
    const auto t1 = system_clock::now();
#endif
    const MatrixXd X = MDS::computeMDS(D, std::min<unsigned>(featureDim, D.rows()));
#ifdef TIME
    const auto t2 = system_clock::now();
    std::cout << "Metric MDS: " << duration_cast<milliseconds>(t2 - t1).count() << " [ms]" << std::endl;
#endif

    // Correct the rotation (optional)
    const MatrixXd Y = (featureDim == 2 && n > 2) ? correctRotation(X, images) : X;

    // set the results to the images
    for (unsigned i = 0; i < n; ++ i)
    {
        images[i]->setFeatureVector(Y.col(i));
    }
}

void Core::computeMetricLearning()
{
    // If this is the first time to compute, initialize the alpha
    if (alpha.rows() != 38)
    {
        alpha = VectorXd::Zero(38);
    }

    // Prepare distances
    const MatrixXd D_params = computeParameterDistances(goodnessFunction.pList); // TODO: This can be incrementally updated

    // Compute the metric learning
    alpha = MetricLearning::computeMetricLearning(distance, D_params, alpha, goodnessFunction.pList.size());
}

void Core::initialize(const string& dirPath)
{
    // Prepare working directories
    generateDirectories();

    // Import photos
    importPhotos(dirPath);

    // Compute (unlearned) distances between photos
    distance = computeImageDistances(images);
    exportRawDistance();

    // Set the first photo to the UI
    previewWidget->setCurrentImage(*images[currentIndex]->getScaledQImage());

    // Prepare data for study (for the first time only)
    studyData.resize(images.size());

    // Start study
    const VectorXd x = EigenUtility::std2eigen(parameters);
    getCurrentStudyData().confidence         = confidence;
    getCurrentStudyData().autoParameter      = x;
    getCurrentStudyData().naiveAutoParameter = x;
    getCurrentStudyData().index              = currentIndex;
    getCurrentStudyData().targetImage        = images[currentIndex];
    getCurrentStudyData().parameterSequence.push_back(x);
    time_point = std::chrono::system_clock::now();
}

VectorXd Core::getCurrentFeatureVector() const
{
    return images[currentIndex]->getFeatureVector();
}

double Core::computeConfidence(unsigned index) const
{
    const VectorXd&         feature      = images[index]->getFeatureVector();
    const vector<VectorXd>& prevFeatures = goodnessFunction.getFeatureList();

    if (prevFeatures.size() < 2)
    {
        return 0.0;
    }

    // compute distances
    vector<double> distances;
    for (const VectorXd& f : prevFeatures)
    {
        distances.push_back((feature - f).norm());
    }

    // sort them
    partial_sort(distances.begin(), distances.begin() + 1, distances.end());

    // compute confidence
    return pow(1.0 / (1.0 + distances[0]), 3.0);
}

bool Core::goNext()
{
    const VectorXd x = EigenUtility::std2eigen(parameters);
    const shared_ptr<Image> currentImage = images[currentIndex];

    // for study
    getCurrentStudyData().elapsedTime    = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_point).count();
    getCurrentStudyData().userParameter  = x;
    getCurrentStudyData().visualized     = useVisualization;
    getCurrentStudyData().optimized      = useOptimization;
    getCurrentStudyData().print();
    getCurrentStudyData().exportModifiedScaledPhoto(workingDirectoryPath);

    // Finish the task
    if (currentIndex + 1 == images.size())
    {
        return finishTask();
    }

    // Go next
    currentIndex ++;

#ifdef TIME
    auto t1 = std::chrono::system_clock::now();
#endif

    // Note: The parameters should be added before metric learning
    goodnessFunction.pList.push_back(x);

    // preference learning
    if (currentIndex > 1)
    {
        computeMetricLearning();
        computeMDS();
    }

    // Update the data points in the goodness function
    // Note: the parameters are already added
    goodnessFunction.fList.clear();
    for (unsigned i = 0; i < currentIndex; ++ i)
    {
        goodnessFunction.fList.push_back(images[i]->getFeatureVector());
    }

    // Compute the adaptive kernel density estimation
    goodnessFunction.computeCovariance();
    goodnessFunction.regularizeCovariance();

#ifdef TIME
    auto t2 = std::chrono::system_clock::now();
    std::cout << "Self-Reinforcement Process: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " [ms]" << std::endl;
#endif

    // sort the rest of photos
    if (useSortingPhotos)
    {
        unsigned optimalIndex = currentIndex;
        double   optimalConf  = computeConfidence(optimalIndex);
        for (unsigned i = currentIndex + 1; i < images.size(); ++ i)
        {
            const double conf = computeConfidence(i);
            if (conf > optimalConf)
            {
                optimalConf  = conf;
                optimalIndex = i;
            }
        }
        const auto tmp = images[currentIndex];
        images[currentIndex] = images[optimalIndex];
        images[optimalIndex] = tmp;
    }

    const shared_ptr<Image> nextImage = images[currentIndex];

    // confidence
    confidence = computeConfidence(currentIndex);
    mainWindow->updateConfidenceValueInUI(confidence);

    // compute local/global best/worst values
    localMax  = goodnessFunction.getValue(goodnessFunction.getBestParameterSet(getCurrentFeatureVector()), getCurrentFeatureVector());
    localMin  = goodnessFunction.getValue(goodnessFunction.getBestParameterSet(getCurrentFeatureVector(), true), getCurrentFeatureVector());

    // Compute optimal parameters for the next photo
    const VectorXd opt = goodnessFunction.getBestParameterSet(getCurrentFeatureVector());
    if (useInitialOptimization)
    {
        parameters = EigenUtility::eigen2std(opt);
    }
    else
    {
        parameters = vector<double>(parameters.size(), 0.5);
    }

    // Update preview
    previewWidget->setCurrentImage(*nextImage->getScaledQImage());

    // Store the enhanced photo
    enhancedImages.push_back(currentImage->getModifiedScaledQImage(x));

    // Update the reference photos
    setReferencePhotos();

    // For study
    getCurrentStudyData().confidence         = confidence;
    getCurrentStudyData().autoParameter      = opt;
    getCurrentStudyData().naiveAutoParameter = goodnessFunction.getAverageParameterSet();
    getCurrentStudyData().index              = currentIndex;
    getCurrentStudyData().targetImage        = images[currentIndex];
    getCurrentStudyData().parameterSequence.push_back(x);
    time_point = std::chrono::system_clock::now();

    // Export embedded map
    exportMap();

    return true;
}

void Core::optimizeParameters(int exclusiveParameter)
{
    // Just return when there is no sufficient data
    if (currentIndex < 2) return;

    const double   scale     = confidence / (localMax - localMin);
    const VectorXd p_current = EigenUtility::std2eigen(parameters);
    const VectorXd f         = images[currentIndex]->getFeatureVector();

    VectorXd p_new     = goodnessFunction.applyGradientAscent(p_current, f, scale);
    p_new(exclusiveParameter) = parameters[exclusiveParameter];
    parameters = EigenUtility::eigen2std(p_new);
}

//////////////////////////////////////////////
// user interface
//////////////////////////////////////////////

int Core::getSizeOfFont() const
{
    const static int sizes[] = { 13, 18, 24, 32 };
    return sizes[uiSize];
}

int Core::getSizeOfVisualizationHeight() const
{
    const static int heights[] = { 30, 36, 42, 50 };
    return heights[uiSize];
}

int Core::getSizeOfButtonHeight() const
{
    const static int heights[] = { 32, 42, 46, 54 };
    return heights[uiSize];
}

int Core::getSizeOfTextBoxWidth() const
{
    const static int widths[] = { 36, 48, 60, 76 };
    return widths[uiSize];
}

int Core::getSizeOfTextBoxHeight() const
{
    const static int heights[] = { 24, 28, 32, 40 };
    return heights[uiSize];
}

int Core::getSizeOfScrollAreaWidth() const
{
    const static int widths[] = { 340, 400, 480, 560 };
    return widths[uiSize];
}
