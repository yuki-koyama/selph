#include "utility.h"

#include <QApplication>
#include <QDir>

using namespace std;

namespace Utility {

string getResourceDirectory()
{
    return QCoreApplication::applicationDirPath().toStdString() + "/../Resources/";
}

string getTemporaryDirectory()
{
    return QCoreApplication::applicationDirPath().toStdString() + "/../StudyData/";
}

vector<string> getPhotoFileList(string dirPath)
{
    QDir path(dirPath.c_str());
    QStringList filters;
    filters << "*.jpg" << "*.JPG";
    QStringList list = path.entryList(filters);
    vector<string> result;
    for (QString& s : list) {
        result.push_back(s.toStdString());
    }
    return result;
}

}
