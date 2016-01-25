#ifndef STUDYDATA_H
#define STUDYDATA_H

#include <memory>
#include <vector>
#include <iostream>
#include <Eigen/Core>

class Image;

struct StudyData
{
    StudyData();

    // input
    unsigned                     index;
    bool                         autoEnhanced;
    bool                         visualized;
    bool                         optimized;
    Eigen::VectorXd              autoParameter;
    Eigen::VectorXd              naiveAutoParameter;
    Eigen::VectorXd              userParameter;
    std::shared_ptr<Image>       targetImage;
    std::vector<Eigen::VectorXd> parameterSequence;
    double                       sliderMovementSum;
    double                       confidence;
    long                         elapsedTime; // [ms]

    // util
    long   numberOfMouseEvents() const;
    double integratedTweakingDistance() const;
    double estimationError() const;
    double naiveEstimationError() const;

    // export
    static void printFirstRow(std::ostream& stream = std::cout);
    void print(std::ostream& stream = std::cout) const;
    void exportModifiedScaledPhoto  (const std::string& workingDirectoryPath) const;
    void exportModifiedOriginalPhoto(const std::string& workingDirectoryPath) const;
};

#endif // STUDYDATA_H
