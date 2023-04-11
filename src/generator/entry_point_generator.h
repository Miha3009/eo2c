#ifndef ENTRY_POINT_GENERATOR_H
#define ENTRY_POINT_GENERATOR_H

#include <vector>
#include <filesystem>
#include <fstream>
#include "translation_unit.h"
#include "id_tag_table.h"
#include "imports_map.h"
#include "code_model.h"

namespace fs = std::filesystem;

class EntryPointGenerator {
    CodeModel codeModel;
    std::vector<TranslationUnit>& units;
    std::string mainObjectPackage;
    int stackSize;
    std::ofstream out;

public:
    EntryPointGenerator(TranslationUnit& mainUnit, IdTagTable& idTagTable, ImportsMap& importsMap, std::vector<TranslationUnit>& units, std::string mainObjectPackage, int stackSize);
    ~EntryPointGenerator();
    bool run();

private:
    bool writeMain();
    void writeArguments(Function& f, Object* mainObject);
};

#endif //ENTRY_POINT_GENERATOR_H
