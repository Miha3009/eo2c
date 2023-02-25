#ifndef CODE_MODEL_H
#define CODE_MODEL_H

#include <string.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <filesystem>

typedef std::string Import;

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

class Function {
public:
    std::string signature;
    std::vector<std::string> body;
    std::string bodyAtom;
    int varCounter;

    Function();
    void setSignature(std::string signature);
    void addLine(std::string line);
    std::string getVar();
    std::string nextVarDeclaration();
    std::string getName();
};

class CodeModel {
    std::ofstream outHeader;
    std::ofstream outSource;
    std::string name;
    std::unordered_map<std::string, int>& nameTagTable;
    std::unordered_set<Import> headerImports;
    std::unordered_set<Import> sourceImports;
    std::vector<Struct> structs;
    std::vector<Function> functions;

public:
    CodeModel(std::unordered_map<std::string, int>& nameTagTable);
    ~CodeModel();
    bool open(std::filesystem::path path);
    void addHeaderImport(Import import);
    void addSourceImport(Import import);
    void addStruct(Struct s);
    void addFunction(Function f);
    void addFunctionAtom(std::string path);
    std::string getName();
    void write();
    void writeDefineBegin();
    void writeImports();
    void writeStructs();
    void writeOffset();
    void writeFunctions();
    void writeDefineEnd();
    std::string getTag(std::string name);
    std::string getDefine();
};

std::string genVarName(int num);
std::string genCall(Call call);
std::string genCall(std::string callName);
std::string genCall(std::string callName, std::vector<std::string> argumets);

#endif // CODE_MODEL_H
