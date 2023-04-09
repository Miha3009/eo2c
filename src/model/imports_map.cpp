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
        if(package != "") {
            for(Object* obj : unit.root->getChildren()) {
                if(obj->getOriginValue() != "") {
                    map[package + "." + obj->getOriginValue()] = &unit;
                }
            }
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

Object* ImportsMap::getObject(std::string alias, std::string objectName) {
    if(map.count(alias) == 1) {
        TranslationUnit* unit = map.at(alias);
        for(Object* obj : unit->root->getChildren()) {
            if(obj->getOriginValue() == objectName) {
                return obj;
            }
        }
    }
    return nullptr;
}