#include "core.h"

#include <fstream>
#include <ctime>
#include <QDir>
#include "utility.h"
#include "image.h"

using std::string;
using std::vector;
using std::ostream;
using std::ofstream;
using std::shared_ptr;
using std::make_shared;
using std::cout;
using std::endl;

namespace
{
inline void createDirIfNotExist(string dirPath)
{
    QDir dir(QString::fromStdString(dirPath));
    if (!dir.exists()) dir.mkpath(".");
}
inline string getUniqueName()
{
    vector<char> buffer(200);
    std::time_t t = std::time(NULL);
    std::strftime(&buffer[0], sizeof(char) * 200, "%Y%m%d_%H%M%S", std::localtime(&t));
    return string(&buffer[0]);
}
}

void Core::generateDirectories()
{
    string dirPath = Utility::getTemporaryDirectory();
    createDirIfNotExist(dirPath);

    workingDirectoryPath = dirPath + "/" + getUniqueName();
    createDirIfNotExist(workingDirectoryPath);

    createDirIfNotExist(workingDirectoryPath + "/study");
    createDirIfNotExist(workingDirectoryPath + "/map");
    createDirIfNotExist(workingDirectoryPath + "/result");
}

void Core::printFeatureCoordinates(ostream& stream) const
{
    for (unsigned i = 0; i < images.size(); ++ i)
    {
        const shared_ptr<Image> image = images[i];
        stream << i << "," << image->getFileName();
        const Eigen::VectorXd& coord = image->getFeatureVector();
        for (unsigned i = 0; i < coord.rows(); ++ i) {
            stream << "," << coord(i);
        }
        stream << endl;
    }
}

void Core::importPhotos(const string& dirPath)
{
    const vector<string> fileList = Utility::getPhotoFileList(dirPath);
    for (const string& s : fileList)
    {
        cout << "Importing " << s << " ..." << endl;
        images.push_back(make_shared<Image>(dirPath + s));
    }
}

void Core::exportMap() const
{
    // Dir name
    const string dirName = workingDirectoryPath + "/map/";

    // Csv
    ofstream coord(dirName + std::to_string(currentIndex) + ".csv");
    printFeatureCoordinates(coord);
}

void Core::exportRawDistance() const
{
    // Dir name
    const string dirName = workingDirectoryPath + "/study/raw_distance.txt";

    // Output
    ofstream file(dirName);
    for (vector<vector<double>> tmp1 : distance)
    {
        for (vector<double> tmp2 : tmp1)
        {
            for (double tmp3 : tmp2)
            {
                file << tmp3 << ",";
            }
        }
    }
}

bool Core::finishTask() const
{
    // export the coordinate
    ofstream coord(workingDirectoryPath + "/study/coord.csv");
    printFeatureCoordinates(coord);

    // export the study data
    ofstream study(workingDirectoryPath + "/study/study.csv");
    StudyData::printFirstRow(study);
    for (const StudyData& data : studyData)
    {
        data.print(study);
        data.exportModifiedScaledPhoto(workingDirectoryPath);
        data.exportModifiedOriginalPhoto(workingDirectoryPath);
    }

    // export the actual parameters
    ofstream param(workingDirectoryPath + "/study/params.txt");
    for (const StudyData& data : studyData)
    {
        param << data.userParameter.transpose() << endl;
    }

    return false;
}
