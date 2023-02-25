#ifndef COMPILER_H
#define COMPILER_H

#include <filesystem>
#include <optional>
#include <vector>

typedef std::filesystem::path path;

void compile(path sources, std::optional<path> buildPath);
path getFileBuildPath(path file, path sources, std::optional<path> buildPath);
std::vector<path> findFiles(path& sources);
std::string readFile(path& file);
bool needUpdate(path& source, path& build);
std::string getFilename(path file);

#endif // COMPILER_H
