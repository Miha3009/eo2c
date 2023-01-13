#include <iostream>
#include <algorithm>
#include "object.h"

Object::Object(Object* parent, objectType type) {
    this->type = type;
    this->parent = parent;
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

Object* Object::makeChild(objectType type) {
    Object* child = new Object(this, type);
    children.push_back(child);
    return child;
}

void Object::setName(std::string name) {
    this->name = name;
}

void Object::setValue(std::string value) {
    this->value = value;
}

void Object::setType(objectType type) {
    this->type = type;
}

void Object::addAttribute(std::string&& name) {
    attributes.push_back(name);
}

Object* Object::swapWithParent() {
    Object* p = parent;
    parent = p->parent;
    children.insert(children.begin(), p);
    parent->children.push_back(this);
    p->parent = this;
    for(int i = 0; i < parent->children.size(); ++i) {
        if(parent->children[i] == p) {
            parent->children.erase(parent->children.begin() + i);
            break;
        }
    }
    for(int i = 0; i < p->children.size(); ++i) {
        if(p->children[i] == this) {
            p->children.erase(p->children.begin() + i);
            break;
        }
    }
    return p;
}

bool Object::validate() {
    std::vector<std::string> names(attributes);
    for(Object* child : children) {
        if(child->name != "") {
            names.push_back(child->name);
        }
    }
    std::sort(std::begin(names), std::end(names));
    for(int i = 1; i < names.size(); ++i) {
        if(names[i] == names[i-1]) {
            std::cout << "error: object with name " << names[i] << " already defined\n";
            return false;
        }
    }
    for(Object* child : children) {
        if(!child->validate()) return false;
    }
    return true;
}

bool Object::isRoot() {
    return parent == nullptr;
}

std::string& Object::getName() {
    return name;
}

std::string Object::getFullName() {
    if(name.empty()) return "";

    std::string result = name;
    Object* obj = parent;
    while(obj != nullptr) {
        if(!obj->name.empty()) {
            result = obj->name + "_" + result;
        }
        obj = obj->parent;
    }
    return result;
}

std::string& Object::getValue() {
    return value;
}

objectType Object::getType() {
    return type;
}

std::vector<Object*> Object::getChildren() {
    return children;
}

std::vector<std::string> Object::getAttributes() {
    return attributes;
}

Object* Object::getParent() {
    return parent;
}

Object* Object::getRefObject() {
    if(type != REF_TYPE) return nullptr;
    switch(value[0]) {
    case '$':
        return scopeObject(this);
    case '^':
        return scopeObject(scopeObject(this));
    }
    return nullptr;
}

void Object::printDebug() {
    printDebug(0);
}

void Object::printDebug(int s) {
    std::cout << std::string(s, ' ');
    if(!name.empty()) std::cout << "name=" << name << " ";
    if(type) std::cout << "type=" << objectTypeNames[type] << " ";
    if(!value.empty()) std::cout << "value=" << value << " ";
    if(type == REF_TYPE) std::cout << getRefObject()->name << " ";
    for(int i = 0; i < attributes.size(); ++i) {
        if(i == 0) std::cout << "[" << attributes[i];
        else std::cout << " " << attributes[i];
        if(i == attributes.size() - 1) std::cout << "]";
    }
    std::cout << "\n";
    for(Object* child : children) {
        child->printDebug(s+2);
    }
}

Object* Object::scopeObject(Object* obj) {
    if(!obj) return nullptr;
    obj = obj->parent;
    while(obj->type != CLASS_TYPE) {
        obj = obj->parent;
    }
    return obj;
}
