#include "validator.h"
#include <iostream>
#include <unordered_set>
#include <algorithm>

Validator::Validator(TranslationUnit& unit, ImportsMap& importsMap): unit{unit}, importsMap{importsMap}, errorStream{&std::cout} {
}

bool Validator::validate() {
    return checkMetas(unit.metas) && validate(unit.root);
}

bool Validator::validate(Object* obj) {
    switch (obj->getType()) {
    case CLASS_TYPE:
        if(!checkDuplicateNames(obj) || !checkAnonymous(obj) || !checkAttributes(obj) || !checkChildrenTypes(obj, {CLASS_TYPE, APPLICATION_TYPE})) {
            return false;
        }
        break;
    case VAR_TYPE:
        if(!checkContext(obj)) {
            return false;
        }
        break;
    case BYTES_TYPE:
        if(!checkChildrenTypes(obj, {BYTE_TYPE})) {
            return false;
        }
        break;
    default:
        break;
    }
    for(Object* child : obj->getChildren()) {
        if(!validate(child)) return false;
    }
    return true;
}

bool Validator::checkDuplicateNames(Object* obj) {
    std::unordered_set<std::string> names;
    for(Attribute& a : obj->getAttributes()) {
        if(names.count(a.name)) {
            *errorStream << unit.source.string() << ": error: attribute with name " << a.name << " already defined\n";
            return false;
        }
        names.insert(a.name);
    }
    for(Object* child : obj->getChildren()) {
        if(child->getValue() != "") {
            if(names.count(child->getValue())) {
                *errorStream << unit.source.string() << ": error: object with name " << child->getOriginValue() << " already defined\n";
                return false;
            }
            names.insert(child->getValue());
        }
    }
    return true;
}

bool Validator::checkAnonymous(Object* obj) {
    for(Object* child : obj->getChildren()) {
        if(child->getOriginValue() == "") {
            *errorStream << unit.source.string() << ": error: anonymous object unacceptable here\n";
            return false;
        }
    }
    return true;
}

bool Validator::checkAttributes(Object* obj) {
    std::vector<Attribute> attributes = obj->getAttributes();
    for(int i = 0; i + 1 < attributes.size(); ++i) {
        if(attributes[i].isVararg) {
            *errorStream << unit.source.string() << ": error: varargs attribute must be the last\n";
            return false;
        }
    }
    return true;
}

bool Validator::checkContext(Object* obj) {
    std::string value = obj->getOriginValue();
    if((obj->getParent()->getType() == SEQUENCE_TYPE && obj != obj->getParent()->getChildren()[0])
    || (obj->getParent()->getType() == APPLICATION_TYPE && obj->hasFlags(DOT_FLAG))
    || value == "QQ") {
        return true;
    }
    for(Meta meta : unit.metas) {
        if(meta.type == "alias" && meta.value.length() >= value.length() + 1 &&
           0 == meta.value.compare(meta.value.length() - value.length() - 1, value.length() + 1, "." + value)) {
            return true;
        }
    }
    Object* classObj = obj;
    while(classObj != nullptr) {
        classObj = classObj->getClassObject();
        for(Object* child : classObj->getChildren()) {
            if(child->getOriginValue() == value) {
                return true;
            }
        }
        for(Attribute attr : classObj->getAttributes()) {
            if(attr.name == value) {
                return true;
            }
        }
        classObj = classObj->getParent();
    }
    *errorStream << unit.source.string() << ": error: variable " << value << " not found in context\n";
    return false;
}

bool Validator::checkChildrenTypes(Object* obj, std::vector<objectType> types) {
    for(Object* child : obj->getChildren()) {
        if(std::find(types.begin(), types.end(), child->getType()) == types.end()) {
            *errorStream << unit.source.string() << ": error: invalid object type " << objectTypeNames[child->getType()] << "\n";
            return false;
        }
    }    
    return true;
}

bool Validator::checkMetas(std::vector<Meta>& metas) {
    for(Meta& meta : metas) {
        if(meta.type == "alias") {
            if(importsMap.getImport(unit, meta.value) == "") {
                *errorStream << unit.source.string() << ": error: object for alias " << meta.value << "not found\n";
                return false;
            }
        }
    }
    return true;
}

void Validator::setErrorStream(std::ostream* stream) {
    errorStream = stream;
}
