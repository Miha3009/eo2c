#include <iostream>
#include <unordered_set>
#include <algorithm>
#include "generator.h"
#include "application_generator.h"

Generator::Generator(TranslationUnit& unit, IdTagTable& idTagTable, ImportsMap& importsMap, fs::path exeDir):
    unit{unit}, codeModel{unit, idTagTable, importsMap}, exeDir{exeDir} {
}

bool Generator::run() {
    if(!codeModel.open()) {
        return false;
    }
    genModel(unit.root);
    return codeModel.write();
}

void Generator::genModel(Object* obj) {
    for(Object* child : obj->getChildren()) {
        genModel(child);
    }
    if(obj->getType() == CLASS_TYPE && !obj->isDecorator() && !obj->isRoot()) {
        genStruct(obj);
        genEval(obj);
        genInit(obj);
    }
}

void Generator::genStruct(Object* obj) {
    Struct s;
    s.name = obj->getClassName();
    s.fields.push_back({"EO_head", "head"});
    for(Attribute& a : obj->getAttributes()) {
        s.fields.push_back({"int", a.name});
    }
    for(Object* child : obj->getChildren()) {
        if(child->isDecorator()) {
            continue;
        }
        if(child->getType() == CLASS_TYPE) {
            s.fields.push_back({child->getClassName(), child->getValue()});
        } else if(child->getType() == APPLICATION_TYPE) {
            s.fields.push_back({"EO_object", child->getValue()});
        }
    }
    if(!getValueType(obj).empty()) {
        s.fields.push_back({getValueType(obj), "value"});
    }
    codeModel.addStruct(s);
}

void Generator::genInit(Object* obj) {
    Function f;
    std::vector<std::string> signatureArgs = {
        obj->getClassName() + "* obj",
        "int parent_offset"
    };
    if(!getValueType(obj).empty()) {
        signatureArgs.push_back(getValueType(obj) + " value");
    }
    f.signature = genCall("void init_" + obj->getClassName(), signatureArgs);
    std::vector<Attribute> attributes = obj->getAttributes();
    int varargs = -1;
    if(!attributes.empty() && attributes.back().isVararg) {
        varargs = attributes.size() - 1;
    }
    std::string offset_var = "&offset_" + obj->getClassName();
    if(attributes.empty() && obj->getChildren().empty()) {
        offset_var = "&offset_default";
    }
    f.addLine(genCall("init_head", {
        "(EO_object*)obj",
        "eval_" + obj->getClassName(),
        "parent_offset",
        std::to_string(varargs),
        genCall("sizeof", {obj->getClassName()}),
        offset_var,
    }));
    if(obj->getType() == CLASS_TYPE) {
        for(Object* child : obj->getChildren()) {
            if(child->isDecorator()) {
                continue;
            }
            if(child->getType() == CLASS_TYPE) {
                f.addLine(genCall("init_" + child->getClassName(), {
                    "&obj->" + child->getValue(),
                    genCall("offsetof", {obj->getClassName(), child->getValue()})
                }));
            } else if(child->getType() == APPLICATION_TYPE) {
                ApplicationGenerator appGen(child, codeModel, genEvalSignature(child, "obj_"));
                appGen.run();
                appGen.getFunction().setType(CHILD, appGen.getResultVar());
                codeModel.addFunction(appGen.getFunction());
                f.addLine(genCall("init_head", {
                    "&obj->" + child->getValue(),
                    appGen.getFunction().getName(),
                    genCall("offsetof", {obj->getClassName(), child->getValue()}),
                    "-1",
                    "sizeof(EO_object)",
                    "&offset_default"
                }));
                genInit(child);
                Struct s;
                s.name = child->getClassName();
                s.fields.push_back({"EO_head", "head"});
                codeModel.addStruct(s);
            }
        }
        for(Attribute a : obj->getAttributes()) {
            f.addLine("obj->" + a.name + " = 0");
        }
        if(!getValueType(obj).empty()) {
            f.addLine("obj->value = value");
        }
        if(obj->getClassName() == "bytes") {
            f.addLine("obj->head.size += sizeof(unsigned char) * obj->value.length");
        } else if(obj->getClassName() == "EO_string") {
            f.addLine("obj->head.size += sizeof(wchar_t) * obj->value.length");
        }
    }
    codeModel.addFunction(f);
}

void Generator::genEval(Object* obj) {
    if(obj->hasFlags(ATOM_FLAG)) {
        codeModel.addFunctionAtom(getTemplate(obj));
        return;
    }
    std::vector<Object*> children = obj->getChildren();
    auto it = std::find_if(children.begin(), children.end(), [](Object* o){ return o->isDecorator(); });
    Function f;
    if(it == children.end()) {
        f.signature = genEvalSignature(obj, "obj");
        f.addLine("return obj");
    } else {
        if((*it)->hasFlags(ATOM_FLAG)) {
            codeModel.addFunctionAtom(getTemplate(obj));
            return;
        }
        ApplicationGenerator appGen(*it, codeModel, genEvalSignature(obj, "obj"));
        appGen.run();
        f = appGen.getFunction();
        f.setType(DECORATOR, appGen.getResultVar());
    }
    codeModel.addFunction(f);
}

std::string Generator::getValueType(Object* obj) {
    if(obj->getOriginValue() == "bool") {
        return "bool";
    } else if(obj->getOriginValue() == "int") {
        return "long long";
    } else if(obj->getOriginValue() == "float") {
        return "double";
    } else if(obj->getOriginValue() == "string") {
        return "string_value";
    } else if(obj->getOriginValue() == "bytes") {
        return "bytes_value";
    } else if(obj->getOriginValue() == "array") {
        return "int";
    }
    return "";
}

std::string Generator::getTemplate(Object* obj) {
    std::string className = obj->isDecorator() ? obj->getParent()->getClassName() : obj->getClassName();
    for(Meta& m : unit.metas) {
        if(m.type == "package") {
            std::string package = m.value;
            std::replace(package.begin(), package.end(), '.', '/');
            return (exeDir / fs::path("templates") / fs::path(package) / fs::path(className + ".cpp")).string();
        }
    }
    return "";
}

std::string Generator::genEvalSignature(Object* obj, std::string param) {
    return genCall("EO_object* eval_" + obj->getClassName(), {"EO_object* " + param});
}
