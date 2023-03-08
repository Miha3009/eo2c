#ifndef IMPORTS_MAP_H
#define IMPORTS_MAP_H

#include <string>
#include <unordered_map>
#include <filesystem>
#include "object.h"
#include "translation_unit.h"

namespace fs = std::filesystem;

class ImportsMap {
    fs::path buildPath;
    std::unordered_map<std::string, TranslationUnit*> map;

public:
    void build(fs::path buildPath, std::vector<TranslationUnit>& units);
    TranslationUnit* getUnit(std::string alias);
    std::string getImport(TranslationUnit& from, std::string alias);
    std::string getObjectImport(TranslationUnit& from);
    std::string getClassName(std::string alias);
};

#endif // IMPORTS_MAP_H
