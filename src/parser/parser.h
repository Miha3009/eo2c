#ifndef PARSER_H
#define PARSER_H


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

    std::string curValue;
    bool hasError;
    int tabs;

    public:
        parser(const char* str, std::string path);

        bool isProgram();
        bool isMetas();
        bool isMeta();
        bool isObjects();
        bool isObject();
        bool isName();
        bool isComment();
        bool isDetails();
        bool isTail();
        bool isVtail();
        bool isAbstraction();
        bool isAttributes();
        bool isAttribute();
        bool isSuffix();
        bool isRef();
        bool isApplication();
        bool isHtail();
        bool isHead();
        bool isData();
        bool isBytes();
        bool isString();
        bool isInteger();
        bool isChar();
        bool isRegex();
        bool isFloat();
        bool isExp();
        bool isBool();
        bool isKeyword(std::string&& word);

        int getTabsCount();
        void ignore();
        bool errorMessage(std::string&& messageText);
        void printMessage(std::string&& messageText, std::string&& messageType);

        inline bool isSymbol(char ch) {
            return str[pos] == ch;
        }
        inline bool isEOL() {
            return str[pos] == '\n' || str[pos] == '\0';
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
        inline bool isAny() {
            return !isEOL();
        }
        inline void next() {
            pos++;
            column++;
        }
        inline void nextLine() {
            pos++;
            column = 1;
            line++;
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
