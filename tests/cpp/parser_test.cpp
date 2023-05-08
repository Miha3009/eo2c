#define CATCH_CONFIG_MAIN

#include "../../include/catch.hpp"
#include <string>
#include <vector>
#include "helper.h"
#include "parser.h"
#include "translation_unit.h"

void testParserFunction(std::vector<std::string> strs, bool (Parser::*func)(), bool result) {
    for(int i = 0; i < strs.size(); ++i) {
        TranslationUnit unit = makeUnit();
        Parser p(unit, strs[i].c_str());
        p.setTest(true);
        REQUIRE(((&p)->*func)() == result);
        if(result) {
            REQUIRE(p.isEOF());
        }
        delete unit.root;
    }
}

TEST_CASE("Meta test") {
    std::vector<std::string> ok = {"+meta\n", "+alias org.java.package\n"};
    std::vector<std::string> fail = {"", "+"};
    testParserFunction(ok, &Parser::isMeta, true);
    testParserFunction(fail, &Parser::isMeta, false);
}

TEST_CASE("Object test") {
    std::vector<std::string> ok = {"[x] > a", "[x y...] > z\n  x > a", "[x y...] > z\n  x > a\n  [x] > b", "#Comment\n[] > x"};
    std::vector<std::string> fail = {""};
    testParserFunction(ok, &Parser::isObject, true);
    testParserFunction(fail,&Parser::isObject, false);
}

TEST_CASE("Application test") {
    std::vector<std::string> ok = {"a", "a.", "a. > @", "^.a", "^.^.a", "a.b", "(a.b c).d"};
    for(int i = 0; i < ok.size(); ++i) {
        TranslationUnit unit = makeUnit();
        Parser p(unit, ok[i].c_str());
        p.setTest(true);
        REQUIRE(p.isApplication(true));
        REQUIRE(p.isEOF());
    }
}

TEST_CASE("Abstraction test") {
    std::vector<std::string> ok = {"[x] > a", "[x]"};
    std::vector<std::string> fail = {""};
    testParserFunction(ok, &Parser::isAbstraction, true);
    testParserFunction(fail, &Parser::isAbstraction, false);
}

TEST_CASE("Attributes test") {
    std::vector<std::string> ok = {"[]", "[a]", "[a b]", "[a...]"};
    std::vector<std::string> fail = {"", "[", "[a  b]", "[a b ]"};
    testParserFunction(ok, &Parser::isAttributes, true);
    testParserFunction(fail, &Parser::isAttributes, false);
}

TEST_CASE("Suffix test") {
    std::vector<std::string> ok = {" > a", " > a!", " > @"};
    std::vector<std::string> fail = {"", " > "};
    testParserFunction(ok, &Parser::isSuffix, true);
    testParserFunction(fail, &Parser::isSuffix, false);
}

TEST_CASE("Name test") {
    std::vector<std::string> ok = {"a", "a546*&%#", "int"};
    std::vector<std::string> fail = {"", "A"};
    testParserFunction(ok, &Parser::isName, true);
    testParserFunction(fail, &Parser::isName, false);
}

TEST_CASE("Bytes test") {
    std::vector<std::string> ok = {"--", "A0-", "A0-A0-"};
    std::vector<std::string> fail = {"", "G0-", "A", "A0"};
    testParserFunction(ok, &Parser::isBytes, true);
    testParserFunction(fail, &Parser::isBytes, false);
}

TEST_CASE("String test") {
    std::vector<std::string> ok = {"\"\"", "\"abc\"", "\"\\n\"", "\"\\\"\""};
    std::vector<std::string> fail = {"", "\"\n\""};
    testParserFunction(ok, &Parser::isString, true);
    testParserFunction(fail, &Parser::isString, false);
}

TEST_CASE("Integer test") {
    std::vector<std::string> ok = {"1", "-1", "+1", "0x1a"};
    std::vector<std::string> fail = {"", "0x"};
    testParserFunction(ok, &Parser::isInteger, true);
    testParserFunction(fail, &Parser::isInteger, false);
}

TEST_CASE("Char test") {
    std::vector<std::string> ok = {"'a'", "'\\0'", "'\\\\'", "'\\''"};
    std::vector<std::string> fail = {"", "''", "'a"};
    testParserFunction(ok, &Parser::isChar, true);
    testParserFunction(fail, &Parser::isChar, false);
}

TEST_CASE("Regex test") {
    std::vector<std::string> ok = {"/a?/", "/a/g", "/\\//"};
    std::vector<std::string> fail = {"", "//"};
    testParserFunction(ok, &Parser::isRegex, true);
    testParserFunction(fail, &Parser::isRegex, false);
}

TEST_CASE("Float test") {
    std::vector<std::string> ok = {"1.0", "123.456", "-1.0", "+1.0", "1e10", "1e-10"};
    std::vector<std::string> fail = {"", "1", "1.", "1.0e"};
    testParserFunction(ok, &Parser::isFloat, true);
    testParserFunction(fail, &Parser::isFloat, false);
}

TEST_CASE("Bool test") {
    std::vector<std::string> ok = {"TRUE", "FALSE"};
    std::vector<std::string> fail = {"", "true", "false"};
    testParserFunction(ok, &Parser::isBool, true);
    testParserFunction(fail, &Parser::isBool, false);
}
