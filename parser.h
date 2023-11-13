#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
typedef enum BencodeKind {
    BYTESTRING,
    INTEGER,
    LIST,
    DICTIONARY
} BencodeKind;

typedef struct BencodeList {
    size_t len;
    size_t cap;
    struct BencodeType *values;
} BencodeList;

typedef struct BencodeType {
    BencodeKind kind;
    union {
        char *asString;
        signed long asInt;
        struct BencodeList asList;
        void *asDict;
    };
} BencodeType;

BencodeType parse(char *);
BencodeType parse_ex(char *input, char **end_ptr);

#endif // PARSER_H
