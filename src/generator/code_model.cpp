#include <iostream>
#include <algorithm>
#include <iterator>
#include "code_model.h"
#include "util.h"

Call::Call(std::string name): name{name} {
}

void Call::addArgument(std::string argument) {
    arguments.push_back(argument);
}

Function::Function(): varCounter{0} {
}

void Function::addLine(std::string line) {
    body.push_back(line);
}

void Function::setType(FunctionType type, std::string varname) {
    switch (type)
    {
    case CHILD:
    case INNER:
        body.insert(body.begin(), "StackPos pos = " + genCall("stack_store"));
        body.insert(body.begin(), "EO_object* obj = " + genCall("get_home", {"obj_"}));
        body.push_back(genCall("stack_restore", {"pos"}));
        body.push_back("return " + genCall("move_object", {varname}));
        break;
    case DECORATOR:
        body.insert(body.begin(), "StackPos pos = " + genCall("stack_store"));
        body.push_back(genCall("stack_restore", {"pos"}));
        body.push_back("return " + genCall("move_object", {varname}));
        break;
    default:
        break;
    }
    this->type = type;
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

CodeModel::CodeModel(TranslationUnit& unit, IdTagTable& idTagTable, ImportsMap& importsMap): unit{unit}, idTagTable{idTagTable}, importsMap{importsMap} {
}

CodeModel::~CodeModel() {
    outHeader.close();
    outSource.close();
}

bool CodeModel::open(bool withHeader) {
    if(withHeader) {
        fs::create_directories(unit.buildHeader.parent_path());
        outHeader.open(unit.buildHeader.string());
        if(!outHeader.is_open()) {
            std::cout << "error: File " << unit.buildHeader.string() << " isn't accesible" << std::endl;
            return false;
        }
        sourceImports.insert("\"" + unit.buildHeader.filename().string() + "\"");
    } else {
        outHeader = NullOfstream();
    }
    fs::create_directories(unit.buildCpp.parent_path());
    outSource.open(unit.buildCpp.string());
    if(!outSource.is_open()) {
        std::cout << "error: File " << unit.buildCpp.string() << " isn't accesible" << std::endl;
        return false;
    }
    filename = unit.buildHeader.stem().filename().string();
    return true;
}

bool CodeModel::addImport(std::string alias) {
    std::string import = importsMap.getImport(unit, alias);
    if(import != "") {
        sourceImports.insert("\"" + import + "\"");
    } else {
        std::cout << "File for alias " << alias << " not found\n";
        return false;
    }
    return true;
}

void CodeModel::addStdImport(std::string import) {
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

bool CodeModel::write() {
    try {
        writeDefineBegin();
        writeImports();
        writeStructs();
        writeOffset();
        writeFunctions();
        writeDefineEnd();
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
        return false;
    }
    return true;
}

void CodeModel::writeDefineBegin() {
    outHeader << "#ifndef " << getDefine() << "\n";
    outHeader << "#define " << getDefine() << "\n\n";
}

void CodeModel::writeImports() {
    outHeader << "#include \"" << convertSeparator(importsMap.getObjectImport(unit)) << "\"\n\n";
    for(std::string import : sourceImports) {
        outSource << "#include " << convertSeparator(import) << "\n";
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
            if(idTagTable.getTag(f.name) != "") {
                outSource << "\t{" << idTagTable.getTag(f.name) << ", offsetof(" << s.name << ", " << f.name << ")},\n";
            }
        }
        outSource << "};\n\n";
    }
}

void CodeModel::writeFunctions() {
    for(Function& f : functions) {
        if(f.type != INNER) {
            outHeader << f.signature << ";\n\n";
        }
        outSource << f.signature << " {\n";
        if(!f.bodyAtom.empty()) {
            outSource << f.bodyAtom;
        } else {
            for(std::string line : f.body) {
                outSource << "\t" << line;
                if(line.back() != '{' && line.back() != '}') {
                    outSource << ";";
                }
                outSource << "\n";
            }
        }
        outSource << "}\n\n";
    }
}

void CodeModel::writeDefineEnd() {
    outHeader << "#endif // " << getDefine() << "\n";
}

IdTagTable& CodeModel::getIdTagTable() { 
    return idTagTable;
}

ImportsMap& CodeModel::getImportsMap() {
    return importsMap;
}

std::string CodeModel::getDefine() {
    std::string define = filename + "_H";
    std::transform(define.begin(), define.end(), define.begin(), [](unsigned char c) {
        if((c < '0' || c > '9') && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z')) {
            return '_';
        }
        return (char)std::toupper(c);
    });
    return define;
}

std::string CodeModel::getClassNameByValue(std::string value) {
    std::string alias = "";
    for(Meta& meta : unit.metas) {
        if(meta.type == "alias" && meta.value.length() >= value.length() + 1 &&
           0 == meta.value.compare(meta.value.length() - value.length() - 1, value.length() + 1, "." + value)) {
            alias = meta.value;
            break;
        }
    }
    std::string import = importsMap.getImport(unit, alias);
    if(import == "") {
        return "";
    }
    sourceImports.insert("\"" + import + "\"");
    return importsMap.getObject(alias, value)->getClassName();
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
