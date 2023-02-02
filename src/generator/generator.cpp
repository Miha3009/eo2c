#include <iostream>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include "generator.h"

bool gen(Object* root, std::vector<Meta> metas, std::filesystem::path path, std::unordered_map<std::string, int>& nameTagTable) {
    if(!root) {
        return false;
    }
    Generator g(root, metas, nameTagTable);
    if(!g.openFiles(path)) {
        return false;
    }
    g.run();
    return true;
}

Generator::Generator(Object* root, std::vector<Meta> metas, std::unordered_map<std::string, int>& nameTagTable):
    root{root}, nameTagTable{nameTagTable}, metas{metas}
{
}

Generator::~Generator() {
    outHeader.close();
    outSource.close();
}

bool Generator::openFiles(std::filesystem::path path) {
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
    filename = path.filename().string();
    return true;
}

void Generator::run() {
    if(root->getName().empty()) {
        root->setName(filename);
    }
    genHead();
    genBody(root);
    genTail();
}

void Generator::genHead() {
    outHeader << "#ifndef " << getDefine() << "\n";
    outHeader << "#define " << getDefine() << "\n\n";
    outHeader << "#include \"object.h\"\n\n";
    outSource << "#include \"" << filename << ".h\"\n";
    genDataImports();
    outSource << "\n";
}

void Generator::genBody(Object* obj) {
    for(Object* child : obj->getChildren()) {
        genBody(child);
    }
    if(obj->getType() == CLASS_TYPE) {
        genStruct(obj);
        genSize(obj);
        genEval(obj);
        genInit(obj);
    }
}

void Generator::genTail() {
    outHeader << "#endif // " << getDefine() << "\n";
}

void Generator::genStruct(Object* obj) {
    outHeader << "struct " << obj->getFullName() << " {\n";
    outHeader << "\tEO_head head;\n";
    std::vector<Field> fields;
    for(Attribute& a : obj->getAttributes()) {
        if(a.isVararg) {
            fields.push_back({"EO_object**", a.name});
            fields.push_back({"int", a.name + "_length"});
        } else {
            fields.push_back({"EO_object*", a.name});
        }
    }
    for(Object* child : obj->getChildren()) {
        if(child->getType() == CLASS_TYPE) {
            fields.push_back({child->getFullName(), child->getName()});
        }
    }
    outSource << "static const std::unordered_map<Tag, int> offset_" << obj->getFullName() << " {\n";
    for(Field& field : fields) {
        outHeader << "\t" << field.type << " " << field.name << ";\n";
        outSource << "\t{" << getTag(field.name) << ", offsetof(" << obj->getFullName() << ", " << field.name << ")},\n";
    }
    if(!getValueType(obj).empty()) {
        outHeader << "\t" << getValueType(obj) << " value;\n";
    }
    outHeader << "};\n\n";
    outSource << "};\n\n";
}

void Generator::genInit(Object* obj) {
    varCounter = 0;
    std::string signature = genInitSignature(obj);
    std::string fullName = obj->getFullName();
    std::vector<Attribute> attributes = obj->getAttributes();
    outHeader << signature << ";\n\n";
    outSource << signature << " {\n";
    int varargs = -1;
    if(!attributes.empty() && attributes.back().isVararg) {
        varargs = attributes.size() - 1;
        outSource << "\tobj->" << attributes.back().name << "_length = 0;\n";
    }
    outSource << "\tinit_head((EO_object*)obj, parent_offset, eval_" << fullName << ", size_" << fullName << ", &offset_"
              << fullName << ", " << varargs << ");\n";
    for(Object* child : obj->getChildren()) {
        if(child->getType() == CLASS_TYPE) {
            outSource << "\tinit_" << child->getFullName() << "(&obj->" << child->getName() << ", offsetof("
                      << obj->getFullName() << ", " << child->getName() << "));\n";
        } else if(child->getType() == APPLICATION_TYPE && !child->isDecorator()) {
            int resultVar = genApplication(child, "obj", false);
            outSource << "\tobj->" << obj->getName() << " = " << getVarName(resultVar) << ";\n";
        }
    }
    if(!getValueType(obj).empty()) {
        outSource << "\tobj->value = value;\n";
    }
    outSource << "}\n\n";
}

void Generator::genEval(Object* obj) {
    varCounter = 0;
    outHeader << "EO_object* eval_" << obj->getFullName() << "(EO_object* obj);\n\n";
    if(obj->isAtom()) {
        writeSourcesFromFile("templates/" + getPackage() + "/" + obj->getFullName() + ".cpp");
        outSource << "\n\n";
    } else {
        outSource << "EO_object* eval_" << obj->getFullName() << "(EO_object* obj) {\n";
        std::vector<Object*> children = obj->getChildren();
        auto it = std::find_if(children.begin(), children.end(), [](Object* o){ return o->isDecorator(); });
        if(it == children.end()) {
            outSource << "\treturn obj;\n";
        } else {
            outSource << "\tStackPos pos = stack_store();\n";
            int resultVar = genApplication(*it, "obj", false);;
            outSource << "\tstack_restore(pos);\n";
            outSource << "\treturn stack_move(" << getVarName(resultVar) << ");\n";
        }
        outSource << "}\n\n";
    }
}

void Generator::genSize(Object* obj) {
    outHeader << "size_t size_" << obj->getFullName() << "(EO_object* obj);\n\n";
    outSource << "size_t size_" << obj->getFullName() << "(EO_object* obj) {\n";
    outSource << "\treturn sizeof(" << obj->getFullName() << ");\n";
    outSource << "}\n\n";
}

int Generator::genApplication(Object* obj, std::string varName, bool isHead) {
    std::vector<int> attributeVars;
    for(Object* a : obj->getApplicationAttributes()) {
        attributeVars.push_back(genApplicationPart(a, varName, false, false));
    }
    int headVar = genApplicationPart(obj->getApplicationHead(), varName, !obj->getApplicationAttributes().empty(), true);
    if(!obj->getApplicationAttributes().empty()) {
        outSource << "\tadd_attributes(" << getVarName(headVar);
        for(int attributeVar : attributeVars) {
            outSource << ", " << getVarName(attributeVar);
        }
        outSource << ");\n";
        outSource << "\t" << getVarName(headVar) << " = evaluate(" << getVarName(headVar) << ");\n";
    }
    if(isHead && obj->getParent()->getType() == APPLICATION_TYPE && !obj->getParent()->getApplicationAttributes().empty()) {
        genMethods(obj, true);
        headVar = varCounter - 1;
    } else {
        genMethods(obj, false);
        headVar = varCounter - 1;
    }
    return headVar;
}

int Generator::genApplicationPart(Object* obj, std::string varName, bool hasAttributes, bool isHead) {
    if(obj->getType() == APPLICATION_TYPE) {
        return genApplication(obj, getVarName(varCounter), isHead);
    } else {
        outSource << "\tEO_object* " << getVarName(varCounter) << " = ";
        if(obj->getType() == VAR_TYPE) {
            genVar(obj);
        } else if(obj->getType() == REF_TYPE) {
            genRef(obj, "obj");
        } else {
            genData(obj);
        }
        varCounter++;
        genMethods(obj, hasAttributes);
        return varCounter - 1;
    }
}

std::string Generator::genInitSignature(Object* obj) {
    std::string signature = "void init_" + obj->getFullName() + "(" + obj->getFullName() + "* obj, int parent_offset";
    if(!getValueType(obj).empty()) {
        signature += ", " + getValueType(obj) + " value";
    }
    signature += ")";
    return signature;
}

std::string Generator::getValueType(Object* obj) {
    if(obj->getName() == "EO_bool") {
        return "bool";
    } else if(obj->getName() == "EO_int") {
        return "long long";
    } else if(obj->getName() == "EO_float") {
        return "double";
    } else if(obj->getName() == "EO_string") {
        return "std::string";
    } else if(obj->getName() == "bytes") {
        return "bytes_value";
    }
    return "";
}

std::string Generator::genGetSub(Object* obj, std::string varName) {
    if(obj == nullptr) {
        return varName;
    }

    std::string result = varName;
    bool first = true;
    while(true) {
        if(obj->getType() == VAR_TYPE || obj->getType() == METHOD_TYPE) {
            if(first && varName == "obj") {
                result += "->" + obj->getValue();
            } else {
                result = "get_sub(" + result + ", " + getTag(obj->getValue()) + ")";
            }
        } else if(obj->getType() == REF_TYPE) {
            if(obj->getValue() == "^") {
                result = "get_parent(" + result + ")";
            }
        }
        if(obj->getChildren().empty()) {
            break;
        }
        obj = obj->getChildren()[0];
        first = false;
    }
    return result;
}

void Generator::genVar(Object* obj) {
    Object* classObj = obj->getClassObject();
    if(obj->getValue() == classObj->getName()) {
        outSource << "stack_alloc(sizeof(" << classObj->getFullName() << "));\n";
        outSource << "\tinit_" << classObj->getFullName() << "((" << classObj->getFullName() << "*)" << getVarName(varCounter)
                  << ", ((StackPos)" << getVarName(varCounter) << "-(StackPos)obj));\n";
        return;
    }
    bool isAttribute = false;
    for(Attribute& a : classObj->getAttributes()) {
        if(a.name == obj->getValue()) {
            isAttribute = true;
            break;
        }
    }
    outSource << "(EO_object*)";
    if(!isAttribute) {
        outSource << "&";
    }
    outSource << "((" << classObj->getFullName() << "*)obj)->" << obj->getValue() << ";\n";
}

void Generator::genRef(Object* obj, std::string varName) {
    if(obj->getValue() == "^") {
        outSource << "get_parent(" << varName << ");\n";
    }
}

void Generator::genData(Object* obj) {
    std::string type;
    std::string value = obj->getValue();
    bool first = true;
    switch(obj->getType()) {
    case INT_TYPE:
        type = "EO_int";
        value += "LL";
        break;
    case FLOAT_TYPE:
        type = "EO_float";
        break;
    case BOOL_TYPE:
        type = "EO_bool";
        break;
    case STRING_TYPE:
        type = "EO_string";
        break;
    case BYTES_TYPE:
        type = "bytes";
        value = "make_bytes(";
        for(Object* child : obj->getChildren()) {
            if(child->getType() == BYTE_TYPE) {
                if(!first) {
                    value += ", ";
                }
                value += "(unsigned char)0x" + child->getValue();
                first = false;
            }
        }
        value += ")";
        break;
    default:
        return;
    }
    outSource << "stack_alloc(sizeof(" << type << "));\n";
    outSource << "\tinit_" << type << "((" << type << "*)" << getVarName(varCounter) << ", 0, " << value << ");\n";
}

void Generator::genMethods(Object* obj, bool hasAttributes) {
    std::vector<Object*> children = obj->getChildren();
    for(int i = 0; i < children.size(); ++i) {
        if(children[i]->getType() == METHOD_TYPE) {
            outSource << "\tEO_object* " << getVarName(varCounter) << " = ";
            std::string tmp = "get_sub(" + getVarName(varCounter - 1) + ", " + getTag(children[i]->getValue()) + ")";
            if(i == children.size() - 1 && hasAttributes) {
                outSource << tmp << ";\n";
            } else {
                outSource << "evaluate(" << tmp << ");\n";
            }
            ++varCounter;
        } else if(children[i]->getType() == REF_TYPE) {
            genRef(obj, getVarName(varCounter - 1));
        }
    }
}

std::string Generator::getTag(std::string name) {
    return std::to_string(nameTagTable[name]);
}

std::string Generator::getDefine() {
    std::string define = filename + "_H";
    std::transform(define.begin(), define.end(), define.begin(), ::toupper);
    return define;
}

std::string Generator::getPackage() {
    for(Meta& m : metas) {
        if(m.type == "package") {
            std::string package = m.value;
            std::replace(package.begin(), package.end(), '.', '/');
            return package;
        }
    }
    return "";
}

std::string Generator::getVarName(int num) {
    return "var_" + std::to_string(num);
}

void Generator::genDataImports() {
    std::unordered_set<std::string> imports;
    std::vector<Object*> objStack;
    objStack.push_back(root);
    while(!objStack.empty()) {
        Object* obj = objStack.back();
        objStack.pop_back();
        switch(obj->getType()) {
        case INT_TYPE:
            imports.insert("int.h");
            break;
        case FLOAT_TYPE:
            imports.insert("float.h");
            break;
        case BOOL_TYPE:
            imports.insert("bool.h");
            break;
        case BYTES_TYPE:
            imports.insert("bytes.h");
            break;
        case STRING_TYPE:
            imports.insert("string.h");
            break;
        }
        for(Object* child : obj->getChildren()) {
            objStack.push_back(child);
        }
    }
    for(std::string import : imports) {
        outSource << "#include \"" << import << "\"\n";
    }
}

void Generator::writeSourcesFromFile(std::string path) {
    std::ifstream in(path);
    if(in.fail()) {
        std::cout << "error: File " << path << " isn't accesible" << std::endl;
        return;
    }
    outSource << in.rdbuf();
}
