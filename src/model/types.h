#ifndef TYPES_H
#define TYPES_H

enum objectType {
    CLASS_TYPE,
    APPLICATION_TYPE,
    SEQUENCE_TYPE,
    REF_TYPE,
    VAR_TYPE,
    NAMED_ATTRIBUTE_TYPE,
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
    "named_attribute",
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

enum Flags {
    DOT_FLAG      = 0x01,
    VARARGS_FLAG  = 0x02,
    ATOM_FLAG     = 0x04,
    CLONE_FLAG    = 0x08,
    CONSTANT_FLAG = 0x10
};

#endif // TYPES_H
