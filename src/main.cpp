#include <string>
#include <iostream>
#include <optional>
#include "../include/args-parser/all.hpp"
#include "compiler.h"

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
