#ifndef TYPES_H
#define TYPES_H

enum objectType {
    CLASS_TYPE,
    APPLICATION_TYPE,
    REF_TYPE,
    VAR_TYPE,
    METHOD_TYPE,
    BOOL_TYPE,
    BYTES_TYPE,
    BYTE_TYPE,
    INT_TYPE,
    FLOAT_TYPE,
    TEXT_TYPE,
    STRING_TYPE,
    CHAR_TYPE,
    REGEX_TYPE
};

static const char* objectTypeNames[] {
    "class",
    "application",
    "ref",
    "var",
    "method",
    "bool",
    "bytes",
    "byte",
    "int",
    "float",
    "text",
    "string",
    "char",
    "regex"
};

#endif // TYPES_H
