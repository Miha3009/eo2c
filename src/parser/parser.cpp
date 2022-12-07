#include <iostream>
#include "parser.h"

#define DEBUG

void parse(const char* str, std::string path) {
    parser p(str, path);
    if(p.isProgram()) {
        std::cout << "OK" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
    }
    return;
}

parser::parser(const char* str, std::string path): str{str}, path{path},
    pos{0}, line{1}, column{1}
{
    int p = 0;
    spaces = {0, 0};
    while(str[p] != '\0') {
        if(str[p] == '\n') {
            ++p;
            int s = 0;
            while(str[p] == ' ') {
                ++s;
                ++p;
            }
            spaces.push_back(s);
        } else {
            ++p;
        }
    }
}

// program ::= [license] [metas] objects
bool parser::isProgram() {
    isLicense();
    isMetas();
    if(isObjects()) {
        goto _end;
    }
    return false;
_end:
#ifdef DEBUG
    debugMessage("This is program");
#endif
    return !hasError;
}

// license ::= {comment EOL}
bool parser::isLicense() {
    if(isComment()) {
        goto _1;
    }
    return false;
_1:
    if(isEOL()) {
        goto _end;
    }
    if(isComment()) {
        goto _1;
    }
    return errorMessage("End of line or comment expected");
_end:
#ifdef DEBUG
    debugMessage("This is license");
#endif
    return true;
}

// metas ::= {meta} EOL
bool parser::isMetas() {
    if(isMeta()){
        goto _1;
    }
    return false;
_1:
    if(isEOL()) {
        goto _end;
    }
    if(isMeta()) {
        goto _1;
    }
    return errorMessage("End of line or meta expected");
_end:
#ifdef DEBUG
    debugMessage("This is metas");
#endif
    return true;
}

// meta ::= '#' name [' ' ANY {ANY}] EOL
bool parser::isMeta() {
    if(isSymbol('+')){
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isName()) {
        goto _2;
    }
    return errorMessage("meta name expected");
_2:
    if(isSymbol(' ')) {
        next(1);
        goto _3;
    }
    goto _4;
_3:
    if(!isEOL()) {
        next(1);
        goto _3;
    }
    goto _end;
_4:
    if(isEOL()) {
        goto _end;
    }
    return errorMessage("End of line expected");
_end:
#ifdef DEBUG
    debugMessage("This is meta");
#endif
    return true;
}

// objects ::= object {EOL [object]}
bool parser::isObjects() {
    if(isObject()) {
        goto _1;
    }
    return false;
_1:
    if(isComment()) {
        goto _1;
    }
    if(isEOL()) {
        if(spaces[line] != 0) {
            return errorMessage("zero spaces expected");
        }
        goto _1;
    }
    goto _2;
_2:
    if(isEOF()) {
        goto _end;
    }
_3:
    if(isObject()){
        goto _1;
    }
    return errorMessage("Object or end of file expected");
_end:
#ifdef DEBUG
    debugMessage("This is objects");
#endif
    return true;
}

// object ::= (abstraction | application) details
bool parser::isObject() {
    Location l;
    if(isAbstraction()) {
        goto _1;
    }
    if(isApplication()) {
        goto _1;
    }
    return false;
_1:
    if(isTail()) {
        goto _2;
    }
    goto _2;
_2:
    storeLocation(l);
    if(isEOL()) {
        if(isMethod()) {
            goto _3;
        } else {
            restoreLocation(l);
            goto _end;
        }
    }
    goto _end;
_3:
    isHtail();
    isSuffix();
    isTail();
    goto _2;
_end:
#ifdef DEBUG
    debugMessage("This is object");
#endif
    return true;
}

// tail ::= EOL TAB {object EOL} UNTAB
bool parser::isTail() {
    Location l;
    storeLocation(l);
    if(isEOL()) {
        goto _1;
    }
    return false;
_1:
    if(spaces[line] == spaces[l.line]+2) {
#ifdef DEBUG
        debugMessage("This is tab");
#endif
        goto _2;
    }
    restoreLocation(l);
    return false;
_2:
    if(isObject()) {
        goto _4;
    }
    return errorMessage("object expected");
_3:
    if(isObject()) {
        goto _4;
    }
    goto _end;
_4:
    if(isEOL()) {
        goto _5;
    }
_5:
    if(spaces[line] == spaces[l.line]+2) {
        goto _3;
    }
    goto _end;
_end:
#ifdef DEBUG
    debugMessage("This is tail");
#endif
    return true;
}

bool parser::isMethod() {
    Location l;
    storeLocation(l);
    if(isSymbol('.')) {
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isName()) {
        goto _2;
    }
    if(isSymbol('&') || isSymbol('<') || isSymbol('^') || isSymbol('@')) {
        next(1);
        goto _2;
    }
    restoreLocation(l);
    return false;
_2:
    if(isSymbol('!')) {
        next(1);
        goto _end;
    }
    goto _end;
_end:
#ifdef DEBUG
    debugMessage("This is method");
#endif
    return true;
}

// abstraction ::= attributes [suffix]
bool parser::isAbstraction() {
_1:
    if(isComment()) {
        goto _1;
    }
    goto _2;
_2:
    if(isAttributes()) {
        goto _3;
    }
    return false;
_3:
    if(isSuffix()) {
        goto _4;
    }
    if(isHtail()) {
        goto _end;
    }
    goto _end;
_4:
    if(str[pos]   == ' ' &&
       str[pos+1] == '/') {
        goto _5;
    }
    goto _end;
_5:
    if(isName()) {
        goto _end;
    }
    if(isSymbol('?')) {
        next(1);
        goto _end;
    }
    return errorMessage("name or ? expected");
_end:
#ifdef DEBUG
    debugMessage("This is abstraction");
#endif
    return true;
}

// attributes ::= '[' (attribute {' ' attribute}) ']'
bool parser::isAttributes() {
    if(isSymbol('[')) {
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isSymbol(']')) {
        next(1);
        goto _end;
    }
    goto _2;
_2:
    if(isAttribute()){
        goto _3;
    }
    return errorMessage("attribute or ']' expected");;
_3:
    if(isSymbol(' ')) {
        next(1);
        goto _2;
    }
    if(isSymbol(']')) {
        next(1);
        goto _end;
    }
    return errorMessage("']' expected");
_end:
#ifdef DEBUG
    debugMessage("This is attributes");
#endif
    return true;
}

// attribute ::= '@' | name ['...']
bool parser::isAttribute() {
    std::string tmpValue = "";
    if(isSymbol('@')) {
        tmpValue += str[pos];
        next(1);
        goto _end;
    }
    if(isName()) {
        tmpValue += lexValue;
        goto _1;
    }
    return false;
_1:
    if(str[pos]   == '.' &&
       str[pos+1] == '.' &&
       str[pos+2] == '.') {
        tmpValue += "...";
        next(3);
        goto _end;
    }
    goto _end;
_end:
#ifdef DEBUG
    debugMessage("This is attribute");
#endif
    lexValue = tmpValue;
    return true;
}

// suffix ::= ' ' '>' ' ' ('@' | name) [!]
bool parser::isSuffix() {
    std::string tmpValue;
    if(str[pos]   == ' ' &&
       str[pos+1] == '>' &&
       str[pos+2] == ' ') {
        next(3);
        goto _1;
    }
    return false;
_1:
    if(isAttribute()) {
        goto _2;
    }
    return errorMessage("'@' or name expected");
_2:
    if(isSymbol('!')) {
        lexValue += "!";
        next(1);
    }
#ifdef DEBUG
    debugMessage("This is suffix");
#endif
    return true;
}

bool parser::isHas() {
    if(isSymbol(':')) {
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isSymbol('^')) {
        next(1);
        goto _end;
    }
    if(isName()) {
        goto _end;
    }
    return errorMessage("'^' or name expected");
_end:
#ifdef DEBUG
    debugMessage("This is has");
#endif
    return true;
}

// application ::= head [htail]
bool parser::isApplication() {
    if(isHead()) {
        goto _1;
    }
    if(isScope()) {
        goto _2;
    }
    return false;
_1:
    if(isHead()) {
        goto _1;
    }
    if(isScope()) {
        goto _2;
    }
    goto _2;
_2:
    if(isMethod()) {
        goto _2;
    }
    if(isHas()) {
        goto _2;
    }
    if(isSuffix()) {
        goto _2;
    }
    goto _3;
_3:
    if(isHtail()) {
        goto _end;
    }
    goto _end;
_end:
#ifdef DEBUG
    debugMessage("This is application");
#endif
    return true;
}

bool parser::isScope() {
    if(isSymbol('(')) {
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isApplication()) {
        goto _2;
    }
    return errorMessage("application expected");
_2:
    if(isSymbol(')')) {
        next(1);
        goto _end;
    }
    return errorMessage("')' expected");
_end:
#ifdef DEBUG
    debugMessage("This is scope");
#endif
    return true;
}

// htail ::= ' ' ('(' application ')' | application (ref | ':' name | suffix | ' ' application))
bool parser::isHtail() {
    if(isSuffix()) {
        goto _1;
    }
    if(isSymbol(' ')) {
        next(1);
        goto _2;
    }
    return false;
_1:
    if(isSuffix()) {
        goto _1;
    }
    if(isSymbol(' ')) {
        next(1);
        goto _2;
    }
    goto _end;
_2:
    if(isScope()) {
        goto _1;
    }
    if(isAbstraction()) {
        goto _1;
    }
    if(isMethod()) {
        goto _1;
    }
    if(isHas()) {
        goto _1;
    }
    if(isApplication()) {
        goto _1;
    }
    return errorMessage("htail expected");
_end:
#ifdef DEBUG
    debugMessage("This is htail");
#endif
    return true;
}

// head ::= ['...'] (name [''' | '.'] | data | '@' | '$' | '&' | '^' | '*')
bool parser::isHead() {
    std::string tmpValue = "";
    if(str[pos]   == '.' &&
       str[pos+1] == '.' &&
       str[pos+2] == '.'){
        tmpValue += "...";
        next(3);
    }
    if(isSymbol('Q')) {
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    if(isSymbol('@') || isSymbol('^') || isSymbol('$') || isSymbol('&') || isSymbol('*')){
        tmpValue = str[pos];
        next(1);
        goto _2;
    }
    if(isData()){
        tmpValue = lexValue;
        goto _end;
    }
    if(isName()){
        tmpValue += lexValue;
        goto _3;
    }
    if(isAbstraction()) {
        goto _end;
    }
    return false;
_1:
    if(isSymbol('Q')) {
        tmpValue += str[pos];
        next(1);
        goto _end;
    }
    goto _end;
_2:
    if(isSymbol('.')) {
        tmpValue += ".";
        next(1);
        goto _end;
    }
    goto _end;
_3:
    if(isSymbol('\'') || isSymbol('.')){
        tmpValue += str[pos];
        next(1);
        goto _end;
    }
    goto _end;
_end:
#ifdef DEBUG
    debugMessage("This is head");
#endif
    lexValue = tmpValue;
    return true;
}

// name ::= /[a-z]/ {ANY}
bool parser::isName() {
    if(isLetter()) {
        lexValue = str[pos];
        next(1);
        goto _1;
    }
    return false;
_1:
    while(!isSymbol(' ') &&
          !isSymbol('\n') &&
          !isSymbol('\r') &&
          !isSymbol('\0') &&
          !isSymbol(']') &&
          !isSymbol(')') &&
          !isSymbol('.')) {
        lexValue+= str[pos];
        next(1);
    }
#ifdef DEBUG
    debugMessage("This is name");
#endif
    return true;
}

// data ::= bytes | string | integer | char | float | regex
bool parser::isData() {
    return isBool()
    || isInteger()
    || isFloat()
    || isBytes()
    || isString()
    || isChar()
    || isRegex();
}

// bytes ::= byte {'-' byte} | '--'
// byte ::= /[\dA-F][\dA-F]/
bool parser::isBytes() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(str[pos] == '-' && str[pos+1] == '-') {
        tmpValue += "--";
        next(2);
        goto _end;
    }
    if(isHex()) {
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isHex()) {
        tmpValue += str[pos];
        next(1);
        goto _4;
    }
    restoreLocation(l);
    return false;
_2:
    if(isHex()) {
        tmpValue += str[pos];
        next(1);
        goto _3;
    }
    return errorMessage("Hex digit expected");
_3:
    if(isHex()) {
        tmpValue += str[pos];
        next(1);
        goto _4;
    }
    return errorMessage("Hex digit expected");
_4:
    if(isSymbol('-')){
        tmpValue += str[pos];
        next(1);
        goto _2;
    }
_end:
#ifdef DEBUG
    debugMessage("This is bytes");
#endif
    lexValue = tmpValue;
    return true;
}

// string ::= /"[^"]*"/
bool parser::isString() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('"')){
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    return false;
_1:
    while(!isSymbol('"')){
        if(isEOL()) {
            restoreLocation(l);
            return errorMessage("Unexpected end of line. '\"' expected");
        }
        if(isSymbol('\\')) {
            if(isEOL()) {
                return errorMessage("Unexpected end of line. '/' expected");
            }
            tmpValue += str[pos];
            next(1);
        }
        tmpValue += str[pos];
        next(1);
    }
    tmpValue += str[pos];
    next(1);
#ifdef DEBUG
    debugMessage("This is string");
#endif
    lexValue = tmpValue;
    return true;
}

// integer ::= /[+-]?\d+|0x[a-f\d]+/
bool parser::isInteger() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('+') || isSymbol('-')) {
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    if(isSymbol('0')) {
        tmpValue += str[pos];
        next(1);
        goto _2;
    }
_1:
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _5;
    }
    restoreLocation(l);
    return false;
_2:
    if(isSymbol('x')) {
        tmpValue += str[pos];
        next(1);
        goto _3;
    }
    goto _5;
_3:
    if(isDigit() || str[pos] >= 'a' && str[pos] <= 'f') {
        tmpValue += str[pos];
        next(1);
        goto _4;
    }
    restoreLocation(l);
    return errorMessage("Hex digits expected");
_4:
    if(isDigit() || str[pos] >= 'a' && str[pos] <= 'f') {
        tmpValue += str[pos];
        next(1);
        goto _4;
    }
    goto _end;
_5:
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _5;
    }
_end:
#ifdef DEBUG
    debugMessage("This is integer");
#endif
    lexValue = tmpValue;
    return true;
}

// char ::= /'\?[^']'/
bool parser::isChar() {
    std::string tmpValue = "";
    if(isSymbol('\'')) {
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isSymbol('\\')) {
        tmpValue += str[pos];
        next(1);
        goto _2;
    }
    if(isEOL()) return errorMessage("Unexpected end of line");
    if(!isSymbol('\'')){
        tmpValue += str[pos];
        next(1);
        goto _3;
    }
    return errorMessage("Empty char");
_2:
    if(isEOL()) return errorMessage("Unexpected end of line");
    tmpValue += str[pos];
    next(1);
    goto _3;
_3:
    if(isSymbol('\'')) {
        tmpValue += str[pos];
        next(1);
        goto _end;
    }
    return errorMessage("Symbol ' expected");
_end:
#ifdef DEBUG
    debugMessage("This is char");
#endif
    lexValue = tmpValue;
    return true;
}

// regex ::= //.+/[a-z]*/
bool parser::isRegex() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('/')) {
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isSymbol('/')) {
        restoreLocation(l);
        return false;
    }

    while(!isSymbol('/')) {
        if(isEOL()) {
            return errorMessage("Unexpected end of line. '/' expected");
        }
        if(isSymbol('\\')) {
            if(isEOL()) {
                return errorMessage("Unexpected end of line. '/' expected");
            }
            tmpValue += str[pos];
            next(1);
        }
        tmpValue += str[pos];
        next(1);
    }
    tmpValue += str[pos];
    next(1);
_2:
    if(isLetter()) {
        tmpValue += str[pos];
        next(1);
        goto _2;
    }
#ifdef DEBUG
    debugMessage("This is regex");
#endif
    lexValue = tmpValue;
    return true;
}

// float ::=  /[+-]?\d+(\.\d*)?/ [exp]
// exp ::= /e(+|-)?\d+/
bool parser::isFloat() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('+') || isSymbol('-')) {
        tmpValue += str[pos];
        next(1);
    }
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    restoreLocation(l);
    return false;
_1:
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    if (isSymbol('.')) {
        tmpValue += str[pos];
        next(1);
        goto _2;
    }
    if(isSymbol('e')) {
        tmpValue += str[pos];
        next(1);
        goto _3;
    }
    restoreLocation(l);
    return false;
_2:
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _2;
    }
    if(isSymbol('e')) {
        tmpValue += str[pos];
        next(1);
        goto _3;
    }
    goto _end;
_3:
    if(isSymbol('+') || isSymbol('-')) {
        tmpValue += str[pos];
        next(1);
    }
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _4;
    }
    restoreLocation(l);
    return errorMessage("Mantissa digits expected");
_4:
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _4;
    }
    goto _end;
_end:
#ifdef DEBUG
    debugMessage("This is float");
#endif
    lexValue = tmpValue;
    return true;
}

// bool ::= 'TRUE' | 'FALSE'
bool parser::isBool() {
    if (str[pos]   == 'T' &&
        str[pos+1] == 'R' &&
        str[pos+2] == 'U' &&
        str[pos+3] == 'E') {
        next(4);
        goto _end;
    }
    if (str[pos]   == 'F' &&
        str[pos+1] == 'A' &&
        str[pos+2] == 'L' &&
        str[pos+3] == 'S' &&
        str[pos+4] == 'E') {
        next(5);
        goto _end;
    }
    return false;
_end:
#ifdef DEBUG
    debugMessage("This is bool");
#endif
    return true;
}

// comment ::= '#' {ANY} EOL
bool parser::isComment() {
    if(isSymbol('#')) {
        next(1);
        goto _1;
    }
    return false;
_1:
    while(!isEOL()) next(1);
#ifdef DEBUG
    debugMessage("This is comment");
#endif
    return true;
}

// EOL ::= [\r] \n
bool parser::isEOL() {
    if(isSymbol('\r')) {
        next(1);
    }
    if(isSymbol('\n')) {
        pos++;
        line++;
        column = 1;
        goto _1;
    }
    return false;
_1:
    if(isSymbol('\0')) {
#ifdef DEBUG
        debugMessage("This is end of file");
#endif
        return true;
    }
    while(isSymbol(' ')) next(1);
#ifdef DEBUG
    debugMessage("This is EOL");
#endif
    return true;
}

void parser::ignore() {
_0:
    if(isSymbol(' ')
       || isSymbol('\t')
       || isSymbol('\r')) {
        next(1);
        goto _0;
    }
    if(isEOL()) {
        goto _0;
    }
    if(isComment()) {
        goto _0;
    }
}

bool parser::errorMessage(std::string&& messageText) {
    hasError = true;
    printMessage(std::move(messageText), "error");
    return false;
}

void parser::debugMessage(std::string&& messageText) {
    printMessage(std::move(messageText), "debug");
}

void parser::printMessage(std::string&& messageText, std::string&& messageType) {
    std::cout << path << ":"
         << line << ":"
         << column << ": " << messageType << ": "
         << messageText << "\n";
    int printPos = pos - column + 1;
    while(str[printPos] != '\n' && str[printPos] != '\0') {
        std::cout << str[printPos];
        ++printPos;
    }
    std::cout << "\n";
    for(int i = 1; i < column; ++i) {
        std::cout << " ";
    }
    std::cout << "^\n";
}
