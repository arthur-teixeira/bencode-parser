#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
typedef enum BencodeKind {
    BYTESTRING,
    INTEGER,
    LIST,
    DICTIONARY
} BencodeKind;

typedef struct BencodeType {
    BencodeKind kind;
    union {
        char *asString;
        signed long asInt;
        void *asList;
        void *asDict;
    };
} BencodeType;

BencodeType *parse(char *);

#endif // PARSER_H
