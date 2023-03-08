#ifndef CODE_MODEL_H
#define CODE_MODEL_H

#include <string.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include "id_tag_table.h"
#include "imports_map.h"
#include "translation_unit.h"

namespace fs = std::filesystem;

struct Field {
    std::string type;
    std::string name;
};

struct Struct {
    std::string name;
    std::vector<Field> fields;
};

class Call {
public:
    std::string name;
    std::vector<std::string> arguments;

    Call(std::string name);
    void addArgument(std::string argument);
};

enum FunctionType {
    CHILD,
    DECORATOR,
    INNER
};

class Function {
public:
    std::string signature;
    std::vector<std::string> body;
    std::string bodyAtom;
    int varCounter;
    FunctionType type;

    Function();
    void addLine(std::string line);
    void setType(FunctionType type, std::string varname);
    std::string getVar();
    std::string nextVarDeclaration();
    std::string getName();
};

class CodeModel {
    std::ofstream outHeader;
    std::ofstream outSource;
    std::string filename;
    TranslationUnit& unit;
    IdTagTable& idTagTable;
    ImportsMap& importsMap;
    std::unordered_set<std::string> sourceImports;
    std::vector<Struct> structs;
    std::vector<Function> functions;

public:
    CodeModel(TranslationUnit& unit, IdTagTable& idTagTable, ImportsMap& importsMap);
    ~CodeModel();
    bool open();
    void addImport(std::string alias);
    void addStdImport(std::string import);
    void addStruct(Struct s);
    void addFunction(Function f);
    void addFunctionAtom(std::string path);
    std::string getFilename();
    bool write();
    void writeDefineBegin();
    void writeImports();
    void writeStructs();
    void writeOffset();
    void writeFunctions();
    void writeDefineEnd();
    IdTagTable& getIdTagTable();
    ImportsMap& getImportsMap();
    std::string getDefine();
    std::string getClassNameByValue(std::string value);
};

std::string genVarName(int num);
std::string genCall(Call call);
std::string genCall(std::string callName);
std::string genCall(std::string callName, std::vector<std::string> argumets);

#endif // CODE_MODEL_H
