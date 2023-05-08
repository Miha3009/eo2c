#include "application_generator.h"
#include <iostream>

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
    if(root->getType() == APPLICATION_TYPE) {
        genApplication(root);
    } else {
        genSequence(root);
    }
    resultVar = variableNames[root];
    f.addLine(resultVar + " = " + genCall("evaluate", {resultVar}));
}

void ApplicationGenerator::genApplication(Object* obj) {
    Object* head = obj->getApplicationHead();
    genPart(head, "obj", true, false);
    variableNames[obj] = variableNames[head];
    std::string headVar = variableNames[head];
    std::vector<Object*> attributes = obj->getApplicationAttributes();
    if(!attributes.empty()) {
        varsWithAttributes.insert(headVar);
        f.addLine(genCall("add_attribute_start", {headVar, std::to_string(attributes.size())}));
        for(Object* a : attributes) {
            genPart(a, "obj", false, false);
            if(head->getType() == ARRAY_TYPE) {
                f.addLine(genCall("add_attribute_to_array", {headVar, f.getVar()}));
            } else if(a->hasFlags(VARARGS_FLAG)) {
                f.addLine(genCall("add_array_attribute", {headVar, f.getVar()}));
            } else if(a->getType() == NAMED_ATTRIBUTE_TYPE) {
                f.addLine(genCall("add_attribute_by_tag", {headVar, f.getVar(), codeModel.getIdTagTable().getTag(a->getValue())}));
            } else {
                f.addLine(genCall("add_attribute", {headVar, f.getVar(), std::to_string(attributes.size())}));
            }
        }
        f.addLine(genCall("add_attribute_end", {headVar}));
    }
}

void ApplicationGenerator::genSequence(Object* obj) {
    std::vector<Object*> children = obj->getChildren();
    std::string parentVar = "obj";
    for(int i = 0; i < children.size(); ++i) {
        genPart(children[i], parentVar, true, i + 1 != children.size());
        parentVar = variableNames[children[i]];
    }
    variableNames[obj] = parentVar;
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
            genSequence(obj);
        } else {
            genInnerApplication(obj);
        }
    } else if(obj->getType() == VAR_TYPE) {
        genVar(obj, parentVar, isTemp);
    } else if(obj->getType() == REF_TYPE) {
        genRef(obj, parentVar, isTemp);
    } else if(obj->getType() == CLASS_TYPE) {
        genClass(obj);
    } else if(obj->getType() == NAMED_ATTRIBUTE_TYPE) {
        genPart(obj->getChildren()[0], parentVar, isHead, isTemp);
    } else {
        genData(obj);
    }
    variableNames[obj] = f.getVar();
}

void ApplicationGenerator::genInnerApplication(Object* obj) {
    std::string signature = genCall("EO_object* " + f.getName() + "_" + std::to_string(++innerFunctionsCount), {"EO_object* obj_"});
    ApplicationGenerator appGen(obj, codeModel, signature);
    appGen.run();
    Function f2 = appGen.getFunction();
    f2.setType(INNER, appGen.getResultVar());
    codeModel.addFunction(f2);
    if(obj->hasFlags(CONSTANT_FLAG)) {
        f.addLine(f.nextVarDeclaration() + genCall("run_inner_application", {"obj", f2.getName()}));
    } else {
        f.addLine(f.nextVarDeclaration() + genCall("make_inner_application", {"obj", f2.getName()}));        
    }
}

void ApplicationGenerator::genVar(Object* obj, std::string parentVar, bool isTemp) {
    if(parentVar == "obj") {
        if(findInClass(obj, parentVar, isTemp) || findVarInAlias(obj)) return;
    }
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
    } else if(obj->getValue() == "$") {
        f.addLine(f.nextVarDeclaration() + "obj");
    } else if(obj->getValue() == "&") {
        if(isTemp) {
            f.addLine(f.nextVarDeclaration() + genCall("get_home", {parentVar}));
        } else {
            f.addLine(f.nextVarDeclaration() + genCall("clone", {genCall("get_home", {parentVar})}));
        }
    } else if(obj->getValue() == "<") {
        f.addLine(f.nextVarDeclaration() + genCall("get_id", {parentVar}));
    }
}

void ApplicationGenerator::genData(Object* obj) {
    std::string type;
    std::string value = obj->getValue();
    Call c("make_bytes");
    int array_length;
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
        value = genCall("make_string", {"L" + obj->getValue()});
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
        array_length = obj->getParent()->getApplicationAttributes().size();
        if(array_length > 0 && obj != obj->getParent()->getApplicationHead()) {
            array_length = 0;
        }
        f.addLine(f.nextVarDeclaration() + genCall("make_array", {std::to_string(array_length)}));
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

void ApplicationGenerator::genClass(Object* obj) {
    std::string type = obj->getClassName();
    f.addLine(f.nextVarDeclaration() + genCall("stack_alloc", {genCall("sizeof", {type})}));
    f.addLine(genCall("init_" + type, {"(" + type + "*)" + f.getVar(), "(StackPos)" + f.getVar() + "-(StackPos)obj"}));
}

bool ApplicationGenerator::findInClass(Object* obj, std::string parentVar, bool isTemp) {
    int depth = 0;
    Object* classObj = obj;
    while(classObj != nullptr) {
        classObj = classObj->getClassObject();
        for(Attribute attr : classObj->getAttributes()) {
            if(attr.name == obj->getValue()) {
                for(int i = 0; i < depth; ++i) {
                    f.addLine(f.nextVarDeclaration() + genCall("get_parent", {parentVar}));
                    parentVar = f.getVar();
                }
                std::string copyBool = isTemp ? "false" : "true";
                f.addLine(f.nextVarDeclaration() + genCall("get_sub", {parentVar, codeModel.getIdTagTable().getTag(obj->getValue()), copyBool}));
                return true;
            }
        }
        for(Object* child : classObj->getChildren()) {
            if(obj->getOriginValue() == child->getOriginValue()) {
                std::string classType = child->getClassName();
                f.addLine(f.nextVarDeclaration() + genCall("stack_alloc", {genCall("sizeof", {classType})}));
                f.addLine(genCall("init_" + classType, {"(" + classType + "*)" + f.getVar(), "(StackPos)" + f.getVar() + "-(StackPos)obj"}));
                return true;
            }
        }
        classObj = classObj->getParent();
        ++depth;
    }
    return false;
}

bool ApplicationGenerator::findVarInAlias(Object* obj) {
    std::string className = codeModel.getClassNameByValue(obj->getOriginValue());
    if(className != "") {
        f.addLine(f.nextVarDeclaration() + genCall("stack_alloc", {genCall("sizeof", {className})}));
        f.addLine(genCall("init_" + className, {"(" + className + "*)" + f.getVar(), "0"}));
        return true;
    }
    return false;
}
