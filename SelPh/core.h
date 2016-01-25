#ifndef CORE_H
#define CORE_H

#include <vector>
#include <iostream>
#include <memory>
#include <chrono>
#include <Eigen/Core>
#include "studydata.h"
#include "goodnessfunction.h"

class MainWindow;
class PreviewWidget;
class Image;
class QImage;

class Core
{
public:
    Core();

    static Core& getInstance()
    {
        static Core core;
        return core;
    }

    // constant values
    int parameterDim;
    const int featureDim;

    // parameters
    std::vector<double> parameters;

    // This member is computed in the initialization stage only once
    typedef std::vector<std::vector<std::vector<double>>> Distance;
    Distance distance;

    static double computeDistance(const Eigen::VectorXd& alpha, const Distance& D, unsigned index1, unsigned index2)
    {
        double d = 0.0;
        const unsigned n = alpha.rows();
        for (unsigned i = 0; i < n; ++ i)
        {
            d += alpha(i) * D[index1][index2][i];
        }
        return d;
    }

    // for metric learning
    Eigen::VectorXd alpha;
    void computeMetricLearning();

    // user interface
    MainWindow*    mainWindow;
    PreviewWidget* previewWidget;

    // Reference photo management
    const unsigned nReferencePhotos;
    std::vector<std::shared_ptr<QImage>> enhancedImages;

    // image managements
    unsigned currentIndex;
    void initialize(const std::string& dirPath);
    std::vector<std::shared_ptr<Image>> images;
    Eigen::VectorXd getCurrentFeatureVector() const;

    // for visualization management
    double localMax;
    double localMin;
    double confidence;
    double computeConfidence(unsigned index) const;

    // go to the next image
    bool goNext();

    // for study
    StudyData& getCurrentStudyData() { return studyData[currentIndex]; }
    std::vector<StudyData> studyData;
    std::chrono::system_clock::time_point time_point;
    bool isBaselineMode;

    // size of user interface
    int uiSize;                                 // 0, 1, 2, 3
    int getSizeOfVisualizationHeight() const;
    int getSizeOfFont()                const;
    int getSizeOfButtonHeight()        const;
    int getSizeOfTextBoxWidth()        const;
    int getSizeOfTextBoxHeight()       const;
    int getSizeOfScrollAreaWidth()     const;

    // options
    bool useVisualization;
    bool useOptimization;
    bool useInitialOptimization;
    bool useSortingPhotos;       // This will never be used (always false)

    int nIterations;

    GoodnessFunction goodnessFunction;

    int gradationResolution;

    void optimizeParameters(int exclusiveParameter);

    // Export / Import methods
    void printFeatureCoordinates(std::ostream& stream = std::cout) const;

private:

    // Multi-dimensional scaling
    Eigen::MatrixXd D;
    void computeMDS();

    void setReferencePhotos();

    // Export / Import methods
    void importPhotos(const std::string& dirPath);
    bool finishTask() const;
    void exportMap() const;
    void exportRawDistance() const;
    void generateDirectories();

    // Set once in the initialization
    std::string workingDirectoryPath;
};

#endif // CORE_H
