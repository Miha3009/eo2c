#include <iostream>
#include <algorithm>
#include "cmake_generator.h"
#include "util.h"

CmakeGenerator::CmakeGenerator(std::vector<TranslationUnit>& units, fs::path buildPath, bool lib, std::string project): 
    units{units}, buildPath{buildPath}, lib{lib}, project{project} {
}

CmakeGenerator::~CmakeGenerator() {
    out.close();
}

bool CmakeGenerator::open() {
    fs::path file = buildPath / fs::path("CMakeLists.txt");
    out.open(file);
    if(!out.is_open()) {
        std::cout << "error: File " << file.string() << " isn't accesible" << std::endl;
        return false;
    }
    return true;
}

void CmakeGenerator::run() {
    writeHead();
    writeIncludeDirectories();
    writeSetSources();
    writeTail();
}

void CmakeGenerator::writeHead() {
    out << "cmake_minimum_required(VERSION 3.5 FATAL_ERROR)\n\n";
    out << "set(CMAKE_CXX_STANDARD 11)\n\n";
    out << "project(" << project << ")\n\n";
}

void CmakeGenerator::writeIncludeDirectories() {
    std::vector<std::string> directories;
    for(auto& dir : fs::directory_iterator(buildPath)) {
        if(fs::is_directory(dir) && dir.path().filename() != "build") {
            directories.push_back(dir.path().filename().string());
        }
    }
    out << "include_directories(";
    for(int i = 0; i < directories.size(); ++i) {
        if(i != 0) {
            out << " ";
        }
        out << directories[i];
    }
    out << ")\n\n";
}

void CmakeGenerator::writeSetSources() {
    out << "set(SOURCES object.cpp";
    if(!lib) {
        out << " main.cpp";
    }
    for(TranslationUnit& unit : units) {
        out << " " << convertSeparator(unit.buildCpp.lexically_relative(buildPath).string());
    }
    out << ")\n\n";
}

void CmakeGenerator::writeTail() {
    if(lib) {
        out << "add_library(" << project << " ${SOURCES})\n";
    } else {
        out << "add_executable(" << project << " ${SOURCES})\n";
    }
}
