#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>
#include "object.h"

class Optimizer {
    std::vector<Object*> objects;
    std::unordered_map<std::string, int> nameTagTable;

public:
    Optimizer(std::vector<Object*> objects);
    bool run();
    std::vector<Object*> getObjects();
    std::unordered_map<std::string, int>& getNameTagTable();

private:
    Object* removeRootWithOneChild(Object* obj);
    void applyInverseNotation(Object* obj);
    Object* removeReduntObjects(Object* obj);
    void convertIdentifier(Object* obj);
    std::string convertIdentifier(std::string id);
    void updateNameTagTable(Object* obj);
    char toHex(int x);
    bool endsWithDot(std::string str);
};

#endif // OPTIMIZER_H
