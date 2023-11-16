#ifndef PARSER_H
#define PARSER_H

#include "stb_hashtable.h"
#include <stddef.h>
#include <stdio.h>
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

BencodeType parse_item(char *input, char **end_ptr);

BencodeList parse(char *input);

typedef enum {
    LIST_START,
    DICT_START,
    INT_START,
    INT,
    END,
    STRING_SIZE,
    STRING,
    COLON,
    ILLEGAL,
} TokenType;

typedef struct {
    TokenType type;
    union {
        char *asString;
        long asInt;
    };
} Token;

typedef struct {
    FILE *input;
    char *buf;
    size_t bufsize;
    size_t pos;
    size_t read_pos;
    char ch;
    Token prevprev;
    Token prev;
} Lexer;

void open_stream(Lexer *l, const char *filename);
Token next_token(Lexer *l);

#endif // PARSER_H
