#ifndef COMPILER_H
#define COMPILER_H

#include <filesystem>
#include <vector>
#include <unordered_map>
#include "translation_unit.h"
#include "config.h"
#include "imports_map.h"

namespace fs = std::filesystem;

class Compiler {
    Config& config;
    ImportsMap importsMap;

public:
    Compiler(Config& config);
    bool generate();
    bool compile();
    bool dataize();
    void configure();
    void clean();

private:
    bool validateArguments();
    std::vector<TranslationUnit> makeUnits();
    bool parse(std::vector<TranslationUnit>& units);
    bool optimize(std::vector<TranslationUnit>& units);
    bool genSources(std::vector<TranslationUnit>& units);
    bool copyBaseFiles();
    bool genMain(std::vector<TranslationUnit>& units);
    bool genCmake(std::vector<TranslationUnit>& units);
    bool runMake();
    bool runExecutable();
    void addUnitsFromPath(fs::path sourceDir, std::vector<TranslationUnit>& units);
    std::string readFile(fs::path file);
};

#endif // COMPILER_H
