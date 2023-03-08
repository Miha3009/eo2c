#include <string>
#include <iostream>
#include <filesystem>
#include "../include/args-parser/all.hpp"
#include "compiler.h"
#include "config.h"

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
    Args::CmdLine cmd(argc, argv, Args::CmdLine::CommandIsRequired);
    try {
        cmd
        .addCommand(SL("generate"), Args::ValueOptions::NoValue, false,
                    SL("Generate C++ sources from EO."),
                    SL("Long description"))
        .end()
        .addCommand(SL("compile"), Args::ValueOptions::NoValue, false,
                    SL("Generate C++ sources from EO sources and compile it."),
                    SL("Long description"))
        .end()
        .addCommand(SL("dataize"), Args::ValueOptions::NoValue, false,
                    SL("Run EOLANG program."),
                    SL("Long description"))
        .end()
        .addCommand(SL("configure"), Args::ValueOptions::NoValue, false,
                    SL("Save configuration."),
                    SL("Long description"))
        .end()
        .addCommand(SL("clean"), Args::ValueOptions::NoValue, false,
                    SL("Delete build directory and config."),
                    SL("Long description"))
        .end()
        .addArgWithFlagAndName('s', "source", true, false, "Source directory.", "Specify a source directory.")
        .addArgWithFlagAndName('b', "build", true, false, "Build directory.", "Explicitly specify a build directory.")
        .addArgWithFlagAndName('m', "main", true, false, "Package of main object.", "Determines which object will be dataized.")
        .addArgWithFlagAndName('q', "quiet", false, false, "Disable progress messages.", "Don't print anything except errors and output of the EO program.")
        .addArgWithFlagAndName('f', "flags", true, false, "Flags for CMake.", "Specify the flags with which CMake will be run during compilation.")
        .addArgWithFlagAndName('a', "arguments", true, false, "Arguments for executable.", "Specifty the arguments that will be passed to the executable program during dataization")
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
    fs::path exeDir = fs::path(argv[0]);
    Config config(exeDir.parent_path());
    if(cmd.isDefined("-s")) {
        config.setSources(fs::path(cmd.value("-s")));
    }
    if(cmd.isDefined("-b")) {
        config.setBuildPath(fs::path(cmd.value("-b")));
    }
    if(cmd.isDefined("-m")) {
        config.setMainObjectPackage(cmd.value("-m"));
    }
    if(cmd.isDefined("-q")) {
        config.setQuiet(true);
    }
    if(cmd.isDefined("-f")) {
        config.setCMakeFlags(cmd.value("-f"));
    }
    if(cmd.isDefined("-a")) {
        config.setArguments(cmd.value("-a"));
    }
    Compiler compiler(config);
    if(cmd.isDefined("generate")) {
        compiler.generate();
    }
    if(cmd.isDefined("compile")) {
        compiler.compile();
    }
    if(cmd.isDefined("dataize")) {
        compiler.dataize();
    }
    if(cmd.isDefined("configure")) {
        compiler.configure();
    }
    if(cmd.isDefined("clean")) {
        compiler.clean();
    }
    return 0;
}
