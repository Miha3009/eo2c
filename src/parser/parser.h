#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "object.h"
#include "translation_unit.h"

struct Location {
    int pos;
    int line;
    int column;
};

class Parser {
    const char* str = nullptr;
    std::vector<int> spaces;

    int pos;
    int line;
    int column;

    bool hasError = false;
    std::string lexValue;
    objectType dataType;
    Object* curObj;
    TranslationUnit& unit;

public:
    Parser(TranslationUnit& unit, const char* str);

    bool isProgram();
    bool isLicense();
    bool isMetas();
    bool isMeta();
    bool isObjects();
    bool isObject();
    bool isName();
    bool isTail();
    bool isMethod();
    bool isAbstraction();
    bool isAttributes();
    bool isAttribute();
    bool isSuffix();
    bool isRef();
    bool isHas();
    bool isApplication(bool parseHtail);
    bool isScope();
    bool isHtail();
    bool isHead();
    bool isData();
    bool isBytes();
    bool isByte();
    bool isText();
    bool isString();
    bool isInteger();
    bool isChar();
    bool isRegex();
    bool isFloat();
    bool isBool();
    bool isComment();
    bool isEscapeSequence();
    bool isEOL();

    void ignore();
    bool errorMessage(std::string&& messageText);
    void debugMessage(std::string&& messageText);
    void printMessage(std::string&& messageText, std::string&& messageType);

    inline bool isSymbol(char ch) {
        return str[pos] == ch;
    }
    inline bool isEOF() {
        return str[pos] == '\0';
    }
    inline bool isLetter() {
        return str[pos] >= 'a' && str[pos] <= 'z';
    }
    inline bool isDigit() {
        return str[pos] >= '0' && str[pos] <= '9';
    }
    inline void next(int step) {
        pos += step;
        column += step;
    }
    inline void storeLocation(Location& l) {
        l.pos = pos;
        l.line = line;
        l.column = column;
    }
    inline void restoreLocation(Location& l) {
        pos = l.pos;
        line = l.line;
        column = l.column;
    }
};

#endif // PARSER_H
