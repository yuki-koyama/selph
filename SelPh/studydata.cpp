#include "studydata.h"

#include <iostream>
#include <QImage>
#include "image.h"

using namespace Eigen;
using namespace std;

StudyData::StudyData() : autoEnhanced(false), visualized(false), optimized(false), sliderMovementSum(0.0)
{
}

void StudyData::printFirstRow(ostream& stream)
{
    stream        << "index";
    stream << "," << "file name";
    stream << "," << "use autoenhancement";
    stream << "," << "use visualization";
    stream << "," << "use optimization";
    stream << "," << "elapsed time [ms]";
    stream << "," << "estimation error";
    stream << "," << "naive estimation error";
    stream << "," << "exploration path length";
    stream << "," << "number of mouse events";
    stream << "," << "slider edit distance";
    stream << "," << "confidence";
    stream << endl;
}

void StudyData::print(ostream& stream) const
{
    stream        << index;
    stream << "," << targetImage->getFileName();
    stream << "," << autoEnhanced;
    stream << "," << visualized;
    stream << "," << optimized;
    stream << "," << elapsedTime;
    stream << "," << estimationError();
    stream << "," << naiveEstimationError();
    stream << "," << integratedTweakingDistance();
    stream << "," << numberOfMouseEvents();
    stream << "," << sliderMovementSum,
    stream << "," << confidence;
    stream << endl;
}

void StudyData::exportModifiedScaledPhoto(const string& workingDirectoryPath) const
{
    const shared_ptr<QImage> modifiedImage = targetImage->getModifiedScaledQImage(userParameter);
    modifiedImage->save(QString::fromStdString(workingDirectoryPath + "/result/" + std::to_string(index) + ".jpg"), NULL, 100);
}

void StudyData::exportModifiedOriginalPhoto(const string& workingDirectoryPath) const
{
    const shared_ptr<QImage> modifiedImage = targetImage->getModifiedOriginalQImage(userParameter);
    modifiedImage->save(QString::fromStdString(workingDirectoryPath + "/result/orig-" + std::to_string(index) + ".jpg"), NULL, 100);
}

double StudyData::estimationError() const
{
    return (autoParameter - userParameter).norm();
}

double StudyData::naiveEstimationError() const
{
    return (naiveAutoParameter - userParameter).norm();
}

double StudyData::integratedTweakingDistance() const
{
    const unsigned n = parameterSequence.size();
    double sum = 0.0;
    for (unsigned i = 1; i < n; ++ i) {
        VectorXd x = parameterSequence[i - 1];
        VectorXd y = parameterSequence[i    ];
        sum += (x - y).norm();
    }
    return sum;
}

long StudyData::numberOfMouseEvents() const
{
    return parameterSequence.size() - 1;
}
