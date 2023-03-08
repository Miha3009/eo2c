#ifndef ID_TAG_TABLE_H
#define ID_TAG_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>

class IdTagTable {
    std::unordered_map<std::string, int> map;
    std::vector<bool> bitmap;
    int next_tag = 0;

public:
    void update(std::string id);
    void update(std::string id, int tag);
    std::unordered_map<std::string, int>& getMap();
    std::string getTag(std::string id);
};

#endif // ID_TAG_TABLE_H
