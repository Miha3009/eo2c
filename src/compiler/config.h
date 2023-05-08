#ifndef CONFIG_H
#define CONFIG_H

#include <filesystem>
#include <vector>
#include <unordered_map>
#include "id_tag_table.h"
#include "translation_unit.h"

namespace fs = std::filesystem;

class Config {
    fs::path sources;
    fs::path configFile;
    fs::path buildPath;
    fs::path exeDir;
    std::string compilerVersion;
    std::string mainObjectPackage;
    bool mainObjectPackageChanged;
    bool quiet;
    std::string cmakeFlags;
    bool cmakeFlagsChanged;
    std::string arguments;
    IdTagTable idTagTable;
    std::vector<fs::path> eoFiles;
    int stackSize;
    bool stackSizeChanged;
    int lib;
    bool libChanged;
    std::string projectName;
    bool projectNameChanged;

public:
    Config(fs::path exeDir);
    void load();
    void save();
    void clean();
    bool isProjectStructureChanged(std::vector<TranslationUnit>& units);
    void setSources(fs::path sources);
    void setBuildPath(fs::path buildPath);
    void setMainObjectPackage(std::string mainObjectPackage);
    void setQuiet(bool quiet);
    void setCMakeFlags(std::string cmakeFlags);
    void setArguments(std::string arguments);
    void setStackSize(std::string stack);
    void setProjectName(std::string projectName);
    void setLib(bool lib);
    fs::path getExeDir();
    fs::path getSources();
    fs::path getBuildPath();
    std::string getMainObjectPackage();
    std::string getCMakeFlags();
    std::string getArguments();
    std::string getProjectName();
    IdTagTable& getIdTagTable();
    int getStackSize();
    bool isQuiet();
    bool isLib();
    bool isMainObjectPackageChanged();
    bool isCMakeFlagsChanged();
    bool isStackSizeChanged();
    bool isProjectNameChanged();
    bool isLibChanged();

private:
    void updateProjectStructure(std::vector<TranslationUnit>& units);
};

#endif // CONFIG_H
