#include <fstream>
#include "config.h"
#include <iostream>
#include <algorithm>

static const std::string currentCompilerVersion = "1.0.0";

Config::Config(fs::path exeDir): exeDir{exeDir}, mainObjectPackage{""}, arguments{""}, mainObjectPackageChanged{false},
    quiet{false}, cmakeFlagsChanged{false}, stackSize{0}, lib{-1}, libChanged{false}, projectName{""}, projectNameChanged{false} {
    setSources(fs::current_path());
}

void Config::load() {
    if(!fs::exists(configFile)) {
        return;
    }
    std::ifstream s(configFile);
    s >> compilerVersion;
    if(compilerVersion != currentCompilerVersion) {
        return;
    }
    std::string savedBuildPath, savedMainObjectPackage, savedCMakeFlags, savedStackSize, savedProjectName, id, eoFile;
    int savedLib, count, tag;
    s >> savedBuildPath;
    if(buildPath.empty()) {
        buildPath = fs::path(savedBuildPath);
    }
    s >> savedMainObjectPackage;
    if(mainObjectPackage == "") {
        mainObjectPackage = savedMainObjectPackage;
    } else {
        mainObjectPackageChanged = savedMainObjectPackage != mainObjectPackage;
    }
    s >> savedStackSize;
    if(stackSize == 0) {
        setStackSize(savedStackSize);
    } else {
        stackSizeChanged = stackSize != atoi(savedStackSize.c_str());        
    }
    s.get();
    std::getline(s, savedCMakeFlags);
    if(cmakeFlags == "") {
        cmakeFlags = savedCMakeFlags;
    } else {
        cmakeFlagsChanged = savedCMakeFlags != cmakeFlags;
    }
    s >> count;
    for(int i = 0; i < count; ++i) {
        s >> id >> tag;
        idTagTable.update(id, tag);
    }
    s >> count;
    for(int i = 0; i < count; ++i) {
        s >> eoFile;
        eoFiles.push_back(fs::path(eoFile));
    }
    s >> savedLib;
    if(lib == -1) {
        lib = savedLib;
    } else {
        libChanged = savedLib != lib;
    }
    s >> savedProjectName;
    if(projectName == "") {
        projectName = savedProjectName;
    } else {
        projectNameChanged = savedProjectName != projectName;
    }
    s.close();
}

void Config::save() {
    compilerVersion = currentCompilerVersion;
    std::ofstream s(configFile);
    s << compilerVersion << "\n";
    s << fs::absolute(buildPath).string() << "\n";
    s << mainObjectPackage << "\n";
    s << std::to_string(stackSize) << "\n";
    s << cmakeFlags << "\n";
    s << idTagTable.getMap().size() << "\n";
    std::unordered_map<std::string, int>& idMap = idTagTable.getMap();
    std::vector<std::pair<std::string, int>> sortedIdMap(idMap.begin(), idMap.end());
    std::sort(sortedIdMap.begin(), sortedIdMap.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    for(auto& idTag : sortedIdMap) {
        s << idTag.first << " " << idTag.second << "\n";
    }
    s << eoFiles.size() << "\n";
    for(fs::path eoFile : eoFiles) {
        s << eoFile.string() << "\n";
    }
    s << lib << "\n";
    s << projectName << "\n";
    s.close();
}

void Config::clean() {
    if(fs::exists(configFile)) {
        fs::remove(configFile);
    }
}

bool Config::isProjectStructureChanged(std::vector<TranslationUnit>& units) {
    if(eoFiles.size() != units.size()) {
        updateProjectStructure(units);
        return true;
    }
    for(TranslationUnit& unit : units) {
        bool hasEquivalent = false;
        for(fs::path file : eoFiles) {
            if(std::filesystem::equivalent(unit.source, file)) {
                hasEquivalent = true;
                break;
            }
        }
        if(!hasEquivalent) {
            updateProjectStructure(units);
            return true;
        }
    }
    return false;
}

void Config::setSources(fs::path sources) {
    this->sources = fs::absolute(sources);
    if(fs::is_directory(sources)) {
        configFile = this->sources / fs::path(".config");
    } else {
        configFile = this->sources.parent_path() / fs::path(".config");
    }
}

void Config::setBuildPath(fs::path buildPath) {
    this->buildPath = buildPath;
}

void Config::setMainObjectPackage(std::string mainObjectPackage) {
    this->mainObjectPackage = mainObjectPackage;
}

void Config::setQuiet(bool quiet) {
    this->quiet = quiet;
}

void Config::setCMakeFlags(std::string cmakeFlags) {
    this->cmakeFlags = cmakeFlags;
}

void Config::setArguments(std::string arguments) {
    this->arguments = arguments;
}

void Config::setStackSize(std::string stackSize) {
    this->stackSize = atoi(stackSize.c_str());
}

void Config::setLib(bool lib) {
    if(lib) {
        this->lib = 1;
    } else {
        this->lib = 0;
    }
}

void Config::setProjectName(std::string projectName) {
    this->projectName = projectName;
}

fs::path Config::getExeDir() {
    return exeDir;
}

fs::path Config::getSources() {
    return sources;
}

fs::path Config::getBuildPath() {
    if(buildPath.empty()) {
        buildPath = fs::current_path() / fs::path(".eo2c");
    }
    return buildPath;
}

std::string Config::getMainObjectPackage() {
    return mainObjectPackage;
}

std::string Config::getCMakeFlags() {
    return cmakeFlags;
}

std::string Config::getArguments() {
    return arguments;
}

std::string Config::getProjectName() {
    if(projectName == "") {
        setProjectName("eolang");
    }
    return projectName;
}

IdTagTable& Config::getIdTagTable() {
    return idTagTable;
}

int Config::getStackSize() {
    if(stackSize == 0) {
        stackSize = 1 << 20;
    }
    return stackSize;
}

bool Config::isLib() {
    if(lib == -1) {
        return false;
    }
    return lib;
}

bool Config::isMainObjectPackageChanged() {
    return mainObjectPackageChanged;
}

bool Config::isQuiet() {
    return quiet;
}

bool Config::isCMakeFlagsChanged() {
    return cmakeFlagsChanged;
}

bool Config::isStackSizeChanged() {
    return stackSizeChanged;
}

bool Config::isProjectNameChanged() {
    return projectNameChanged;
}

bool Config::isLibChanged() {
    return libChanged;
}

void Config::updateProjectStructure(std::vector<TranslationUnit>& units) {
    eoFiles.clear();
    for(TranslationUnit& unit : units) {
        eoFiles.push_back(unit.source);
    }
}