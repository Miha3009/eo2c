#include <iostream>
#include <algorithm>
#include "entry_point_generator.h"

EntryPointGenerator::EntryPointGenerator(TranslationUnit& mainUnit, IdTagTable& idTagTable, ImportsMap& importsMap, std::vector<TranslationUnit>& units,
    std::string mainObjectPackage): codeModel{mainUnit, idTagTable, importsMap}, units{units}, mainObjectPackage{mainObjectPackage} {    
}

EntryPointGenerator::~EntryPointGenerator() {
    out.close();
}

bool EntryPointGenerator::run() {
    codeModel.addImport(mainObjectPackage);
    if(!writeMain()) {
        return false;
    }
    return codeModel.open() && codeModel.write();
}

bool EntryPointGenerator::writeMain() {
    Function f;
    f.signature = "int main(int argc, char *argv[])";
    TranslationUnit* unit = codeModel.getImportsMap().getUnit(mainObjectPackage);
    if(unit == nullptr) {
        return false;
    }
    std::string className = unit->root->getClassName();
    f.addLine("EO_object* obj = " + genCall("stack_alloc", {genCall("sizeof", {className})}));
    f.addLine(genCall("init_" + className, {"(" + className + "*)obj", "0"}));
    writeArguments(f, unit->root);
    f.addLine("evaluate(obj)");
    f.addLine("return 0");
    codeModel.addFunction(f);
    return true;
}

void EntryPointGenerator::writeArguments(Function& f, Object* mainObject) {
    std::vector<Attribute> attributes = mainObject->getAttributes();
    codeModel.addStdImport("<clocale>");
    f.addLine("setlocale(LC_ALL, \"\")");
    if(attributes.empty()) {
        f.addLine("if(argc > 1) {");
        f.addLine("\twprintf(L\"The program does not accept arguments\")");
        f.addLine("\treturn 0;");
        f.addLine("}");
    } else {
        if(!attributes.back().isVararg) {
            f.addLine("if(argc > " + std::to_string(attributes.size() + 1) + ") {");
            f.addLine("\twprintf(L\"The program accepts no more than " + std::to_string(attributes.size()) + " arguments\")");
            f.addLine("\treturn 0");
            f.addLine("}");
        }
        f.addLine("for(int i = 1; i < argc; ++i) {");
        f.addLine("\tEO_object* arg = make_object_from_arg(argv[i])");
        f.addLine("\tif(arg != nullptr) {");
        f.addLine("\t\tadd_attribute(obj, arg, argc-1)");
        f.addLine("\t}");
        f.addLine("}");
    }
}
