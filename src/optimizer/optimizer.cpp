#include <iostream>
#include <algorithm>
#include "optimizer.h"

#define DEBUG

Optimizer::Optimizer(std::vector<Object*> objects_): objects(objects_) {
}

bool Optimizer::run() {
    if(std::any_of(objects.begin(), objects.end(), [](Object* obj){ return !obj->validate(); })) {
        return false;
    }
    for(int i = 0; i < objects.size(); ++i) {
        objects[i] = removeRootWithOneChild(objects[i]);
        applyInverseNotation(objects[i]);
        objects[i] = removeReduntObjects(objects[i]);
        convertIdentifier(objects[i]);
        updateNameTagTable(objects[i]);
#ifdef DEBUG
        objects[i]->printDebug();
#endif // DEBUG
    }
    return true;
}

std::vector<Object*> Optimizer::getObjects() {
    return objects;
}

std::unordered_map<std::string, int>& Optimizer::getNameTagTable() {
    return nameTagTable;
}

Object* Optimizer::removeRootWithOneChild(Object* obj) {
    if(obj->getChildren().size() == 1) {
        Object* tmpObj = obj->getChildren()[0];
        tmpObj->setParent(nullptr);
        obj->clearChildren();
        delete obj;
        return tmpObj;
    }
    return obj;
}

void Optimizer::applyInverseNotation(Object* obj) {
    std::vector<Object*> children = obj->getChildren();
    for(int i = 0; i < children.size(); ++i) {
        applyInverseNotation(children[i]);
    }
    if(obj->getType() == APPLICATION_TYPE &&
       (children[0]->getType() == VAR_TYPE || children[0]->getType() == REF_TYPE) &&
       endsWithDot(children[0]->getValue())) {
        Object* tmpObj = children[0];
        children[0] = new Object(obj, SEQUENCE_TYPE);
        children[0]->addChild(children[1]);
        children[0]->addChild(tmpObj);
        children.erase(children.begin() + 1);
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
    if(endsWithDot(obj->getValue())) {
        std::string value = obj->getValue();
        obj->setValue(value.substr(0, value.length()-1));
    }
    if(obj->getType() == VAR_TYPE) {
        obj->setValue(convertIdentifier(obj->getValue()));
    }
    if(!obj->getName().empty() && obj->getName() != "@") {
        obj->setName(convertIdentifier(obj->getName()));
    }
    std::vector<Attribute> attributes = obj->getAttributes();
    for(int i = 0; i < attributes.size(); ++i) {
        attributes[i].name = convertIdentifier(attributes[i].name);
    }
    obj->setAttributes(attributes);
}

std::string Optimizer::convertIdentifier(std::string id) {
    static std::vector<std::string> keywords = {"if", "while", "and", "or", "xor", "not", "bool", "int", "float", "string", "char", "array"};
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

void Optimizer::updateNameTagTable(Object* obj) {
    if(obj->getType() == VAR_TYPE) {
        if(!obj->getValue().empty() && !nameTagTable.count(obj->getValue())) {
            std::cout << obj->getValue() << " : " << nameTagTable.size() << "\n";
            nameTagTable[obj->getValue()] = nameTagTable.size();
        }
    } else if(obj->getType() == CLASS_TYPE || (obj->getType() == APPLICATION_TYPE && !obj->isDecorator())) {
        if(!obj->getName().empty() && !nameTagTable.count(obj->getName())) {
            std::cout << obj->getName() << " : " << nameTagTable.size() << "\n";
            nameTagTable[obj->getName()] = nameTagTable.size();
        }
    }
    for(Object* child : obj->getChildren()) {
        updateNameTagTable(child);
    }
}

char Optimizer::toHex(int x) {
    if(x < 10) {
        return '0' + x;
    } else {
        return 'A' + (x-10);
    }
}

bool Optimizer::endsWithDot(std::string str) {
    return str.length() > 0 && str[str.length()-1] == '.';
}
