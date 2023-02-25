#include <iostream>
#include <unordered_set>
#include <algorithm>
#include "generator.h"
#include "application_generator.h"

bool gen(Object* root, std::vector<Meta> metas, std::filesystem::path path, std::unordered_map<std::string, int>& nameTagTable) {
    if(!root) {
        return false;
    }
    CodeModel model = CodeModel(nameTagTable);
    if(!model.open(path)) {
        return false;
    }
    Generator g(root, metas, model);
    g.run();
    return true;
}

Generator::Generator(Object* root, std::vector<Meta> metas, CodeModel& codeModel): root{root}, metas{metas}, codeModel{codeModel} {
}

void Generator::run() {
    codeModel.addHeaderImport("\"object.h\"");
    codeModel.addSourceImport("\"" + codeModel.getName() + ".h\"");
    genModel(root);
    codeModel.write();
}

void Generator::genModel(Object* obj) {
    for(Object* child : obj->getChildren()) {
        genModel(child);
    }
    if(obj->getType() == CLASS_TYPE) {
        genStruct(obj);
        genEval(obj);
        genInit(obj);
    }
}

void Generator::genStruct(Object* obj) {
    Struct s;
    s.name = obj->getFullName();
    s.fields.push_back({"EO_head", "head"});
    for(Attribute& a : obj->getAttributes()) {
        s.fields.push_back({"int", a.name});
    }
    if(!getValueType(obj).empty()) {
        s.fields.push_back({getValueType(obj), "value"});
    }
    for(Object* child : obj->getChildren()) {
        if(child->getType() == CLASS_TYPE) {
            s.fields.push_back({child->getFullName(), child->getName()});
        } else if(child->getType() == APPLICATION_TYPE && !child->isDecorator()) {
            s.fields.push_back({"EO_object", child->getName()});
        }
    }
    codeModel.addStruct(s);
}

void Generator::genInit(Object* obj) {
    Function f;
    std::vector<std::string> signatureArgs = {
        obj->getFullName() + "* obj",
        "int parent_offset"
    };
    if(!getValueType(obj).empty()) {
        signatureArgs.push_back(getValueType(obj) + " value");
    }
    f.signature = genCall("void init_" + obj->getFullName(), signatureArgs);
    std::vector<Attribute> attributes = obj->getAttributes();
    int varargs = -1;
    if(!attributes.empty() && attributes.back().isVararg) {
        varargs = attributes.size() - 1;
    }
    std::string offset_var = "&offset_" + obj->getFullName();
    if(attributes.empty() && obj->getChildren().empty()) {
        offset_var = "&offset_default";
    }
    f.addLine(genCall("init_head", {
        "(EO_object*)obj",
        "eval_" + obj->getFullName(),
        "parent_offset",
        std::to_string(varargs),
        genCall("sizeof", {obj->getFullName()}),
        offset_var,
    }));
    for(Object* child : obj->getChildren()) {
        if(child->getType() == CLASS_TYPE) {
            f.addLine(genCall("init_" + child->getFullName(), {
                "&obj->" + child->getName(),
                genCall("offsetof", {obj->getFullName(), child->getName()})
            }));
        } else if(child->getType() == APPLICATION_TYPE && !child->isDecorator()) {
            ApplicationGenerator appGen(child, codeModel, genEvalSignature(child, "obj_"));
            appGen.getFunction().addLine("EO_object* obj = " + genCall("get_parent", {"obj_"}));
            appGen.getFunction().addLine("StackPos pos = " + genCall("stack_store"));
            appGen.run();
            appGen.getFunction().addLine(genCall("stack_restore", {"pos"}));
            appGen.getFunction().addLine("return " + genCall("clone", {appGen.getResultVar()}));
            codeModel.addFunction(appGen.getFunction());
            f.addLine(genCall("init_head", {
                "&obj->" + child->getName(),
                appGen.getFunction().getName(),
                genCall("offsetof", {obj->getFullName(), child->getName()}),
                "-1",
                "sizeof(EO_object)",
                "&offset_default"
            }));
        }
    }
    for(Attribute a : obj->getAttributes()) {
        f.addLine("obj->" + a.name + " = 0");
    }
    if(!getValueType(obj).empty()) {
        f.addLine("obj->value = value");
    }
    if(obj->getFullName() == "bytes") {
        f.addLine("obj->head.size += obj->value.length");
    }
    codeModel.addFunction(f);
}

void Generator::genEval(Object* obj) {
    if(obj->isAtom()) {
        codeModel.addFunctionAtom("templates/" + getPackage() + "/" + obj->getFullName() + ".cpp");
        return;
    }
    std::vector<Object*> children = obj->getChildren();
    auto it = std::find_if(children.begin(), children.end(), [](Object* o){ return o->isDecorator(); });
    Function f;
    if(it == children.end()) {
        f.signature = genEvalSignature(obj, "obj");
        f.addLine("return obj");
    } else {
        ApplicationGenerator appGen(*it, codeModel, genEvalSignature(obj, "obj"));
        appGen.getFunction().addLine("StackPos pos = " + genCall("stack_store"));
        appGen.run();
        f = appGen.getFunction();
        f.addLine(genCall("stack_restore", {"pos"}));
        f.addLine("return " + genCall("clone", {appGen.getResultVar()}));
    }
    codeModel.addFunction(f);
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
    } else if(obj->getName() == "EO_array") {
        return "int";
    }
    return "";
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

std::string Generator::genEvalSignature(Object* obj, std::string param) {
    return genCall("EO_object* eval_" + obj->getFullName(), {"EO_object* " + param});
}
