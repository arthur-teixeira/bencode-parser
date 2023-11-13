#ifndef PARSER_H
#define PARSER_H

#include "stb_hashtable.h"
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

typedef struct BencodeDict {
    hash_table_t table; // hash_table_t<BencodeType>;
} BencodeDict;

typedef struct BencodeType {
    BencodeKind kind;
    union {
        char *asString;
        signed long asInt;
        BencodeList asList;
        BencodeDict asDict;
    };
} BencodeType;

BencodeType parse(char *input, char **end_ptr);

BencodeList parse_stream(char *input);

#endif // PARSER_H
