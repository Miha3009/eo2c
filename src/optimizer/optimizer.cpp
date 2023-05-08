#include <iostream>
#include <algorithm>
#include "optimizer.h"

//#define DEBUG

Optimizer::Optimizer(TranslationUnit& unit, IdTagTable& idTagTable, ImportsMap& importsMap): unit{unit}, idTagTable{idTagTable}, importsMap{importsMap} {
}

bool Optimizer::run() {
    applyInverseNotation(unit.root);
    expandQQ(unit.root, unit);
    unit.root = removeReduntObjects(unit.root);
    convertIdentifier(unit.root);
    updateIdTagTable(unit.root);
#ifdef DEBUG
    unit.root->printDebug();
#endif // DEBUG
    return true;
}

void Optimizer::applyInverseNotation(Object* obj) {
    std::vector<Object*> children = obj->getChildren();
    for(int i = 0; i < children.size(); ++i) {
        applyInverseNotation(children[i]);
    }
    if(obj->getType() == APPLICATION_TYPE && (children[0]->getType() == VAR_TYPE || children[0]->getType() == REF_TYPE) && children[0]->hasFlags(DOT_FLAG)) {
        Object* tmpObj = children[0];
        children[0] = new Object(obj, SEQUENCE_TYPE);
        children[0]->addChild(children[1]);
        children[0]->addChild(tmpObj);
        children.erase(children.begin() + 1);
    }
    obj->setChildren(children);
}

void Optimizer::expandQQ(Object* obj, TranslationUnit& unit) {
    for(Object* child : obj->getChildren()) {
        expandQQ(child, unit);
    }
    std::vector<Object*> children = obj->getChildren();
    if(obj->getType() == SEQUENCE_TYPE && children[0]->getType() == REF_TYPE && children[0]->getOriginValue() == "QQ") {
        std::string package = "org.eolang";
        std::string resultPackage = package;
        int resultI = 1;
        for(int i = 1; i < children.size(); ++i) {
            package = package + "." + children[i]->getOriginValue();
            if(importsMap.getUnit(package) != nullptr) {
                resultPackage = package;
                resultI = i;
            }
        }
        children.erase(children.begin(), children.begin() + resultI);
        unit.metas.push_back({"alias", resultPackage});
    }
    obj->setChildren(children);
}

Object* Optimizer::removeReduntObjects(Object* obj) {
    std::vector<Object*> children = obj->getChildren();
    for(int i = 0; i < children.size(); ++i) {
        children[i] = removeReduntObjects(children[i]);
    }
    obj->setChildren(children);
    if(obj->getType() == APPLICATION_TYPE && children.size() == 1 && obj->getParent()->getType() != CLASS_TYPE) {
        Object* tmpObj = children[0];
        obj->clearChildren();
        delete obj;
        return tmpObj;
    }
    if(obj->getType() == SEQUENCE_TYPE && children[0]->getType() == SEQUENCE_TYPE) {
        obj->setChildren(children[0]->getChildren());
        for(int i = 1; i < children.size(); ++i) {
            obj->addChild(children[i]);
        }
        children[0]->clearChildren();
        delete children[0];
    }
    return obj;
}

void Optimizer::convertIdentifier(Object* obj) {
    for(Object* child : obj->getChildren()) {
        convertIdentifier(child);
    }
    if(obj->getType() == CLASS_TYPE || obj->getType() == VAR_TYPE || obj->getType() == APPLICATION_TYPE) {
        obj->setValue(convertIdentifier(obj->getOriginValue()));
    }
    std::vector<Attribute> attributes = obj->getAttributes();
    for(int i = 0; i < attributes.size(); ++i) {
        attributes[i].name = convertIdentifier(attributes[i].name);
    }
    obj->setAttributes(attributes);
}

std::string Optimizer::convertIdentifier(std::string id) {
    static std::vector<std::string> keywords = {"if", "while", "and", "or", "xor", "not", "bool", "int", "float", "string", "char", "array", "stdin", "stdout", "sprintf", "sscanf", "nan"};
    if(std::find(keywords.begin(), keywords.end(), id) != keywords.end()) {
        return "EO_" + id;
    }
    std::string newId;
    for(int i = 0; i < id.length(); ++i) {
        if(id[i] == '-') {
            newId.push_back('_');
        } else if((id[i] >= 'a' && id[i] <= 'z') ||
                (id[i] >= 'A' && id[i] <= 'Z') ||
                (id[i] >= '0' && id[i] <= '9')) {
            newId.push_back(id[i]);
        } else {
            newId.push_back(toHex(id[i]%16));
            newId.push_back(toHex(id[i]/16));
        }
    }
    return newId;
}

void Optimizer::updateIdTagTable(Object* obj) {
    if(obj->getType() == VAR_TYPE || obj->getType() == CLASS_TYPE || (obj->getType() == APPLICATION_TYPE && !obj->isDecorator()) || obj->getType() == NAMED_ATTRIBUTE_TYPE) {
        idTagTable.update(obj->getValue());
    }
    for(Attribute& a : obj->getAttributes()) {
        idTagTable.update(a.name);
    }
    for(Object* child : obj->getChildren()) {
        updateIdTagTable(child);
    }
}

char Optimizer::toHex(int x) {
    if(x < 10) {
        return '0' + x;
    } else {
        return 'A' + (x-10);
    }
}
