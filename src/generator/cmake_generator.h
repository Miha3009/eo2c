#ifndef CMAKE_GENERATOR_H
#define CMAKE_GENERATOR_H

#include <vector>
#include <filesystem>
#include <fstream>
#include "translation_unit.h"

namespace fs = std::filesystem;

class CmakeGenerator {
    std::vector<TranslationUnit>& units;
    fs::path buildPath;
    std::ofstream out;

public:
    CmakeGenerator(std::vector<TranslationUnit>& units, fs::path buildPath);
    ~CmakeGenerator();
    bool open();
    void run();

private:
    void writeHead();
    void writeIncludeDirectories();
    void writeSetSources();
    void writeTail();
};

#endif //CMAKE_GENERATOR_H
