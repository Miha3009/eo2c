#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <vector>
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
    int flags;

public:
    Object(Object* parent, objectType type);
    ~Object();
    Object* makeChild(objectType type);
    Object* makeChild(objectType type, std::string value);
    Object* makeParent(objectType type);
    void addChild(Object* child);
    void setChildren(std::vector<Object*> children);
    void clearChildren();
    void deleteLastChild();
    void setValue(std::string value);
    void setOriginValue(std::string value);
    void setType(objectType type);
    void setParent(Object* parent);
    void addFlags(int flags);
    void addAttribute(std::string name);
    void setAttributes(std::vector<Attribute> attributes);
    void addToSequence(objectType type, std::string value);
    bool isRoot();
    bool isDecorator();
    bool hasFlags(int flags);
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
    bool equals(Object* obj, bool withFlags);
    void printDebug();

private:
    void printDebug(int s);
};

#endif // OBJECT_H
