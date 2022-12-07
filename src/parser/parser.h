#ifndef PARSER_H
#define PARSER_H

#include <vector>

void parse(const char* str, std::string path);

struct Location {
    int pos;
    int line;
    int column;
};

class parser
{
    const char* str = nullptr;
    std::string path;

    int pos;
    int line;
    int column;

    bool hasError = false;
    std::string lexValue;
    std::vector<int> spaces;

    public:
        parser(const char* str, std::string path);

        bool isProgram();
        bool isLicense();
        bool isMetas();
        bool isMeta();
        bool isObjects();
        bool isObject();
        bool isName();
        bool isComment();
        bool isTail();
        bool isMethod();
        bool isAbstraction();
        bool isAttributes();
        bool isAttribute();
        bool isSuffix();
        bool isRef();
        bool isHas();
        bool isApplication();
        bool isScope();
        bool isHtail();
        bool isHead();
        bool isData();
        bool isBytes();
        bool isString();
        bool isInteger();
        bool isChar();
        bool isRegex();
        bool isFloat();
        bool isBool();
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
        inline bool isHex() {
            return isDigit() || (str[pos] >= 'A' && str[pos] <= 'F');
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
