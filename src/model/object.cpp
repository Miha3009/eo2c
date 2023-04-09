#include <iostream>
#include <algorithm>
#include "object.h"

static int anonymousClassCount = 0;

Object::Object(Object* parent, objectType type) {
    this->type = type;
    this->parent = parent;
    this->atom = false;
    this->clone = false;
    this->vararg = false;
    this->dot = false;
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

void Object::setValue(std::string value) {
    this->value = value;
}

void Object::setOriginValue(std::string value) {
    int l = value.length();
    if(type == VAR_TYPE || type == REF_TYPE) {
        if(l > 0 && value[l-1] == '\'') {
            this->clone = true;
            value = value.substr(0, l-1);
        }
        if(l > 3 && value[0] == '.' && value[1] == '.' && value[2] == '.') {
            this->vararg = true;
            value = value.substr(3);
        }
        if(l > 0 && value[l-1] == '.') {
            this->dot = true;
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

bool Object::isVararg() {
    return vararg;
}

bool Object::isDot() {
    return dot;
}

bool Object::isDecorator() {
    return originValue == "@";
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
        if(child->value != "" && (child->type == CLASS_TYPE || child->type == APPLICATION_TYPE)) {
            if(names.count(child->value)) {
                std::cout << "error: object with name " << child->value << " already defined\n";
                return make_pair(false, names);
            }
            names.insert(child->value);
        }
    }
    return make_pair(true, names);
}

void Object::printDebug() {
    printDebug(0);
}

void Object::printDebug(int s) {
    std::cout << std::string(s, ' ');
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
