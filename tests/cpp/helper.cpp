#include "helper.h"

TranslationUnit makeUnit() {
    TranslationUnit unit;
    unit.root = new Object(nullptr, CLASS_TYPE);
    unit.root->setOriginValue("root");
    return unit;
}
