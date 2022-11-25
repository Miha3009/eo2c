#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../parser/parser.h"

int main()
{
    std::string fileName = "app.eo";
    std::ifstream fileStream(fileName);
    if(fileStream.fail()) {
        std::cout << "Module" << fileName << " isn't accesible" << std::endl;
        return 1;
    }

    std::stringstream ss;
    ss << fileStream.rdbuf();
    std::string module(ss.str());
    parse(module.c_str(), fileName);

    return 0;
}
