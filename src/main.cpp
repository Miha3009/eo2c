#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "parser.h"

std::vector<std::string> find_files(std::string path) {
    if(!std::filesystem::is_directory(path)) {
        return {path};
    }

    std::vector<std::string> files;
    for(auto const& file : std::filesystem::recursive_directory_iterator(path)) {
        if(file.is_regular_file() && file.path().extension() == ".eo") {
            files.push_back(file.path().string());
        }
    }
    return files;
}

int main(int argc, char** argv)
{
    if(argc < 2) {
        std::cout << "Not enough arguments. You must provide a path to source code";
        return 1;
    }
    std::string path(argv[1]);
    std::vector<std::string> files = find_files(path);
    for(std::string fileName : files) {
        std::ifstream fileStream(fileName);
        if(fileStream.fail()) {
            std::cout << "File" << fileName << " isn't accesible" << std::endl;
            continue;
        }
        std::stringstream ss;
        ss << fileStream.rdbuf();
        std::string file(ss.str());
        parse(file.c_str(), fileName);
    }

    return 0;
}
