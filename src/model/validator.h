#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "object.h"
#include "translation_unit.h"
#include "imports_map.h"
#include <ostream>
#include <vector>

class Validator {
    TranslationUnit& unit;
    ImportsMap& importsMap;
    std::ostream* errorStream;

public:
    Validator(TranslationUnit& unit, ImportsMap& importsMap);
    bool validate();
    bool validate(Object* obj);
    bool checkDuplicateNames(Object* obj);
    bool checkAnonymous(Object* obj);
    bool checkAttributes(Object* obj);
    bool checkContext(Object* obj);
    bool checkChildrenTypes(Object* obj, std::vector<objectType> types);
    bool checkMetas(std::vector<Meta>& metas);
    void setErrorStream(std::ostream* stream);
};

#endif // VALIDATOR_H
