#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>
#include "object.h"
#include "translation_unit.h"
#include "id_tag_table.h"

class Optimizer {
    TranslationUnit& unit;
    IdTagTable& idTagTable;

public:
    Optimizer(TranslationUnit& unit, IdTagTable& idTagTable);
    bool run();

private:
    Object* removeRootWithOneChild(Object* obj);
    void applyInverseNotation(Object* obj);
    Object* removeReduntObjects(Object* obj);
    void convertIdentifier(Object* obj);
    std::string convertIdentifier(std::string id);
    void updateIdTagTable(Object* obj);
    char toHex(int x);
};

#endif // OPTIMIZER_H
