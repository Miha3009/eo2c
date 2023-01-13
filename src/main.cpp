#define DEBUG

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <optional>
#include "parser.h"
#include "object.h"
#include "generator.h"
#include "../include/args-parser/all.hpp"

typedef std::filesystem::path path;

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

void compile(path sources, std::optional<path> buildPath) {
    bool isSourceDirectory = std::filesystem::is_directory(sources);
    std::vector<path> files = find_files(sources);
    for(path file : files) {
        std::string filename = file.string();
        std::ifstream fileStream(file);
        if(fileStream.fail()) {
            std::cout << "error: File " << file.string() << " isn't accesible" << std::endl;
            continue;
        }
        std::stringstream ss;
        ss << fileStream.rdbuf();
        Object obj(nullptr, CLASS_TYPE);
        if(parse(ss.str().c_str(), file.string(), &obj)) {
            obj.printDebug();
            if(obj.validate()) {
                path fileBuild = file;
                if(buildPath) {
                    if(isSourceDirectory) {
                        fileBuild = buildPath.value() / file.lexically_relative(sources);
                    } else {
                        fileBuild = buildPath.value();
                    }
                }
                gen(&obj, fileBuild);
            }
        }
    }
}

int main(int argc, char** argv)
{
    Args::CmdLine cmd(argc, argv, Args::CmdLine::CommandIsRequired);
    try {
		cmd.addCommand(SL("compile"), Args::ValueOptions::OneValue, false,
				SL("Compile EO sources into C++ sources."),
                SL("Compile .eo files into .cpp and .h files."), SL(""), SL("sources"))
			.addArgWithFlagAndName('b', "build", true, false, "Build directory.", "Explicitly specify a build directory.")
			.end()
			.addHelp(true, argv[0], SL("Compiler from EO to C++ (indev)."));

		cmd.parse();
	}
	catch(const Args::HelpHasBeenPrintedException&) {
		return 0;
	}
	catch(const Args::BaseException& x) {
		Args::outStream() << x.desc() << SL( "\n" );
		return 1;
	}

	if(cmd.isDefined("compile")) {
        std::string sourcesPath = cmd.value("compile");
        std::optional<path> buildPath = std::nullopt;
        if(cmd.isDefined("-b")) {
            buildPath = std::optional<path>(path(cmd.value("-b")));
        }
        compile(path(sourcesPath), buildPath);
	}

    return 0;
}
