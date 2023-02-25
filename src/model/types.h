#ifndef TYPES_H
#define TYPES_H

enum objectType {
    CLASS_TYPE,
    APPLICATION_TYPE,
    SEQUENCE_TYPE,
    REF_TYPE,
    VAR_TYPE,
    BOOL_TYPE,
    BYTES_TYPE,
    BYTE_TYPE,
    INT_TYPE,
    FLOAT_TYPE,
    TEXT_TYPE,
    STRING_TYPE,
    CHAR_TYPE,
    REGEX_TYPE,
    ARRAY_TYPE
};

static const char* objectTypeNames[] {
    "class",
    "application",
    "sequence",
    "ref",
    "var",
    "bool",
    "bytes",
    "byte",
    "int",
    "float",
    "text",
    "string",
    "char",
    "regex",
    "array"
};

#endif // TYPES_H
