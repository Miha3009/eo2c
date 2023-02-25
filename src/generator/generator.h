#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <filesystem>
#include <unordered_map>
#include "object.h"
#include "code_model.h"

bool gen(Object* root, std::vector<Meta> metas, std::filesystem::path path, std::unordered_map<std::string, int>& nameTagTable);

class Generator {
    Object* root;
    CodeModel& codeModel;
    Function curFunction;
    std::vector<Meta> metas;

public:
    Generator(Object* root, std::vector<Meta> metas, CodeModel& codeModel);
    void run();

private:
    void genModel(Object* obj);
    void genStruct(Object* obj);
    void genOffsetMap(Object* obj);
    void genInit(Object* obj);
    void genEval(Object* obj);
    std::string getValueType(Object* obj);
    std::string getPackage();
    std::string genEvalSignature(Object* obj, std::string param);
};

#endif // GENERATOR_H
