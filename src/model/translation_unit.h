#ifndef TRANSLATION_UNIT_H
#define TRANSLATION_UNIT_H

#include <filesystem>
#include "object.h"

namespace fs = std::filesystem;

struct TranslationUnit {
    fs::path source;
    fs::path sourceDir;
    fs::path buildHeader;
    fs::path buildCpp;
    Object* root;
    std::vector<Meta> metas;
    bool updated = false;
};

#endif // TRANSLATION_UNIT_H
