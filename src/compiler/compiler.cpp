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
#include "cmake_generator.h"
#include "entry_point_generator.h"

//#define DEBUG

static const std::string EOLANG_LIBRARY_PATH = "eo";

Compiler::Compiler(Config& config): config{config} {
}

bool Compiler::generate() {
    config.load();
    if(!validateArguments()) {
        return false;
    }
    std::vector<TranslationUnit> units = makeUnits();
    if(!parse(units)) {
        return  false;
    }
    importsMap.build(config.getBuildPath(), units);
    if(!optimize(units) || !genSources(units) || !copyBaseFiles() || !genCmake(units) || !genMain(units)) {
        return false;
    }
    config.save();
    return true;
}

bool Compiler::compile() {
    return generate() && runMake();
}

bool Compiler::dataize() {
    return compile() && runExecutable();
}

void Compiler::configure() {
    config.save();
}

void Compiler::clean() {
    config.load();
    if(fs::exists(config.getBuildPath())) {
        fs::remove_all(config.getBuildPath());
    }
    config.clean();
}

bool Compiler::validateArguments() {
    if(!fs::exists(config.getSources())) {
        std::cout << "File " << config.getSources().string() << " does not exist\n";
        return false;
    }
    if(!fs::exists(config.getBuildPath())) {
        try {
            fs::create_directories(config.getBuildPath());
        } catch(const std::exception& e) {
            std::cout << e.what() << "\n";
            return false;
        }
    } else {
        if(!fs::is_directory(config.getBuildPath())) {
            std::cout << "Build path " << config.getBuildPath().string() << " must be a directory\n";
            return false;
        }
    }
    return true;
}

std::vector<TranslationUnit> Compiler::makeUnits() {
    std::vector<TranslationUnit> units;
    if(fs::is_directory(config.getSources())) {
        addUnitsFromPath(config.getSources(), units);
    } else {
        TranslationUnit unit;
        unit.source = config.getSources();
        unit.sourceDir = config.getSources().parent_path();
        units.push_back(unit);
    }
    addUnitsFromPath(fs::absolute(config.getExeDir() / fs::path(EOLANG_LIBRARY_PATH)), units);
    for(TranslationUnit& unit : units) {
        std::string filename = unit.source.stem().filename().string();
        fs::path sourceRelative = unit.source.parent_path().lexically_relative(unit.sourceDir);
        fs::path buildDir = (config.getBuildPath() / sourceRelative).lexically_normal();
        unit.buildCpp = buildDir / fs::path(filename + ".cpp");
        unit.buildHeader = buildDir / fs::path(filename + ".h");
        unit.root = new Object(nullptr, CLASS_TYPE);
        unit.root->setOriginValue(filename);
    }
#ifdef DEBUG
    std::cout << "UNITS\n";
    for(TranslationUnit unit : units) {
        std::cout << "SOURCE: " << unit.source << ", BUILD_CPP: " << unit.buildCpp << ", BUILD_H: " << unit.buildHeader << "\n";
    }
#endif // DEBUG
    return units;
}

bool Compiler::parse(std::vector<TranslationUnit>& units) {
    int successCount = 0;
    for(TranslationUnit& unit : units) {
        std::string content = readFile(unit.source);
        Parser p(unit, content.c_str());
        if(p.isProgram()) {
            ++successCount;
#ifdef DEBUG
            std::cout << "OK: " << unit.source.string() << "\n";
        } else {
            std::cout << "FAIL: " << unit.source.string() << "\n";
#endif // DEBUG
        }
    }
    if(!config.isQuiet()) {
        std::cout << "Pasrsed " << successCount << " files.\n";
    }
    return successCount == units.size();
}

bool Compiler::optimize(std::vector<TranslationUnit>& units) {
    int successCount = 0;
    for(TranslationUnit& unit : units) {
        Optimizer optimizer(unit, config.getIdTagTable(), importsMap);
        if(optimizer.run()) {
            ++successCount;
#ifdef DEBUG
            std::cout << "OK: " << unit.source.string() << "\n";
        } else {
            std::cout << "FAIL: " << unit.source.string() << "\n";
#endif // DEBUG
        }
    }
    if(!config.isQuiet()) {
        std::cout << "Optimized " << successCount << " files.\n";
    }
    return successCount == units.size();
}

bool Compiler::genSources(std::vector<TranslationUnit>& units) {
    int successCount = 0;
    int skipCount = 0;
    for(TranslationUnit& unit : units) {
        if(fs::exists(unit.buildCpp) && fs::last_write_time(unit.source) < fs::last_write_time(unit.buildCpp) &&
           fs::exists(unit.buildHeader) && fs::last_write_time(unit.source) < fs::last_write_time(unit.buildHeader)) {
            ++skipCount;
            continue;
        }
        Generator generator(unit, config.getIdTagTable(), importsMap, config.getExeDir());
        if(generator.run()) {
            unit.updated = true;
            ++successCount;
#ifdef DEBUG
            std::cout << "OK: " << unit.source.string() << "\n";
        } else {
            std::cout << "FAIL: " << unit.source.string() << "\n";
#endif // DEBUG
        }
    }
    if(!config.isQuiet()) {
        std::cout << "Generated " << successCount << " files, skiped " << skipCount << " files.\n";
    }
    return successCount + skipCount == units.size();
}

bool Compiler::copyBaseFiles() {
    try {
        fs::path objectH = config.getBuildPath() / fs::path("object.h");
        fs::path objectCpp = config.getBuildPath() / fs::path("object.cpp");
        if(!fs::exists(objectH)) {            
            fs::copy(config.getExeDir() / fs::path("templates") / fs::path("object.h"), objectH);

        }
        if(!fs::exists(objectCpp)) {
            fs::copy(config.getExeDir() / fs::path("templates") / fs::path("object.cpp"), objectCpp);
        }
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
        return false;
    }
    return true;
}

bool Compiler::genMain(std::vector<TranslationUnit>& units) {
    TranslationUnit mainUnit;
    mainUnit.buildCpp = config.getBuildPath() / fs::path("main.cpp");
    mainUnit.buildHeader = config.getBuildPath() / fs::path("main.h");
    TranslationUnit* unit = importsMap.getUnit(config.getMainObjectPackage());
    if(fs::exists(mainUnit.buildCpp) && fs::exists(mainUnit.buildHeader) && !config.isMainObjectPackageChanged() && unit && !unit->updated && !config.isStackSizeChanged()) {
        return true;
    }
    EntryPointGenerator generator(mainUnit, config.getIdTagTable(), importsMap, units, config.getMainObjectPackage(), config.getStackSize());
    return generator.run();
}

bool Compiler::genCmake(std::vector<TranslationUnit>& units) {
    if(!config.isProjectStructureChanged(units) && !config.isCMakeFlagsChanged() && fs::exists(config.getBuildPath() / fs::path("CMakeLists.txt"))) {
        return true;
    }
    CmakeGenerator cmakeGenerator(units, config.getBuildPath());
    if(!cmakeGenerator.open()) {
        return false;
    }
    cmakeGenerator.run();
    return true;
}

bool Compiler::runMake() {
    fs::path makeDir = config.getBuildPath() / fs::path("build");
    std::string cmd;
    if(config.isCMakeFlagsChanged()) {
        fs::remove_all(makeDir);
    }
    if(!fs::exists(makeDir)) {
        cmd = "cmake -B " + makeDir.string() + " " + config.getCMakeFlags() + " " + config.getBuildPath().string();
        if(config.isQuiet()) {
            cmd += " > nul";
        }
        if(std::system(cmd.c_str())) {
            return false;
        }
    }
    cmd = "make -s -C " + makeDir.string();
    if(config.isQuiet()) {
        cmd += " > nul";
    }
    return std::system(cmd.c_str()) == 0;
}

bool Compiler::runExecutable() {
#ifdef __WIN32__
    std::string cmd = "\"" + (config.getBuildPath() / fs::path("build") / fs::path("eolang.exe")).string() + "\" " + config.getArguments();
    return std::system(cmd.c_str()) == 0;
#elif defined(__linux__)
    std::string cmd = "./" + (config.getBuildPath() / fs::path("build") / fs::path("eolang")).string() + " " + config.getArguments();
    return std::system(cmd.c_str()) == 0;
#else
    return false;
#endif
}

void Compiler::addUnitsFromPath(fs::path sourceDir, std::vector<TranslationUnit>& units) {
    for(auto const& file : fs::recursive_directory_iterator(sourceDir)) {
        if(file.is_regular_file() && file.path().extension() == ".eo") {
            TranslationUnit unit;
            unit.source = file.path();
            unit.sourceDir = sourceDir;
            units.push_back(unit);
        }
    }
}

std::string Compiler::readFile(fs::path file) {
    std::ifstream fileStream(file);
    if(fileStream.fail()) {
        std::cout << "error: File " << file.string() << " isn't accesible" << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << fileStream.rdbuf();
    return ss.str();
}
