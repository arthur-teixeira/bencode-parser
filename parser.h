#ifndef PARSER_H
#define PARSER_H

#include "stb_hashtable.h"
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum BencodeKind {
    BYTESTRING,
    INTEGER,
    LIST,
    DICTIONARY,
    ERROR,
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
        BencodeList asList;
        hash_table_t asDict;
    };
} BencodeType;

typedef enum {
    LIST_START,
    DICT_START,
    INT_START,
    INT,
    END,
    STRING_SIZE,
    STRING,
    COLON,
    END_OF_FILE,
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

typedef struct {
    Lexer l;
    Token cur_token;
    Token peek_token;
    char *errors[500];
    size_t error_index;
} Parser;

void open_stream(Lexer *l, const char *filename);
Token next_token(Lexer *l);
BencodeType parse_item(Parser *p);
BencodeList parse(Parser *p);
void parser_next_token(Parser *p);
Parser new_parser(Lexer l);
bool expect_peek(Parser *p, TokenType expected);
void parse_error(Parser *p, char *error);
Lexer new_lexer(char *filename);

#endif // PARSER_H
