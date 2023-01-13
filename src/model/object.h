#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include "types.h"

class Object {
    std::string name;
    std::string value;
    objectType type;
    std::vector<Object*> children;
    std::vector<std::string> attributes;
    Object* parent;

public:
    Object(Object* parent, objectType type);
    ~Object();
    Object* makeChild(objectType type);
    void setName(std::string name);
    void setValue(std::string value);
    void setType(objectType type);
    void addAttribute(std::string&& name);
    Object* swapWithParent();
    bool validate();
    bool isRoot();
    std::string& getName();
    std::string getFullName();
    std::string& getValue();
    objectType getType();
    std::vector<Object*> getChildren();
    std::vector<std::string> getAttributes();
    Object* getParent();
    Object* getRefObject();
    Object* delete_();
    void printDebug();

private:
    Object* scopeObject(Object* obj);
    void printDebug(int s);
};

#endif // MODEL_H
