#ifndef OBJECT_H
#define OBJECT_H

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
    objectType type;
    std::string value;
    std::string originValue;
    std::vector<Object*> children;
    std::vector<Attribute> attributes;
    Object* parent;
    bool atom;
    bool clone;
    bool vararg;
    bool dot;

public:
    Object(Object* parent, objectType type);
    ~Object();
    Object* makeChild();
    Object* makeChild(objectType type);
    void addChild(Object* child);
    void setChildren(std::vector<Object*> children);
    void clearChildren();
    void setValue(std::string value);
    void setOriginValue(std::string value);
    void setType(objectType type);
    void setParent(Object* parent);
    void setAtom();
    void addAttribute(std::string name);
    void setAttributes(std::vector<Attribute> attributes);
    void addToSequence(objectType type, std::string value);
    bool validate();
    bool isRoot();
    bool isAtom();
    bool isClone();
    bool isVararg();
    bool isDot();
    bool isDecorator();
    std::string getClassName();
    std::string getValue();
    std::string getOriginValue();
    objectType getType();
    std::vector<Object*> getChildren();
    std::vector<Attribute> getAttributes();
    Object* getParent();
    Object* getClassObject();
    Object* getApplicationHead();
    std::vector<Object*> getApplicationAttributes();
    void printDebug();

private:
    bool validate(std::unordered_set<std::string>& context);
    std::pair<bool, std::unordered_set<std::string>> checkDuplicateNames();
    void printDebug(int s);
};

#endif // OBJECT_H
