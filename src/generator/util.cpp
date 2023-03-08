#include "util.h"
#include <algorithm>

std::string convertSeparator(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
}