#include "code_model.h"
#include <iostream>
#include <algorithm>
#include <iterator>

Call::Call(std::string name): name{name} {
}

void Call::addArgument(std::string argument) {
    arguments.push_back(argument);
}

Function::Function(): varCounter(0) {
}

void Function::addLine(std::string line) {
    body.push_back(line);
}

std::string Function::getVar() {
    return genVarName(varCounter);
}

std::string Function::nextVarDeclaration() {
    return "EO_object* " + genVarName(++varCounter) + " = ";
}

std::string Function::getName() {
    std::string name;
    for(int i = 11; i < signature.size(); ++i) {
        if(signature[i] == '(') {
            break;
        }
        name += signature[i];
    }
    return name;
}

CodeModel::CodeModel(std::unordered_map<std::string, int>& nameTagTable): nameTagTable{nameTagTable} {
}

CodeModel::~CodeModel() {
    outHeader.close();
    outSource.close();
}

bool CodeModel::open(std::filesystem::path path) {
    if(!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }
    path.replace_extension("h");
    outHeader.open(path.string());
    if(!outHeader.is_open()) {
        std::cout << "error: File " << path.string() << " isn't accesible" << std::endl;
        return false;
    }
    path.replace_extension("cpp");
    outSource.open(path.string());
    if(!outSource.is_open()) {
        std::cout << "error: File " << path.string() << " isn't accesible" << std::endl;
        return false;
    }
    path.replace_extension("");
    name = path.filename().string();
    return true;
}

void CodeModel::addHeaderImport(Import import) {
    headerImports.insert(import);
}

void CodeModel::addSourceImport(Import import) {
    sourceImports.insert(import);
}

void CodeModel::addStruct(Struct s) {
    structs.push_back(s);
}

void CodeModel::addFunction(Function f) {
    functions.push_back(f);
}

void CodeModel::addFunctionAtom(std::string path) {
    std::ifstream in(path);
    if(in.fail()) {
        std::cout << "error: File " << path << " isn't accesible" << std::endl;
        return;
    }
    std::string line;
    Function f;
    while (std::getline(in, line)) {
        if(line.empty() || line[0] == '}') continue;
        if(line.find("#include") == 0) {
            sourceImports.insert(line.substr(9));
        } else if(line.find("EO_object*") == 0) {
            f.signature = line.substr(0, line.size()-2);
        } else {
            f.bodyAtom += line + "\n";
        }
    }
    functions.push_back(f);
    in.close();
}

std::string CodeModel::getName() {
    return name;
}

void CodeModel::write() {
    writeDefineBegin();
    writeImports();
    writeStructs();
    writeOffset();
    writeFunctions();
    writeDefineEnd();
}

void CodeModel::writeDefineBegin() {
    outHeader << "#ifndef " << getDefine() << "\n";
    outHeader << "#define " << getDefine() << "\n\n";
}

void CodeModel::writeImports() {
    for(Import import : headerImports) {
        outHeader << "#include " << import << "\n";
    }
    outHeader << "\n";
    for(Import import : sourceImports) {
        outSource << "#include " << import << "\n";
    }
    outSource << "\n";
}

void CodeModel::writeStructs() {
    for(Struct& s : structs) {
        outHeader << "struct " << s.name << "{\n";
        for(Field& f : s.fields) {
            outHeader << "\t" << f.type << " " << f.name << ";\n";
        }
        outHeader << "};\n\n";
    }
}

void CodeModel::writeOffset() {
    for(Struct& s : structs) {
        if(s.fields.empty()) {
            continue;
        }
        outSource << "static const std::unordered_map<Tag, int> offset_" << s.name << " {\n";
        for(Field& f : s.fields) {
            if(nameTagTable.count(f.name) == 1) {
                outSource << "\t{" << getTag(f.name) << ", offsetof(" << s.name << ", " << f.name << ")},\n";
            }
        }
        outSource << "};\n\n";
    }
}

void CodeModel::writeFunctions() {
    for(Function& f : functions) {
        outHeader << f.signature << ";\n\n";
        outSource << f.signature << " {\n";
        if(!f.bodyAtom.empty()) {
            outSource << f.bodyAtom;
        } else {
            for(std::string line : f.body) {
                outSource << "\t" << line << ";\n";
            }
        }
        outSource << "}\n\n";
    }
}

void CodeModel::writeDefineEnd() {
    outHeader << "#endif // " << getDefine() << "\n";
}

std::string CodeModel::getTag(std::string name) {
    return std::to_string(nameTagTable[name]);
}

std::string CodeModel::getDefine() {
    std::string define = name + "_H";
    std::transform(define.begin(), define.end(), define.begin(), ::toupper);
    return define;
}

std::string genVarName(int num) {
    return "var_" + std::to_string(num);
}

std::string genCall(Call call) {
    return genCall(call.name, call.arguments);
}

std::string genCall(std::string callName) {
    return genCall(callName, {});
}

std::string genCall(std::string callName, std::vector<std::string> argumets) {
    std::string result = callName + "(";
    for(int i = 0; i < argumets.size(); ++i) {
        if(i != 0) {
            result += ", ";
        }
        result += argumets[i];
    }
    result += ")";
    return result;
}
