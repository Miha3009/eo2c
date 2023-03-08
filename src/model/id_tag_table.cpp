#include "id_tag_table.h"
#include <iostream>

void IdTagTable::update(std::string id) {
    if(id == "" || map.count(id) == 1) {
        return;
    }
    while(next_tag < bitmap.size() && bitmap[next_tag]) ++next_tag;
    if(next_tag >= bitmap.size()) {
        bitmap.resize(next_tag + 1);
    }
    bitmap[next_tag] = true;
    map[id] = next_tag;
}

void IdTagTable::update(std::string id, int tag) {
    if(tag >= bitmap.size()) {
        bitmap.resize(tag + 1, false);
    }
    bitmap[tag] = true;
    map[id] = tag;
}

std::unordered_map<std::string, int>& IdTagTable::getMap() {
    return map;
}

std::string IdTagTable::getTag(std::string id) {
    if(map.count(id) == 1) {
        return std::to_string(map[id]);
    }
    return "";
}