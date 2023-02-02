#include <iostream>
#include <algorithm>
#include "object.h"

Object::Object(Object* parent, objectType type) {
    this->type = type;
    this->parent = parent;
    this->atom = false;
    this->clone = false;
}

Object::~Object() {
    for(Object* child : children) {
        delete child;
    }
    if(parent) {
        auto it = std::find(parent->children.begin(), parent->children.end(), this);
        if(it != parent->children.end()) {
            parent->children.erase(it);
        }
    }
}

Object* Object::makeChild() {
    return makeChild(CLASS_TYPE);
}

Object* Object::makeChild(objectType type) {
    Object* child = new Object(this, type);
    children.push_back(child);
    return child;
}

void Object::clearChildren() {
    children.clear();
}

void Object::setName(std::string name) {
    this->name = convertName(name);
}

void Object::setValue(std::string value) {
    int l = value.length();
    if(type == VAR_TYPE) {
        if(l > 0 && value[l-1] == '\'') {
            this->clone = true;
            value = value.substr(0, l-1);
        }
    }
    this->value = value;
}

void Object::setType(objectType type) {
    this->type = type;
}

void Object::setChildren(std::vector<Object*> children) {
    this->children = children;
    for(Object* child : children) {
        child->setParent(this);
    }
}

void Object::setParent(Object* parent) {
    this->parent = parent;
}

void Object::setAtom() {
    this->atom = true;
}

void Object::addAttribute(std::string name) {
    bool isVararg = false;
    int l = name.length();
    if(l > 2 && name[l-1] == '.' && name[l-2] == '.' && name[l-3] == '.') {
        name = name.substr(0, l-3);
        isVararg = true;
    }
    attributes.push_back({convertName(name), isVararg});
}

void Object::updateInverseNotaion() {
    if(children.size() >= 2 && (children[0]->type == VAR_TYPE || children[0]->type == REF_TYPE) && endsWithDot(children[0]->getValue())) {
        children[0]->type = METHOD_TYPE;
        children[1]->children.push_back(children[0]);
        children.erase(children.begin());
    }
}

bool Object::validate() {
    std::unordered_set<std::string> context;
    return validate(context);
}

bool Object::isRoot() {
    return parent == nullptr;
}

bool Object::isAtom() {
    return atom;
}

bool Object::isClone() {
    return clone;
}

bool Object::isDecorator() {
    return type == APPLICATION_TYPE && name == "@";
}

std::string Object::getName() {
    return name;
}

std::string Object::getFullName() {
    if(!fullName.empty()) return fullName;
    if(name.empty()) return "";
    fullName = name;
    Object* obj = parent;
    while(obj != nullptr) {
        if(!obj->name.empty() && obj->type == CLASS_TYPE) {
            fullName = obj->name + "_" + fullName;
        }
        obj = obj->parent;
    }
    return fullName;
}

std::string Object::getValue() {
    return value;
}

objectType Object::getType() {
    return type;
}

std::vector<Object*> Object::getChildren() {
    return children;
}

std::vector<Attribute> Object::getAttributes() {
    return attributes;
}

Object* Object::getParent() {
    return parent;
}

Object* Object::getClassObject() {
    if(type == CLASS_TYPE) {
        return this;
    } else {
        return parent->getClassObject();
    }
}

Object* Object::getApplicationHead() {
    return children[0];
}

std::vector<Object*> Object::getApplicationAttributes() {
    std::vector<Object*> result;
    for(int i = 1; i < children.size(); ++i) {
        if(children[i]->type == METHOD_TYPE) {
            break;
        }
        result.push_back(children[i]);
    }
    return result;
}

Object* Object::getApplicationMethod() {
    if(children.size() > 0 && children.back()->getType() == METHOD_TYPE) {
        return children[children.size()-1];
    }
    return nullptr;
}

bool Object::validate(std::unordered_set<std::string>& context) {
    if(type == CLASS_TYPE) {
        auto check = checkDuplicateNames();
        if(!check.first) {
            return false;
        }
        for(int i = 0; i + 1 < attributes.size(); ++i) {
            if(attributes[i].isVararg) {
                std::cout << "error: varargs attribute must be the last";
                return false;
            }
        }
        for(Object* child : children) {
            if(!child->validate(check.second)) return false;
        }
    } else {
        for(Object* child : children) {
            if(!child->validate(context)) return false;
        }
    }
    return true;
}

std::pair<bool, std::unordered_set<std::string>> Object::checkDuplicateNames() {
    std::unordered_set<std::string> names;
    for(Attribute& a : attributes) {
        if(names.count(a.name)) {
            std::cout << "error: attribute with name " << a.name << " already defined\n";
            return make_pair(false, names);
        }
        names.insert(a.name);
    }
    for(Object* child : children) {
        if(child->name != "") {
            if(names.count(child->name)) {
                std::cout << "error: object with name " << child->name << " already defined\n";
                return make_pair(false, names);
            }
            names.insert(child->name);
        }
    }
    return make_pair(true, names);
}

void Object::printDebug() {
    printDebug(0);
}

void Object::printDebug(int s) {
    std::cout << std::string(s, ' ');
    if(!name.empty()) std::cout << "name=" << name << " ";
    if(type) std::cout << "type=" << objectTypeNames[type] << " ";
    if(!value.empty()) std::cout << "value=" << value << " ";
    for(int i = 0; i < attributes.size(); ++i) {
        if(i == 0) std::cout << "[" << attributes[i].name;
        else std::cout << " " << attributes[i].name;
        if(attributes[i].isVararg) std::cout << "...";
        if(i == attributes.size() - 1) std::cout << "]";
    }
    std::cout << "\n";
    for(Object* child : children) {
        child->printDebug(s+2);
    }
}

Object* optimize(Object* obj) {
    std::vector<Object*> children = obj->getChildren();
    for(int i = 0; i < children.size(); ++i) {
        children[i] = optimize(children[i]);
    }
    obj->setChildren(children);
    if(endsWithDot(obj->getValue())) {
        obj->setValue(obj->getValue().substr(0, obj->getValue().length()-1));
    }
    if(!obj->isRoot() && (obj->getParent()->getType() == VAR_TYPE || obj->getParent()->getType() == REF_TYPE)) {
        obj->setType(METHOD_TYPE);
    }
    if(obj->getType() == VAR_TYPE || obj->getType() == METHOD_TYPE) {
        obj->setValue(convertName(obj->getValue()));
    }
    if(children.size() == 1) {
        if(obj->isRoot()) {
            Object* tmpObj = children[0];
            tmpObj->setParent(nullptr);
            obj->clearChildren();
            delete obj;
            return tmpObj;
        }
        if(obj->getType() == APPLICATION_TYPE) {
            if(children[0]->getType() == APPLICATION_TYPE) {
                Object* tmpObj = children[0];
                obj->setChildren(tmpObj->getChildren());
                tmpObj->clearChildren();
                delete tmpObj;
            } else if(obj->getParent()->getType() == APPLICATION_TYPE){
                Object* tmpObj = children[0];
                obj->clearChildren();
                delete obj;
                return tmpObj;
            }
        }
    }
    return obj;
}

std::string convertName(std::string name) {
    static std::vector<std::string> keywords = {"if", "while", "and", "or", "xor", "not", "bool", "int", "float", "string", "char"};
    if(std::find(keywords.begin(), keywords.end(), name) != keywords.end()) {
        return "EO_" + name;
    }
    for(int i = 0; i < name.length(); ++i) {
        if((name[i] < 'a' || name[i] > 'z') &&
                (name[i] < 'A' || name[i] > 'Z') &&
                (name[i] < '0' || name[i] > '9') &&
                name[i] !='@') {
            name[i] = '_';
        }
    }
    return name;
}

bool endsWithDot(std::string str) {
    return str.length() > 0 && str[str.length()-1] == '.';
}

void updateNameTagTable(Object* obj, std::unordered_map<std::string, int>& nameTagTable) {
    if(obj->getType() == VAR_TYPE || obj->getType() == METHOD_TYPE) {
        if(!nameTagTable.count(obj->getValue())) {
            std::cout << obj->getValue() << " : " << nameTagTable.size() << "\n";
            nameTagTable[obj->getValue()] = nameTagTable.size();
        }
    } else if(obj->getType() == CLASS_TYPE) {
        if(!nameTagTable.count(obj->getName())) {
            std::cout << obj->getName() << " : " << nameTagTable.size() << "\n";
            nameTagTable[obj->getName()] = nameTagTable.size();
        }
    }
    for(Object* child : obj->getChildren()) {
        updateNameTagTable(child, nameTagTable);
    }
}
