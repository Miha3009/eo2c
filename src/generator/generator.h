#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <fstream>
#include <filesystem>
#include "object.h"

void gen(Object* root, std::filesystem::path path);

class Generator {
    Object* root;
    std::ofstream outHeader;
    std::ofstream outSource;
    std::string filename;

public:
    Generator(Object* root, std::filesystem::path path);
    ~Generator();
    void run();
    void genHead();
    void genStruct(Object* obj);
    void genInit(Object* obj);
    void genEval(Object* obj);
    void genTail();
};

#endif // GENERATOR_H
