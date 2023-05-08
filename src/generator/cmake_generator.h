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
    bool lib;
    std::string project;

public:
    CmakeGenerator(std::vector<TranslationUnit>& units, fs::path buildPath, bool lib, std::string project);
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
