#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "types.h"

struct Meta {
    std::string type;
    std::string value;
};

struct Attribute {
    std::string name;
    bool isVararg;
};

class Object {
    std::string name;
    std::string value;
    objectType type;
    std::vector<Object*> children;
    std::vector<Attribute> attributes;
    Object* parent;
    bool atom;
    bool clone;
    std::string fullName;

public:
    Object(Object* parent, objectType type);
    ~Object();
    Object* makeChild();
    Object* makeChild(objectType type);
    void clearChildren();
    void setName(std::string name);
    void setValue(std::string value);
    void setType(objectType type);
    void setChildren(std::vector<Object*> children);
    void setParent(Object* parent);
    void setAtom();
    void addAttribute(std::string name);
    void updateInverseNotaion();
    bool validate();
    bool isRoot();
    bool isAtom();
    bool isClone();
    bool isDecorator();
    std::string getName();
    std::string getFullName();
    std::string getValue();
    objectType getType();
    std::vector<Object*> getChildren();
    std::vector<Attribute> getAttributes();
    Object* getParent();
    Object* getClassObject();
    Object* getApplicationHead();
    std::vector<Object*> getApplicationAttributes();
    Object* getApplicationMethod();
    void printDebug();

private:
    bool validate(std::unordered_set<std::string>& context);
    std::pair<bool, std::unordered_set<std::string>> checkDuplicateNames();
    void printDebug(int s);
};

Object* optimize(Object* obj);
bool endsWithDot(std::string str);
std::string convertName(std::string name);
void updateNameTagTable(Object* obj, std::unordered_map<std::string, int>& nameTagTable);

#endif // MODEL_H
