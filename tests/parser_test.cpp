#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>
#include <vector>
#include "parser.h"

void testParserFunction(std::vector<std::string> strs, std::string name, bool (parser::*func)(), bool result) {
    for(int i = 0; i < strs.size(); ++i) {
        parser p(strs[i].c_str(), name);
        REQUIRE(((&p)->*func)() == result);
        if(result) {
            REQUIRE(p.isEOF());
        }
    }
}

TEST_CASE("Meta test") {
    std::vector<std::string> ok = {"+meta\n", "+alias org.java.package\n"};
    std::vector<std::string> fail = {"", "+"};
    testParserFunction(ok, "isMeta", &parser::isMeta, true);
    testParserFunction(fail, "isMeta", &parser::isMeta, false);
}

TEST_CASE("Object test") {
    std::vector<std::string> ok = {"[a] > b", "[a] > b\n  [c] > d\n"};
    std::vector<std::string> fail = {""};
    testParserFunction(ok, "isObject", &parser::isObject, true);
    testParserFunction(fail, "isObject", &parser::isObject, false);
}

TEST_CASE("Application test") {
    std::vector<std::string> ok = {"a", "a.", "a. > @", "^.a", "^.^.a", "a.b", "(a.b c).d"};
    testParserFunction(ok, "isApplication", &parser::isApplication, true);
}

TEST_CASE("Abstraction test") {
    std::vector<std::string> ok = {"[]", "[a]", "[a...]", "[@]", "[a b]"};
    std::vector<std::string> fail = {"", "[", "[a  b]", "[a b ]"};
    testParserFunction(ok, "isAttributes", &parser::isAttributes, true);
    testParserFunction(fail, "isAttributes", &parser::isAttributes, false);
}

TEST_CASE("Attributes test") {
    std::vector<std::string> ok = {"[]", "[a]", "[a...]", "[@]", "[a b]"};
    std::vector<std::string> fail = {"", "[", "[a  b]", "[a b ]"};
    testParserFunction(ok, "isAttributes", &parser::isAttributes, true);
    testParserFunction(fail, "isAttributes", &parser::isAttributes, false);
}

TEST_CASE("Suffix test") {
    std::vector<std::string> ok = {" > a", " > a!", " > @"};
    std::vector<std::string> fail = {"", " > "};
    testParserFunction(ok, "isSuffix", &parser::isSuffix, true);
    testParserFunction(fail, "isSuffix", &parser::isSuffix, false);
}

TEST_CASE("Name test") {
    std::vector<std::string> ok = {"a", "a546*&%#"};
    std::vector<std::string> fail = {"", "A"};
    testParserFunction(ok, "isName", &parser::isName, true);
    testParserFunction(fail, "isName", &parser::isName, false);
}

TEST_CASE("Bytes test") {
    std::vector<std::string> ok = {"--", "A0", "A0-A0"};
    std::vector<std::string> fail = {"", "A", "A0-"};
    testParserFunction(ok, "isBytes", &parser::isBytes, true);
    testParserFunction(fail, "isBytes", &parser::isBytes, false);
}

TEST_CASE("String test") {
    std::vector<std::string> ok = {"\"\"", "\"abc\"", "\"\\n\"", "\"\\\"\""};
    std::vector<std::string> fail = {"", "\"\n\""};
    testParserFunction(ok, "isString", &parser::isString, true);
    testParserFunction(fail, "isString", &parser::isString, false);
}

TEST_CASE("Integer test") {
    std::vector<std::string> ok = {"1", "-1", "+1", "0x1a"};
    std::vector<std::string> fail = {"", "0x"};
    testParserFunction(ok, "isInteger", &parser::isInteger, true);
    testParserFunction(fail, "isInteger", &parser::isInteger, false);
}

TEST_CASE("Char test") {
    std::vector<std::string> ok = {"'a'", "'\\0'", "'\\\\'", "'\\''"};
    std::vector<std::string> fail = {"", "''", "'a"};
    testParserFunction(ok, "isChar", &parser::isChar, true);
    testParserFunction(fail, "isChar", &parser::isChar, false);
}

TEST_CASE("Regex test") {
    std::vector<std::string> ok = {"/a?/", "/a/g", "/\\//"};
    std::vector<std::string> fail = {"", "//"};
    testParserFunction(ok, "isRegex", &parser::isRegex, true);
    testParserFunction(fail, "isRegex", &parser::isRegex, false);
}

TEST_CASE("Float test") {
    std::vector<std::string> ok = {"1.0", "123.456", "1.", "-1.0", "+1.0", "1e10", "1e-10"};
    std::vector<std::string> fail = {"", "1", ".1", "1.0e"};
    testParserFunction(ok, "isFloat", &parser::isFloat, true);
    testParserFunction(fail, "isFloat", &parser::isFloat, false);
}

TEST_CASE("Bool test") {
    std::vector<std::string> ok = {"TRUE", "FALSE"};
    std::vector<std::string> fail = {"", "true", "false"};
    testParserFunction(ok, "isBool", &parser::isBool, true);
    testParserFunction(fail, "isBool", &parser::isBool, false);
}
