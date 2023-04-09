#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>
#include "object.h"
#include "translation_unit.h"
#include "id_tag_table.h"
#include "imports_map.h"

class Optimizer {
    TranslationUnit& unit;
    IdTagTable& idTagTable;
    ImportsMap& importsMap;

public:
    Optimizer(TranslationUnit& unit, IdTagTable& idTagTable, ImportsMap& importsMap);
    bool run();

private:
    void applyInverseNotation(Object* obj);
    void expandQQ(Object* obj, TranslationUnit& unit);
    Object* removeReduntObjects(Object* obj);
    void convertIdentifier(Object* obj);
    std::string convertIdentifier(std::string id);
    void updateIdTagTable(Object* obj);
    char toHex(int x);
};

#endif // OPTIMIZER_H
