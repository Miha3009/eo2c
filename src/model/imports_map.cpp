#include "imports_map.h"

void ImportsMap::build(fs::path buildPath, std::vector<TranslationUnit>& units) {
    this->buildPath = buildPath;
    for(TranslationUnit& unit : units) {
        std::string package = "";
        for(Meta& meta : unit.metas) {
            if(meta.type == "package") {
                package = meta.value;
            }
        }
        if(package != "" && unit.root->getOriginValue() != "") {
            map[package + "." + unit.root->getOriginValue()] = &unit;
        }
    }
}

TranslationUnit* ImportsMap::getUnit(std::string alias) {
    if(map.count(alias) == 1) {
        return map.at(alias);
    }
    return nullptr;
}

std::string ImportsMap::getImport(TranslationUnit& from, std::string alias) {
    if(map.count(alias) == 1) {
        return fs::relative(map.at(alias)->buildHeader, from.buildHeader.parent_path()).string();
    }
    return "";
}

std::string ImportsMap::getObjectImport(TranslationUnit& from) {
    return fs::relative(buildPath / fs::path("object.h"), from.buildHeader.parent_path()).string();
}

std::string ImportsMap::getClassName(std::string alias) {
    if(map.count(alias) == 1) {
        return map.at(alias)->root->getClassName();
    }
    return "";
}