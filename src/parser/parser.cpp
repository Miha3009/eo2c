#include <iostream>
#include "parser.h"

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
    pos{0}, line{1}, column{1}, tabs{0}, hasError{false}
{}

// program ::= [metas] objects
bool parser::isProgram() {
    ignore();
    isMetas();
    if(isObjects()) {
        goto _1;
    }
    return false;
_1:
    return !hasError;
}

// metas ::= {meta} EOL
bool parser::isMetas() {
    Location l;
    storeLocation(l);
_1:
    if(isMeta()){
        goto _1;
    }
    if(isEOL()) {
        nextLine();
        goto _2;
    }
    restoreLocation(l);
    return false;
_2:
    return true;
}

// meta ::= '#' name [' ' ANY {ANY}] EOL
bool parser::isMeta() {
    if(isSymbol('+')){
        next();
        goto _1;
    }
    return false;
_1:
    if(isName()) {
        goto _2;
    }
    return false;
_2:
    if(isSymbol(' ')) {
        next();
    }
    return false;
_3:
    while(isAny()) next();
    if(isEOL()) {
        nextLine();
        goto _5;
    }
    return false;
_5:
    return true;
}

// objects ::= object {EOL [object]}
bool parser::isObjects() {
    if(isObject()) {
        goto _1;
    }
    return false;
_1:
    if(isEOF()) {
        goto _3;
    }
    if(isEOL()) {
        nextLine();
        goto _2;
    }
_2:
    if(isObject()){
        goto _1;
    }
    goto _3;
_3:
    return true;
}

// name ::= /[a-z]/ {ANY}
bool parser::isName() {
    if(isLetter()) {
        curValue = str[pos];
        next();
        goto _1;
    }
    return false;
_1:
    if(!isSymbol(' ') && !isEOL() && !isSymbol(']')) {
        curValue += str[pos];
        next();
        goto _1;
    }
    return true;
}

// object ::= (abstraction | application) details
bool parser::isObject() {
    if(isAbstraction()) {
        goto _1;
    }
    if (isApplication()){
        goto _1;
    }
    return false;
_1:
    if(isDetails()){
        goto _2;
    }
    return false;
_2:
    return true;
}

// details ::= [tail] {vtail}
bool parser::isDetails() {
    if(isTail()) {
        goto _1;
    }
_1:
    if(isVtail()) {
        goto _1;
    }
    return true;
}

// tail ::= EOL TAB {object EOL} UNTAB
bool parser::isTail() {
    int tmpTabs;
    Location l;
    Location l2;
    storeLocation(l);
    if(isEOL()) {
        nextLine();
        goto _1;
    }
    return false;
_1:
    if(getTabsCount() == tabs+2) {
        tabs += 2;
        goto _2;
    }
    restoreLocation(l);
    return false;
_2:
    if(isObject()) {
        goto _3;
    }
    restoreLocation(l);
    return false;
_3:
    storeLocation(l2);
    if(isEOL()) {
        nextLine();
        goto _4;
    }
    goto _5;
_4:
    tmpTabs = getTabsCount();
    if(tmpTabs == tabs) {
        goto _2;
    }
    if(tmpTabs == tabs-2) {
        tabs -= 2;
        restoreLocation(l2);
        goto _5;
    }
    restoreLocation(l);
    return false;
_5:
    return true;
}

// vtail ::= EOL ref [htail] [suffix] [tail]
bool parser::isVtail() {
    Location l;
    storeLocation(l);
    if(isEOL()) {
        nextLine();
        goto _1;
    }
    return false;
_1:
    if(getTabsCount() == tabs) {
        goto _2;
    }
    restoreLocation(l);
    return false;
_2:
    if(isRef()) {
        goto _3;
    }
    restoreLocation(l);
    return false;
_3:
    isHtail();
    isSuffix();
    isTail();
    return true;
}

// abstraction ::= attributes [suffix]
bool parser::isAbstraction() {
    if(isAttributes()) {
        goto _1;
    }
    return false;
_1:
    isSuffix();
    return true;
}

// attributes ::= '[' attribute {' ' attribute} ']'
bool parser::isAttributes() {
    if(isSymbol('[')) {
        next();
        goto _1;
    }
    return false;
_1:
    if(isAttribute()){
        goto _2;
    }
    return false;
_2:
    if(isSymbol(' ')) {
        next();
        goto _1;
    }
    if(isSymbol(']')) {
        next();
        goto _3;
    }
    return false;
_3:
    return true;
}

// attribute ::= '@' | name ['...']
bool parser::isAttribute() {
    std::string tmpValue = "";
    if(isSymbol('@')) {
        tmpValue += str[pos];
        next();
        goto _2;
    }
    if(isName()) {
        tmpValue += curValue;
        goto _1;
    }
    return false;
_1:
    if(isKeyword("...")) {
        tmpValue += "...";
        goto _2;
    }
    goto _2;
_2:
    curValue = tmpValue;
    return true;
}

// suffix ::= ' ' '>' ' ' ('@' | name) [!]
bool parser::isSuffix() {
    if(isKeyword(" > ")) {
        goto _1;
    }
    return false;
_1:
    if(isSymbol('@')) {
        next();
        goto _2;
    }
    if(isName()) {
        goto _2;
    }
    return false;
_2:
    if(isSymbol('!')) {
        next();
    }
    return true;
}

// ref ::= '.' (name | '^' | '@' | '<')
bool parser::isRef() {
    if(isSymbol('.')) {
        next();
        goto _1;
    }
    return false;
_1:
    if(isName()) {
        goto _2;
    }
    if(isSymbol('^') || isSymbol('@') || isSymbol('<')) {
        next();
        goto _2;
    }
    return false;
_2:
    return true;
}

// application ::= head [htail]
bool parser::isApplication() {
    if(isHead()) {
        goto _1;
    }
    return false;
_1:
    isHtail();
    return true;
}

// htail ::= '(' application ')' | application (ref | ':' name | suffix | ' ' application)
bool parser::isHtail() {
    if(isSymbol('(')) {
        next();
        goto _1;
    }
    if(isApplication()) {
        goto _3;
    }
    return false;
_1:
    if(isApplication()) {
        goto _2;
    }
    return false;
_2:
    if(isSymbol(')')) {
        next();
        goto _6;
    }
    return false;
_3:
    if(isRef()) {
        goto _6;
    }
    if(isSymbol(':')) {
        next();
        goto _4;
    }
    if(isSuffix()) {
        goto _6;
    }
    if(isSymbol(' ')) {
        next();
        goto _5;
    }
    return false;
_4:
    if(isName()) {
        goto _6;
    }
    return false;
_5:
    if(isApplication()) {
        goto _6;
    }
    return false;
_6:
    return true;
}

// head ::= ['...'] (name [''' | '.'] | data | '@' | '$' | '&' | '^' | '*')
bool parser::isHead() {
    std::string tmpValue = "";
    if(isKeyword("...")){
        tmpValue += "...";
    }
    if(isSymbol('@') || isSymbol('^') || isSymbol('$') || isSymbol('&') || isSymbol('*')){
        tmpValue = str[pos];
        next();
        goto _1;
    }
    if(isData()){
        tmpValue = curValue;
        goto _1;
    }
    if(isName()){
        tmpValue += curValue;
    }
    if(isSymbol('\'')){
        tmpValue += str[pos];
        next();
        goto _1;
    }
    if(isSymbol('.')){
        tmpValue += str[pos];
        next();
        goto _1;
    }
    return false;
_1:
    curValue = tmpValue;
    return true;
}

// data ::= bytes | string | integer | char | float | regex
bool parser::isData() {
    return isBytes()
    || isString()
    || isInteger()
    || isChar()
    || isFloat()
    || isRegex();
}

// bytes ::= byte {'-' byte}
// byte ::= /[\dA-F][\dA-F]/
bool parser::isBytes() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
_0:
    if(isDigit() || (str[pos] >= 'A' && str[pos] <= 'F')) {
        tmpValue += str[pos];
        next();
        goto _1;
    }
    restoreLocation(l);
    return false;
_1:
    if(isDigit() || (str[pos] >= 'A' && str[pos] <= 'F')) {
        tmpValue += str[pos];
        next();
        goto _2;
    }
    restoreLocation(l);
    return false;
_2:
    if(isSymbol('-')){
        tmpValue += str[pos];
        next();
        goto _0;
    }
    curValue = tmpValue;
    return true;
}

// string ::= /"[^"]*"/
bool parser::isString() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('"')){
        tmpValue += str[pos];
        next();
        goto _1;
    }
    return false;
_1:
    while(!isSymbol('"')){
        if(isEOL()) {
            restoreLocation(l);
            return false;
        }
        tmpValue += str[pos];
        next();
    }
    tmpValue += str[pos];
    next();
    curValue = tmpValue;
    return true;
}

// integer ::= /[+-]?\d+|0x[a-f\d]+/
bool parser::isInteger() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('+') || isSymbol('-')) {
        tmpValue += str[pos];
        next();
        goto _1;
    }
    if(isSymbol('0')) {
        tmpValue += str[pos];
        next();
        goto _2;
    }
_1:
    if(isDigit()) {
        tmpValue += str[pos];
        next();
        goto _3;
    }
    restoreLocation(l);
    return false;
_2:
    if(isSymbol('x')) {
        tmpValue += str[pos];
        next();
        goto _3;
    }
    goto _5;
_3:
    if(isDigit() || str[pos] >= 'a' && str[pos] <= 'f') {
        tmpValue += str[pos];
        next();
        goto _4;
    }
    restoreLocation(l);
    return false;
_4:
    if(isDigit() || str[pos] >= 'a' && str[pos] <= 'f') {
        tmpValue += str[pos];
        next();
        goto _4;
    }
    goto _6;
_5:
    if(isDigit()) {
        tmpValue += str[pos];
        next();
        goto _5;
    }
_6:
    curValue = tmpValue;
    return true;
}

// char ::= /'[^']*|\\\d'/
bool parser::isChar() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('\'')) {
        tmpValue += str[pos];
        next();
        goto _1;
    }
    return false;
_1:
    if(isSymbol('\\')) {
        tmpValue += str[pos];
        next();
        goto _2;
    }
    if(!isSymbol('\'') && !isEOF()){
        tmpValue += str[pos];
        next();
        goto _3;
    }
    restoreLocation(l);
    return false;
_2:
    if(isDigit()) {
        tmpValue += str[pos];
        next();
        goto _3;
    }
    restoreLocation(l);
    return false;
_3:
    if(isSymbol('\'')) {
        tmpValue += str[pos];
        next();
        goto _4;
    }
    restoreLocation(l);
    return false;
_4:
    curValue = tmpValue;
    return true;
}

// regex ::= //.+/[a-z]*/
bool parser::isRegex() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('/')) {
        tmpValue += str[pos];
        next();
        goto _1;
    }
    return false;
_1:
    if(!isEOL() && !isSymbol('/')) {
        tmpValue += str[pos];
        next();
        goto _2;
    }
_2:
    if(!isEOL() && !isSymbol('/')) {
        tmpValue += str[pos];
        next();
        goto _2;
    }
    if(isSymbol('/')) {
        tmpValue += str[pos];
        next();
        goto _3;
    }
    restoreLocation(l);
    return false;
_3:
    if(isLetter()) {
        tmpValue += str[pos];
        next();
        goto _3;
    }
    curValue = tmpValue;
    return true;
}

// float ::=  /[+-]?\d+(\.\d+)?/ [exp]
bool parser::isFloat() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('+') || isSymbol('-')) {
        tmpValue += str[pos];
        next();
    }
    if(isDigit()) {
        tmpValue += str[pos];
        next();
        goto _1;
    }
    restoreLocation(l);
    return false;
_1:
    if(isDigit()) {
        tmpValue += str[pos];
        next();
        goto _1;
    }
    if (isSymbol('.')) {
        tmpValue += str[pos];
        goto _2;
    }
    return false;
    restoreLocation(l);
_2:
    if(isDigit()) {
        tmpValue += str[pos];
        next();
        goto _2;
    }
    if(isExp()) {
        tmpValue += curValue;
    }
    curValue = tmpValue;
    return true;
}

// exp ::= /e(+|-)?\d+/
bool parser::isExp() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('e')) {
        tmpValue += str[pos];
        next();
        goto _1;
    }
    return false;
_1:
    if(isSymbol('+') || isSymbol('-')) {
        tmpValue += str[pos];
        next();
    }
    if(isDigit()) {
        tmpValue += str[pos];
        next();
        goto _2;
    }
    restoreLocation(l);
    return false;
_2:
    if(isDigit()) {
        tmpValue += str[pos];
        next();
        goto _2;
    }
    curValue = tmpValue;
    return true;
}

// bool ::= 'TRUE' | 'FALSE'
bool parser::isBool() {
    return isKeyword("TRUE") || isKeyword("FALSE");
}

bool parser::isKeyword(std::string&& word) {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    for(int i = 0; i < word.size(); ++i){
        if(isSymbol(word[i])) {
            tmpValue += str[pos];
            next();
        } else {
            restoreLocation(l);
            return false;
        }
    }
    curValue = tmpValue;
    return true;
}

// comment ::= '#' {ANY} EOL
bool parser::isComment() {
    if(isSymbol('#')) {
        next();
    } else {
        return false;
    }
    while(!isEOL()) next();
    nextLine();
    return true;
}

int parser::getTabsCount() {
    int c = 0;
    while(isSymbol(' ')) {
        next();
        ++c;
    }
    return c;
}

void parser::ignore() {
_0:
    if(isSymbol(' ')
       || isSymbol('\t')
       || isSymbol('\r')) {
        next();
        goto _0;
    }
    if(isEOL()) {
        nextLine();
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

void parser::printMessage(std::string&& messageText, std::string&& messageType) {
    std::cout << path << ":"
         << line << ":"
         << column << ": " << messageType << ": "
         << messageText << "\n";
    int printPos = pos - column + 1;
    while(str[printPos] != '\n') {
        std::cout << str[printPos];
        ++printPos;
    }
    std::cout << "\n";
    for(int i = 1; i < column; ++i) {
        std::cout << " ";
    }
    std::cout << "^\n";
}
