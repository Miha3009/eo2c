#include "application_generator.h"
#include <iostream>

void ApplicationGenerator::print(Object* obj, int s) {
    std::cout << std::string(s, ' ');
    if(obj->getType()) std::cout << "type=" << objectTypeNames[obj->getType()] << " ";
    if(variableNames.count(obj)) std::cout << variableNames[obj];
    std::cout << "\n";
    for(Object* child : obj->getChildren()) {
        print(child, s+2);
    }
}

ApplicationGenerator::ApplicationGenerator(Object* root, CodeModel& codeModel, std::string signature): root{root}, codeModel{codeModel}, innerFunctionsCount{0} {
    f.signature = signature;
}

Function& ApplicationGenerator::getFunction() {
    return f;
}

std::string ApplicationGenerator::getResultVar() {
    return resultVar;
}

void ApplicationGenerator::run() {
    genApplication(root);
    clearEvaluate();
    resultVar = variableNames[root];
}

void ApplicationGenerator::genApplication(Object* obj) {
    Object* head = obj->getApplicationHead();
    std::vector<Object*> attributes = obj->getApplicationAttributes();
    for(Object* a : attributes) {
        if(a->getType() == SEQUENCE_TYPE) {
            genSequenceHead(a, false);
        }
    }
    genPart(head, "obj", true, false);
    variableNames[obj] = variableNames[head];
    if(!obj->getApplicationAttributes().empty()) {
        std::string headVar = variableNames[head];
        varsWithAttributes.insert(headVar);
        genAttributesLength(obj);
        for(Object* a : obj->getApplicationAttributes()) {
            genPart(a, "obj", false, false);
            if(head->getType() == ARRAY_TYPE) {
                f.addLine(genCall("add_attribute_to_array", {headVar, f.getVar()}));
            } else if(a->isVararg()) {
                f.addLine(genCall("add_array_attribute", {headVar, f.getVar()}));
            } else {
                f.addLine(genCall("add_attribute", {headVar, f.getVar(), "L_" + headVar}));
            }
        }
        f.addLine(genCall("update_size", {headVar}));
        f.addLine(headVar + " = " + genCall("evaluate", {headVar}));
    }
}

void ApplicationGenerator::genPart(Object* obj, std::string parentVar, bool isHead, bool isTemp) {
    if(obj->getType() == APPLICATION_TYPE) {
        if(isHead) {
            genApplication(obj);
            return;
        } else {
            genInnerApplication(obj);
        }
    } else if(obj->getType() == SEQUENCE_TYPE) {
        if(isHead) {
            genSequenceHead(obj, isHead);
        }
        genSequenceTail(obj, isHead);
    } else if(obj->getType() == VAR_TYPE) {
        genVar(obj, parentVar, isTemp);
    } else if(obj->getType() == REF_TYPE) {
        genRef(obj, parentVar, isTemp);
    } else {
        genData(obj);
    }
    variableNames[obj] = f.getVar();
}

void ApplicationGenerator::genSequenceHead(Object* obj, bool isHead) {
    std::vector<Object*> children = obj->getChildren();
    std::string parentVar = "obj";
    for(int i = 0; i + 1 < children.size(); ++i) {
        genPart(children[i], parentVar, isHead, true);
        parentVar = variableNames[children[i]];
    }
    Object* tail = children[children.size()-1];
    if(tail->getType() == VAR_TYPE) {
        f.addLine(parentVar + " = " + genCall("ensure_sub", {parentVar, codeModel.getIdTagTable().getTag(tail->getValue())}));
    }
}

void ApplicationGenerator::genSequenceTail(Object* obj, bool isHead) {
    std::vector<Object*> children = obj->getChildren();
    std::string parentVar = variableNames[children[children.size()-2]];
    genPart(children[children.size()-1], parentVar, isHead, false);
}

void ApplicationGenerator::genInnerApplication(Object* obj) {
    std::string signature = genCall("EO_object* " + f.getName() + "_" + std::to_string(++innerFunctionsCount), {"EO_object* obj_"});
    ApplicationGenerator appGen(obj, codeModel, signature);
    appGen.run();
    Function f2 = appGen.getFunction();
    f2.setType(INNER, appGen.getResultVar());
    codeModel.addFunction(f2);
    f.addLine(f.nextVarDeclaration() + genCall("make_meta_object", {"obj", f2.getName()}));
}

void ApplicationGenerator::genVar(Object* obj, std::string parentVar, bool isTemp) {
    if(obj->getParent()->getType() != SEQUENCE_TYPE || obj == obj->getParent()->getChildren()[0]) {
        Object* classObj = obj->getClassObject();
        std::string classType = classObj->getClassName();
        if(obj->getValue() == classObj->getValue()) {
            f.addLine(f.nextVarDeclaration() + genCall("stack_alloc", {genCall("sizeof", {classType})}));
            f.addLine(genCall("init_" + classType, {"(" + classType + "*)" + f.getVar(), "(StackPos)" + f.getVar() + "-(StackPos)obj"}));
            return;
        }
        std::string className = codeModel.getClassNameByValue(obj->getOriginValue());
        if(className != "") {
            f.addLine(f.nextVarDeclaration() + genCall("stack_alloc", {genCall("sizeof", {className})}));
            f.addLine(genCall("init_" + className, {"(" + className + "*)" + f.getVar(), "0"}));
            return;
        }
    }
    /*bool isAttribute = false;
    for(Attribute& a : classObj->getAttributes()) {
        if(a.name == obj->getValue()) {
            isAttribute = true;
            break;
        }
    }
    std::string tmp;
    if(!isAttribute) {
        tmp = genCall("(EO_object*)&", {"(" + classType + "*)obj"}) + "->" + obj->getValue();
    } else {
        tmp = genCall("apply_offset", {"obj", "((" + classType + "*)obj)" + "->" + obj->getValue()});
    }*/
    std::string copyBool = isTemp ? "false" : "true";
    f.addLine(f.nextVarDeclaration() + genCall("get_sub", {parentVar, codeModel.getIdTagTable().getTag(obj->getValue()), copyBool}));
}

void ApplicationGenerator::genRef(Object* obj, std::string parentVar, bool isTemp) {
    if(obj->getValue() == "^") {
        if(isTemp) {
            f.addLine(f.nextVarDeclaration() + genCall("get_parent", {parentVar}));
        } else {
            f.addLine(f.nextVarDeclaration() + genCall("clone", {genCall("get_parent", {parentVar})}));
        }
    }
}

void ApplicationGenerator::genData(Object* obj) {
    std::string type;
    std::string value = obj->getValue();
    Call c("make_bytes");
    switch(obj->getType()) {
    case INT_TYPE:
        type = "EO_int";
        value += "LL";
        codeModel.addImport("org.eolang.int");
        break;
    case FLOAT_TYPE:
        type = "EO_float";
        codeModel.addImport("org.eolang.float");
        break;
    case BOOL_TYPE:
        type = "EO_bool";
        codeModel.addImport("org.eolang.bool");
        break;
    case STRING_TYPE:
        type = "EO_string";
        value = genCall("make_string", {obj->getValue()});
        codeModel.addImport("org.eolang.string");
        break;
    case BYTES_TYPE:
        type = "bytes";
        for(Object* child : obj->getChildren()) {
            if(child->getType() == BYTE_TYPE) {
                c.addArgument("(unsigned char)0x" + child->getValue());
            }
        }
        value = genCall(c);
        codeModel.addImport("org.eolang.bytes");
        break;
    case ARRAY_TYPE:
        f.addLine(f.nextVarDeclaration() + genCall("make_array", {std::to_string(obj->getParent()->getApplicationAttributes().size())}));
        codeModel.addImport("org.eolang.array");
        return;
    default:
        return;
    }
    f.addLine(f.nextVarDeclaration() + genCall("stack_alloc", {genCall("sizeof", {type})}));
    if(!value.empty()) {
        f.addLine(genCall("init_" + type, {"(" + type + "*)" + f.getVar(), "0", value}));
    } else {
        f.addLine(genCall("init_" + type, {"(" + type + "*)" + f.getVar(), "0"}));
    }
}

void ApplicationGenerator::genAttributesLength(Object* obj) {
    if(obj->getApplicationHead()->getType() == ARRAY_TYPE) {
        return;
    }
    f.addLine("int L_" + f.getVar() + " = " + std::to_string(obj->getApplicationAttributes().size()));
}

void ApplicationGenerator::clearEvaluate() {
    for(int i = 0; i < f.body.size(); ++i) {
        if(f.body[i].find("EO_object* ") == 0) {
            std::string tmp = f.body[i].substr(11);
            std::string varName = tmp.substr(0, tmp.find(" "));
            if(varsWithAttributes.count(varName)) {
                int evalBegin = f.body[i].find("evaluate");
                if(evalBegin != -1) {
                    f.body[i] = f.body[i].substr(0, evalBegin) + f.body[i].substr(evalBegin + 9);
                    f.body[i] = f.body[i].substr(0, f.body[i].size() - 1);
                }
            }
        }
    }
}
