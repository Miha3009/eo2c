#include <iostream>
#include "parser.h"

Parser::Parser(TranslationUnit& unit, const char* str): unit{unit}, str{str}, pos{0}, line{1}, column{1} {
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
bool Parser::isProgram() {
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
bool Parser::isLicense() {
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
bool Parser::isMetas() {
    if(isMeta()) {
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
bool Parser::isMeta() {
    std::string type;
    if(isSymbol('+')) {
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isName()) {
        type = lexValue;
        lexValue = "";
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
        lexValue += str[pos];
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
    unit.metas.push_back({type, lexValue});
    return true;
}

// objects ::= object {EOL [object]}
bool Parser::isObjects() {
    curObj = unit.root;
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
    curObj = unit.root;
    if(isObject()) {
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
bool Parser::isObject() {
    Location l;
    Object* tmpObj = curObj;
    curObj = curObj->makeChild(CLASS_TYPE);
    if(isAbstraction()) {
        goto _1;
    }
    if(isApplication(true)) {
        goto _1;
    }
    delete curObj;
    curObj = tmpObj;
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
    curObj = tmpObj;
    return true;
}

// tail ::= EOL TAB {object EOL} UNTAB
bool Parser::isTail() {
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
    if(spaces[line] == 0 && isEOL()) {
        goto _1;
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
        goto _4;
    }
    goto _5;
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

bool Parser::isMethod() {
    Location l;
    storeLocation(l);
    if(str[pos-1] == '.') {
        goto _1;
    }
    if(isSymbol('.')) {
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isName()) {
        curObj->addToSequence(VAR_TYPE, lexValue);
        goto _2;
    }
    if(isSymbol('&') || isSymbol('<') || isSymbol('^') || isSymbol('@')) {
        curObj->addToSequence(REF_TYPE, std::to_string(str[pos]));
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
bool Parser::isAbstraction() {
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
        next(2);
        goto _5;
    }
    goto _end;
_5:
    if(isName()) {
        curObj->addFlags(ATOM_FLAG);
        goto _end;
    }
    if(isSymbol('?')) {
        curObj->addFlags(ATOM_FLAG);
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
bool Parser::isAttributes() {
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
    if(isAttribute()) {
        curObj->addAttribute(lexValue);
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
bool Parser::isAttribute() {
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
bool Parser::isSuffix() {
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
        if(curObj->getType() == CLASS_TYPE) {
            curObj->setOriginValue(lexValue);
        } else {
            Object* tmpObj = curObj;
            while(tmpObj->getParent()->getType() != CLASS_TYPE) {
                tmpObj = tmpObj->getParent();
            }
            tmpObj->setOriginValue(lexValue);
        }
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

bool Parser::isHas() {
    if(isSymbol(':')) {
        curObj = curObj->makeParent(NAMED_ATTRIBUTE_TYPE);
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
        curObj->setOriginValue(lexValue);
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
bool Parser::isApplication(bool parseHtail) {
    Object* tmpObj = curObj;
    if(parseHtail) {
        curObj->setType(APPLICATION_TYPE);
        curObj = curObj->makeChild(APPLICATION_TYPE);
    }
    if(isHead()) {
        curObj->setType(dataType);
        curObj->setOriginValue(lexValue);
        goto _1;
    }
    if(isScope()) {
        goto _2;
    }
    delete curObj;
    curObj = tmpObj;
    return false;
_1:
    if(isHead()) {
        curObj->addToSequence(dataType, lexValue);
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
    if(parseHtail) {
        curObj = tmpObj;
        if(isHtail()) {
            goto _end;
        }
    }
    goto _end;
_end:
#ifdef DEBUG
    debugMessage("This is application");
#endif
    curObj = tmpObj;
    return true;
}

bool Parser::isScope() {
    Object* tmpObj = curObj;
    if(isSymbol('(')) {
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isApplication(true)) {
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
    curObj = tmpObj;
    return true;
}

// htail ::= ' ' ('(' application ')' | application (ref | ':' name | suffix | ' ' application))
bool Parser::isHtail() {
    Object* tmpObj = curObj;
    if(isSymbol(' ')) {
        next(1);
        goto _1;
    }
    return false;
_1:
    curObj = curObj->makeChild(CLASS_TYPE);
    if(isApplication(false)) {
        goto _2;
    }
    if(isScope()) {
        goto _2;
    }
    if(isAbstraction()) {
        goto _2;
    }
    if(isMethod()) {
        goto _2;
    }
    if(isHas()) {
        goto _2;
    }
    return errorMessage("htail expected");
_2:
    curObj = tmpObj;
    if(isSymbol(' ')) {
        next(1);
        goto _1;
    }
    goto _end;
_end:
#ifdef DEBUG
    debugMessage("This is htail");
#endif
    curObj = tmpObj;
    return true;
}

// head ::= ['...'] (name [''' | '.'] | data | '@' | '$' | '&' | '^' | '*')
bool Parser::isHead() {
    std::string tmpValue = "";
    if(str[pos]   == '.' &&
            str[pos+1] == '.' &&
            str[pos+2] == '.') {
        tmpValue += "...";
        next(3);
    }
    if(isSymbol('Q')) {
        dataType = REF_TYPE;
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    if(isSymbol('@') || isSymbol('^') || isSymbol('$') || isSymbol('&') || isSymbol('<')) {
        dataType = REF_TYPE;
        tmpValue = str[pos];
        next(1);
        goto _2;
    }
    if(isSymbol('*')) {
        dataType = ARRAY_TYPE;
        next(1);
        goto _2;
    }
    if(isData()) {
        tmpValue = lexValue;
        goto _end;
    }
    if(isName()) {
        dataType = VAR_TYPE;
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
        next(1);
        goto _end;
    }
    goto _end;
_3:
    if(isSymbol('\'') || isSymbol('.')) {
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
bool Parser::isName() {
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
            !isSymbol(':') &&
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
bool Parser::isData() {
    return isBool()
           || isFloat()
           || isBytes()
           || isInteger()
           || isText()
           || isString()
           || isChar()
           || isRegex();
}

// bytes ::= byte {'-' byte} | '--'
bool Parser::isBytes() {
    Location l;
    storeLocation(l);
    if(str[pos] == '-' && str[pos+1] == '-') {
        Object* tmpObj = curObj->makeChild(BYTE_TYPE);
        tmpObj->setOriginValue("00");
        next(2);
        goto _end;
    }
    if(isByte()) {
        goto _1;
    }
    return false;
_1:
    if(isSymbol('-')) {
        Object* tmpObj = curObj->makeChild(BYTE_TYPE);
        tmpObj->setOriginValue(lexValue);
        next(1);
        goto _2;
    }
    restoreLocation(l);
    return false;
_2:
    if(isByte()) {
        goto _3;
    }
    goto _end;
_3:
    if(isSymbol('-')) {
        Object* tmpObj = curObj->makeChild(BYTE_TYPE);
        tmpObj->setOriginValue(lexValue);
        next(1);
        goto _2;
    }
_end:
#ifdef DEBUG
    debugMessage("This is bytes");
#endif
    lexValue = "";
    dataType = BYTES_TYPE;
    return true;
}

// byte ::= /[\dA-F][\dA-F]/
bool Parser::isByte() {
    if(((str[pos] >= '0' && str[pos] <= '9') || (str[pos] >= 'A' && str[pos] <= 'F')) &&
            ((str[pos+1] >= '0' && str[pos+1] <= '9') || (str[pos+1] >= 'A' && str[pos+1] <= 'F'))) {
        lexValue = std::string({(char)std::tolower(str[pos]), (char)std::tolower(str[pos+1])});
        next(2);
        return true;
    }
    return false;
}

bool Parser::isText() {
    int tmpSpaces = spaces[line];
    std::string tmpValue = "";
    if(str[pos]   == '"' &&
            str[pos+1] == '"' &&
            str[pos+2] == '"') {
        tmpValue += "\"\"\"";
        next(3);
        goto _1;
    }
    return false;
_1:
    if(str[pos]   == '"' &&
            str[pos+1] == '"' &&
            str[pos+2] == '"') {
        tmpValue += "\"\"\"";
        next(3);
        goto _end;
    }
    if(isEscapeSequence()) {
        tmpValue += lexValue;
        goto _1;
    }
    if(str[pos] == '\r') {
        next(1);
        goto _1;
    }
    if(str[pos] == '\n') {
        pos++;
        line++;
        column = 1;
        for(int i = 0; i < tmpSpaces; ++i) {
            if(str[pos] == ' ') next(1);
        }
        goto _1;
    }
    if(isEOF()) {
        return errorMessage("\"\"\" expected");
    }
    tmpValue += str[pos];
    next(1);
    goto _1;
_end:
#ifdef DEBUG
    debugMessage("This is text");
#endif
    lexValue = tmpValue;
    dataType = TEXT_TYPE;
    return true;
}

// string ::= /"[^"]*"/
bool Parser::isString() {
    Location l;
    storeLocation(l);
    std::string tmpValue = "";
    if(isSymbol('"')) {
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    return false;
_1:
    while(!isSymbol('"')) {
        if(isEOL()) {
            restoreLocation(l);
            return errorMessage("Unexpected end of line. '\"' expected");
        }
        if(isEscapeSequence()) {
            tmpValue += lexValue;
            continue;
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
    dataType = STRING_TYPE;
    return true;
}

// integer ::= /[+-]?\d+|0x[a-f\d]+/
bool Parser::isInteger() {
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
    dataType = INT_TYPE;
    return true;
}

// char ::= /'\?[^']'/
bool Parser::isChar() {
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
    if(!isSymbol('\'')) {
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
    dataType = CHAR_TYPE;
    return true;
}

// regex ::= //.+/[a-z]*/
bool Parser::isRegex() {
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
        if(isEscapeSequence()) {
            tmpValue += lexValue;
            continue;
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
    dataType = REGEX_TYPE;
    return true;
}

// float ::=  /[+-]?\d+(\.\d*)?/ [exp]
// exp ::= /e(+|-)?\d+/
bool Parser::isFloat() {
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
        goto _5;
    }
    restoreLocation(l);
    return false;
_2:
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _3;
    }
    restoreLocation(l);
    return false;
_3:
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _3;
    }
    goto _4;
_4:
    if(isSymbol('e')) {
        tmpValue += str[pos];
        next(1);
        goto _5;
    }
    goto _end;
_5:
    if(isSymbol('+') || isSymbol('-')) {
        tmpValue += str[pos];
        next(1);
    }
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _6;
    }
    restoreLocation(l);
    return false;
_6:
    if(isDigit()) {
        tmpValue += str[pos];
        next(1);
        goto _6;
    }
    goto _end;
_end:
#ifdef DEBUG
    debugMessage("This is float");
#endif
    lexValue = tmpValue;
    dataType = FLOAT_TYPE;
    return true;
}

// bool ::= 'TRUE' | 'FALSE'
bool Parser::isBool() {
    if (str[pos]   == 'T' &&
            str[pos+1] == 'R' &&
            str[pos+2] == 'U' &&
            str[pos+3] == 'E') {
        next(4);
        lexValue = "true";
        goto _end;
    }
    if (str[pos]   == 'F' &&
            str[pos+1] == 'A' &&
            str[pos+2] == 'L' &&
            str[pos+3] == 'S' &&
            str[pos+4] == 'E') {
        next(5);
        lexValue = "false";
        goto _end;
    }
    return false;
_end:
#ifdef DEBUG
    debugMessage("This is bool");
#endif
    dataType = BOOL_TYPE;
    return true;
}

// comment ::= '#' {ANY} EOL
bool Parser::isComment() {
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

bool Parser::isEscapeSequence() {
    std::string tmpValue = "";
    if(isSymbol('\\')) {
        tmpValue += str[pos];
        next(1);
        goto _1;
    }
    return false;
_1:
    if(isSymbol('b') || isSymbol('t') || isSymbol('n') || isSymbol('f') ||
            isSymbol('r') || isSymbol('\'') || isSymbol('"') || isSymbol('\\')) {
        tmpValue += str[pos];
        next(1);
        goto _end;
    }
    if(str[pos] >= '0' && str[pos] <= '7') {
        tmpValue += str[pos];
        next(1);
        goto _end;
    }
    if(isSymbol('u')) {
        tmpValue += str[pos];
        next(1);
        goto _2;
    }
    return errorMessage("escape sequence expected");
_2:
    if(isByte()) {
        tmpValue += lexValue;
        goto _3;
    }
    return errorMessage("hex number of unicode character expected");
_3:
    if(isByte()) {
        tmpValue += lexValue;
        goto _end;
    }
    return errorMessage("hex number of unicode character expected");
_end:
#ifdef DEBUG
    debugMessage("This is escape sequence");
#endif
    lexValue = tmpValue;
    return true;
}

// EOL ::= [\r] \n
bool Parser::isEOL() {
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

bool Parser::errorMessage(std::string&& messageText) {
    hasError = true;
    printMessage(std::move(messageText), "error");
    return false;
}

void Parser::debugMessage(std::string&& messageText) {
    printMessage(std::move(messageText), "debug");
}

void Parser::printMessage(std::string&& messageText, std::string&& messageType) {
    std::cout << unit.source.string() << ":"
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
