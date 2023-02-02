#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include "object.h"

struct Field {
    std::string type;
    std::string name;
};

bool gen(Object* root, std::vector<Meta> metas, std::filesystem::path path, std::unordered_map<std::string, int>& nameTagTable);

class Generator {
    Object* root;
    std::ofstream outHeader;
    std::ofstream outSource;
    std::string filename;
    std::unordered_map<std::string, int>& nameTagTable;
    std::vector<Meta> metas;
    int varCounter;

public:
    Generator(Object* root, std::vector<Meta> metas, std::unordered_map<std::string, int>& nameTagTable);
    ~Generator();
    bool openFiles(std::filesystem::path path);
    void run();

private:
    void genHead();
    void genBody(Object* obj);
    void genTail();
    void genStruct(Object* obj);
    void genOffsetMap(Object* obj);
    void genInit(Object* obj);
    void genEval(Object* obj);
    void genSize(Object* obj);
    int genApplication(Object* obj, std::string varName, bool isHead);
    int genApplicationPart(Object* obj, std::string varName, bool hasAttributes, bool isHead);
    std::string genInitSignature(Object* obj);
    std::string getValueType(Object* obj);
    std::string genGetSub(Object* obj, std::string varName);
    void genVar(Object* obj);
    void genRef(Object* obj, std::string varName);
    void genData(Object* obj);
    void genMethods(Object* obj, bool hasAttributes);
    std::string getTag(std::string name);
    std::string getDefine();
    std::string getPackage();
    std::string getVarName(int num);
    void genDataImports();
    void writeSourcesFromFile(std::string path);
};

#endif // GENERATOR_H
