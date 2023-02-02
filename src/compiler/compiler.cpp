#include <unordered_map>
#include <sstream>
#include <fstream>
#include <iostream>
#include "compiler.h"
#include "parser.h"
#include "object.h"
#include "generator.h"

#define DEBUG

void compile(path sources, std::optional<path> buildPath) {
    std::vector<path> files = find_files(sources);
    std::vector<Object*> objects;
    std::vector<std::vector<Meta>> metas;
    std::unordered_map<std::string, int> nameTagTable;
    for(path file : files) {
        Object* obj = new Object(nullptr, CLASS_TYPE);
        std::vector<Meta> meta;
        if(!parse(readFile(file).c_str(), file.string(), obj, meta)) {
            return;
        }
        if(!obj->validate()) {
            return;
        }
        objects.push_back(optimize(obj));
        updateNameTagTable(objects.back(), nameTagTable);
        metas.push_back(meta);
#ifdef DEBUG
        objects.back()->printDebug();
#endif
    }
    for(int i = 0; i < objects.size(); ++i) {
        path fileBuildPath = getFileBuildPath(files[i], sources, buildPath);
        if(need_update(fileBuildPath, files[i]) && !gen(objects[i], metas[i], fileBuildPath, nameTagTable)) {
            return;
        }
    }
}

std::vector<path> find_files(path& sources) {
    if(!std::filesystem::is_directory(sources)) {
        return {sources};
    }
    std::vector<path> files;
    for(auto const& file : std::filesystem::recursive_directory_iterator(sources)) {
        if(file.is_regular_file() && file.path().extension() == ".eo") {
            files.push_back(file.path());
        }
    }
    return files;
}

path getFileBuildPath(path file, path sources, std::optional<path> buildPath) {
    if(buildPath) {
        if(std::filesystem::is_directory(sources)) {
            return buildPath.value() / file.lexically_relative(sources);
        } else {
            return buildPath.value();
        }
    }
    return file;
}

std::string readFile(path& file) {
    std::ifstream fileStream(file);
    if(fileStream.fail()) {
        std::cout << "error: File " << file.string() << " isn't accesible" << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << fileStream.rdbuf();
    return ss.str();
}

bool need_update(path& source, path& build) {
    return true;
    build.replace_extension("cpp");
    if(!std::filesystem::exists(build)) {
        return true;
    }
    auto source_time = std::filesystem::last_write_time(source);
    auto build_time = std::filesystem::last_write_time(build);
    return source_time > build_time;
}
