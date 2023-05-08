#include <iostream>
#include <algorithm>
#include "object.h"

static int anonymousClassCount = 0;

Object::Object(Object* parent, objectType type) {
    this->type = type;
    this->parent = parent;
    this->flags = 0;
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
    return makeChild(type, "");
}

Object* Object::makeChild(objectType type, std::string value) {
    Object* child = new Object(this, type);
    child->setOriginValue(value);
    children.push_back(child);
    return child;
}

Object* Object::makeParent(objectType type) {
    Object* tmpObj = new Object(this, this->type);
    tmpObj->flags = this->flags;
    tmpObj->originValue = this->originValue;
    tmpObj->value = this->value;
    tmpObj->setChildren(this->children);
    this->flags = 0;
    this->originValue = "";
    this->value = "";
    clearChildren();
    addChild(tmpObj);
    this->type = type;
    return this;
}

void Object::addChild(Object* child) {
    children.push_back(child);
    child->setParent(this);
}

void Object::setChildren(std::vector<Object*> children) {
    this->children = children;
    for(Object* child : children) {
        child->setParent(this);
    }
}

void Object::clearChildren() {
    children.clear();
}

void Object::deleteLastChild() {
    if(children.size() > 0) {
        delete children[children.size()-1];
        children.pop_back();
    }
}

void Object::setValue(std::string value) {
    this->value = value;
}

void Object::setOriginValue(std::string value) {
    int l = value.length();
    if((type == VAR_TYPE || type == REF_TYPE) && l > 0) {
        if(l > 3 && value[0] == '.' && value[1] == '.' && value[2] == '.') {
            addFlags(VARARGS_FLAG);
            value = value.substr(3);
        }
        if(value[l-1] == '\'') {
            addFlags(CLONE_FLAG);
            value = value.substr(0, l-1);
        }
        if(value[l-1] == '.') {
            addFlags(DOT_FLAG);
            value = value.substr(0, l-1);
        }
        if(value[l-1] == '!') {
            addFlags(CONSTANT_FLAG);
            value = value.substr(0, l-1);
        }
    }
    this->originValue = value;
    this->value = value;
}

void Object::setType(objectType type) {
    this->type = type;
}

void Object::setParent(Object* parent) {
    this->parent = parent;
}

void Object::addFlags(int flags) {
    this->flags |= flags;
}

void Object::addAttribute(std::string name) {
    bool isVararg = false;
    int l = name.length();
    if(l > 2 && name[l-1] == '.' && name[l-2] == '.' && name[l-3] == '.') {
        name = name.substr(0, l-3);
        isVararg = true;
    }
    attributes.push_back({name, isVararg});
}

void Object::setAttributes(std::vector<Attribute> attributes) {
    this->attributes = attributes;
}

void Object::addToSequence(objectType type, std::string value) {
    if(this->type != SEQUENCE_TYPE) {
        Object* child = new Object(this, this->type);
        child->setOriginValue(this->originValue);
        child->setChildren(this->children);
        this->type = SEQUENCE_TYPE;
        this->value = "";
        this->children.clear();
        this->children.push_back(child);
    }
    Object* child = makeChild(type);
    child->setOriginValue(value);
}

bool Object::isRoot() {
    return parent == nullptr;
}

bool Object::isDecorator() {
    return originValue == "@";
}

bool Object::hasFlags(int flags) {
    return (this->flags & flags) == flags;
}

std::string Object::getClassName() {
    if(value.empty()) {
        value = std::to_string(anonymousClassCount++);
    }
    std::string className = value;
    Object* obj = parent;
    while(!obj->isRoot()) {
        if(obj->type == CLASS_TYPE) {
            if(obj->value.empty()) {
                value = std::to_string(anonymousClassCount++);
            }
            className = obj->value + "_" + className;
        }
        obj = obj->parent;
    }
    return className;
}

std::string Object::getValue() {
    return value;
}

std::string Object::getOriginValue() {
    return originValue;
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
    } else if(parent != nullptr) {
        return parent->getClassObject();
    }
    return nullptr;
}

Object* Object::getApplicationHead() {
    return children[0];
}

std::vector<Object*> Object::getApplicationAttributes() {
    std::vector<Object*> result;
    for(int i = 1; i < children.size(); ++i) {
        result.push_back(children[i]);
    }
    return result;
}

bool Object::equals(Object* obj, bool withFlags) {
    if(this->type != obj->type
    || this->originValue != obj->originValue
    || (withFlags && this->flags != obj->flags)
    || this->attributes.size() != obj->attributes.size()
    || this->children.size() != obj->children.size()) {
        return false;
    }

    for(int i = 0; i < attributes.size(); ++i) {
        if(this->attributes[i].name != obj->attributes[i].name
        || this->attributes[i].isVararg != obj->attributes[i].isVararg) {
            return false;
        }
    }
    for(int i = 0; i < children.size(); ++i) {
        if(!this->children[i]->equals(obj->children[i], withFlags)) {
            return false;
        }
    }

    return true;
}

void Object::printDebug() {
    printDebug(0);
}

void Object::printDebug(int s) {
    std::cout << std::string(s, ' ');
    if(type) std::cout << "type=" << objectTypeNames[type] << " ";
    if(!value.empty()) std::cout << "value=" << value << " ";
    if(flags != 0) std::cout << "flags=" << flags << " ";
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
