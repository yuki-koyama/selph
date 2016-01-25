#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <vector>

namespace Utility {

std::string getResourceDirectory();
std::string getTemporaryDirectory();
std::vector<std::string> getPhotoFileList(std::string dirPath);

}

#endif // UTILITY_H
