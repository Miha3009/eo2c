#ifndef APPLICATION_GENERATOR_H
#define APPLICATION_GENERATOR_H

#include "object.h"
#include "code_model.h"
#include <unordered_set>
#include <unordered_map>

class ApplicationGenerator {
    Object* root;
    Function f;
    CodeModel& codeModel;
    std::string resultVar;
    int innerFunctionsCount;
    std::unordered_set<std::string> varsWithAttributes;
    std::unordered_map<Object*, std::string> variableNames;

public:
    ApplicationGenerator(Object* root, CodeModel& codeModel, std::string signature);
    Function& getFunction();
    std::string getResultVar();
    void run();

private:
    void genApplication(Object* obj);
    void genPart(Object* obj, std::string parentVar, bool isHead, bool isTemp);
    void genMetaObject(Object* obj);
    void genVar(Object* obj, std::string parentVar, bool isTemp);
    void genRef(Object* obj, std::string parentVar, bool isTemp);
    void genData(Object* obj);
    void genSequenceHead(Object* obj, bool isHead);
    void genSequenceTail(Object* obj, bool isHead);
    void genAttributesLength(Object* obj);
    void clearEvaluate();
    void print(Object* obj, int s);
};

#endif //APPLICATION_GENERATOR_H
