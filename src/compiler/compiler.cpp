#include <unordered_map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "compiler.h"
#include "parser.h"
#include "object.h"
#include "optimizer.h"
#include "generator.h"

void compile(path sources, std::optional<path> buildPath) {
    std::vector<path> files = findFiles(sources);
    std::vector<Object*> objects(files.size());
    std::vector<std::vector<Meta>> metas(files.size());
    for(int i = 0; i < files.size(); ++i) {
        objects[i] = new Object(nullptr, CLASS_TYPE);
        objects[i]->setName(getFilename(files[i]));
        if(!parse(readFile(files[i]).c_str(), files[i].string(), objects[i], metas[i])) {
            return;
        }
    }
    Optimizer optimizer(objects);
    if(!optimizer.run()) {
        return;
    }
    objects = optimizer.getObjects();
    for(int i = 0; i < objects.size(); ++i) {
        path fileBuildPath = getFileBuildPath(files[i], sources, buildPath);
        if(needUpdate(fileBuildPath, files[i]) && !gen(objects[i], metas[i], fileBuildPath, optimizer.getNameTagTable())) {
            return;
        }
    }
    //std::system("make");
}

std::vector<path> findFiles(path& sources) {
    if(!std::filesystem::is_directory(sources)) {
        return {sources};
    }
    std::vector<path> files;
    for(auto const& file : std::filesystem::recursive_directory_iterator(sources)) {
        if(file.is_regular_file() && file.path().extension() == ".eo") {
            files.push_back(file.path());
        }
    }
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

bool needUpdate(path& source, path& build) {
    return true;
    build.replace_extension("cpp");
    if(!std::filesystem::exists(build)) {
        return true;
    }
    auto source_time = std::filesystem::last_write_time(source);
    auto build_time = std::filesystem::last_write_time(build);
    return source_time > build_time;
}

std::string getFilename(path file) {
    file.replace_extension("");
    return file.filename().string();
}
