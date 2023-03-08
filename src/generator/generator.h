#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <filesystem>
#include <unordered_map>
#include "object.h"
#include "translation_unit.h"
#include "code_model.h"
#include "id_tag_table.h"
#include "imports_map.h"

namespace fs = std::filesystem;

class Generator {
    TranslationUnit& unit;
    CodeModel codeModel;
    Function curFunction;
    fs::path exeDir;

public:
    Generator(TranslationUnit& unit, IdTagTable& idTagTable, ImportsMap& importsMap, fs::path exeDir);
    bool run();

private:
    void genModel(Object* obj);
    void genStruct(Object* obj);
    void genOffsetMap(Object* obj);
    void genInit(Object* obj);
    void genEval(Object* obj);
    std::string getValueType(Object* obj);
    std::string getTemplate(Object* obj);
    std::string genEvalSignature(Object* obj, std::string param);
};

#endif // GENERATOR_H
