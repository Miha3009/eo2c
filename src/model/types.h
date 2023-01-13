//#ifndef TYPES_H
//#define TYPES_H

enum objectType {
    CLASS_TYPE,
    REF_TYPE,
    VAR_TYPE,
    SCOPE_TYPE,
    HTAIL_TYPE,
    BOOL_TYPE,
    BYTES_TYPE,
    INT_TYPE,
    FLOAT_TYPE,
    TEXT_TYPE,
    STRING_TYPE,
    CHAR_TYPE,
    REGEX_TYPE
};

static const char* objectTypeNames[] {
    "class",
    "ref",
    "var",
    "scope",
    "htail",
    "bool",
    "bytes",
    "int",
    "float",
    "text",
    "string",
    "char",
    "regex"
};

//#endif // TYPES_H
