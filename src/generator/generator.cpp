#include <iostream>
#include "generator.h"

void gen(Object* root, std::filesystem::path path) {
    if(!root) {
        return;
    }
    Generator g(root, path);
    g.run();
}

Generator::Generator(Object* root, std::filesystem::path path): root{root}
{
    if(!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }
    path.replace_extension("h");
    outHeader.open(path.string());
    if(!outHeader.is_open()) {
        std::cout << "error: File " << path.string() << " isn't accesible" << std::endl;
        return;
    }
    path.replace_extension("cpp");
    outSource.open(path.string());
    if(!outSource.is_open()) {
        std::cout << "error: File " << path.string() << " isn't accesible" << std::endl;
        return;
    }
    path.replace_extension("");
    filename = path.filename().string();
}

Generator::~Generator() {
    outHeader.close();
    outSource.close();
}

void Generator::run() {
    if(!outHeader.is_open() || !outSource.is_open()) {
        return;
    }

    genHead();
    genStruct(root);
    genInit(root);
    genEval(root);
    genTail();
}

void Generator::genHead() {
    outHeader << "#ifndef " << filename << "_h\n";
    outHeader << "#define " << filename << "_h\n\n";
    outHeader << "#include \"eo_any.h\"\n\n";
    outSource << "#include \"" << filename << ".h\"\n\n";
}

void Generator::genStruct(Object* obj) {
    std::vector<Object*> children = obj->getChildren();
    if(obj->getType() == CLASS_TYPE) {
        outHeader << "struct " << obj->getFullName() << " {\n\tEoHead head;\n";
        for(Object* child : children) {
            if(child->getType() == CLASS_TYPE) {
                outHeader << "\t" << child->getFullName() << " " << child->getName() << ";\n";
            }
        }
        for(std::string& attribute : obj->getAttributes()) {
            outHeader << "\tvoid* " << attribute << ";\n";
        }
        outHeader << "};\n\n";
    }

    for(int i = 0; i < children.size(); ++i) {
        genStruct(children[i]);
    }
}

void Generator::genInit(Object* obj) {
}

void Generator::genEval(Object* obj) {
}

void Generator::genTail() {
    outHeader << "#endif // " << filename << "_h\n";
}
