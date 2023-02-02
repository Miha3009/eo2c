#ifndef COMPILER_H
#define COMPILER_H

#include <filesystem>
#include <optional>
#include <vector>

typedef std::filesystem::path path;

void compile(path sources, std::optional<path> buildPath);

path getFileBuildPath(path file, path sources, std::optional<path> buildPath);

std::vector<path> find_files(path& sources);

std::string readFile(path& file);

bool need_update(path& source, path& build);

#endif // COMPILER_H
