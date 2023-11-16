#include "parser.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_IMPLEMENTATION
#include "stb_hashtable.h"

#define da_init(da, size)                                                      \
  do {                                                                         \
    da->cap = 16;                                                              \
    da->values = calloc(da->cap, size);                                        \
    da->len = 0;                                                               \
  } while (0);

#define da_append(da, value)                                                   \
  do {                                                                         \
    if (da->len == da->cap) {                                                  \
      da->cap *= 2;                                                            \
      da->values = realloc(da->values, da->cap * sizeof(da->values[0]));       \
    }                                                                          \
    da->values[da->len++] = value;                                             \
  } while (0);

signed long parse_integer(char *input, char **end_ptr) {
  char buf[strlen(input)];

  size_t i = 1;
  while (input[i] != 'e') {
    buf[i - 1] = input[i];
    i++;
  }

  if (end_ptr) {
    *end_ptr = &input[i + 1];
  }

  return strtol(buf, NULL, 10);
}

char *parse_bytestring(char *input, char **end_ptr) {
  size_t n = atoi((char[]){input[0]});

  char *out = calloc(n, sizeof(char));

  size_t i = 2;
  for (; i < n + 2; i++) {
    out[i - 2] = input[i];
  }

  if (end_ptr) {
    *end_ptr = &input[i];
  }

  return out;
}

BencodeList parse_list(char *input, char **end_ptr_) {
  BencodeList l;

  char *end_ptr = &input[1];

  BencodeList *lp = &l;
  da_init(lp, sizeof(BencodeType));

  while (end_ptr[0] != 'e') {
    da_append(lp, parse_item(end_ptr, &end_ptr));
  }

  if (end_ptr_) {
    *end_ptr_ = end_ptr + 1;
  }

  return l;
}

BencodeDict parse_dict(char *input, char **end_ptr_) {
  BencodeDict d;
  hash_table_init(&d.table, 16);

  char *end_ptr = &input[1];

  while (end_ptr[0] != 'e') {
    char *key = parse_bytestring(end_ptr, &end_ptr);
    BencodeType value = parse_item(end_ptr, &end_ptr);
    BencodeType *heap_value = malloc(sizeof(BencodeType));

    memcpy(heap_value, &value, sizeof(BencodeType));
    hash_table_insert(&d.table, key, heap_value);
  }

  if (end_ptr_) {
    *end_ptr_ = end_ptr + 1;
  }

  return d;
}

BencodeType parse_item(char *input, char **end_ptr) {
  assert(strlen(input) > 0);

  BencodeType b;

  char type = input[0];
  switch (type) {
  case 'i':
    b.kind = INTEGER;
    b.asInt = parse_integer(input, end_ptr);
    break;
  case 'l':
    b.kind = LIST;
    b.asList = parse_list(input, end_ptr);
    break;
  case 'd':
    b.kind = DICTIONARY;
    b.asDict = parse_dict(input, end_ptr);
    break;
  default:
    b.kind = BYTESTRING;
    b.asString = parse_bytestring(input, end_ptr);
  }

  return b;
}

BencodeList parse(char *input) {
  BencodeList l = {0};
  BencodeList *lp = &l;

  da_init(lp, sizeof(BencodeType));

  char *end_ptr = input;
  while (strlen(end_ptr) > 0) {
    BencodeType v = parse_item(end_ptr, &end_ptr);
    da_append(lp, v);
  }

  return l;
}

void open_stream(Lexer *l, const char *filename) {
  FILE *f = fopen(filename, "r");

  if (!f) {
    perror("ERROR: could not open file");
    exit(EXIT_FAILURE);
  }

  fread(l->buf, l->bufsize, sizeof(char), f);
  l->input = f;
}

Lexer new_lexer(char *filename) {
  Lexer l;
  l.pos = 0;
  l.read_pos = 0;
  open_stream(&l, filename);
  return l;
}

void read_char(Lexer *l) {
  if (l->read_pos >= strlen(l->buf)) {
    memset(l->buf, 0, l->bufsize);
    fread(l->buf, l->bufsize, sizeof(char), l->input);
    l->read_pos = 0;
    l->pos = 0;

    l->ch = l->buf[l->read_pos];
  } else {
    l->ch = l->buf[l->read_pos];
  }

  l->pos = l->read_pos;
  l->read_pos++;
}

char peek_char(Lexer *l) {
  if (l->read_pos >= strlen(l->buf)) {
    if (l->input && !feof(l->input)) {
      char c = fgetc(l->input);
      ungetc(c, l->input);
      return c;
    }

    return '\0';
  }

  return l->buf[l->read_pos];
}

Token next_token(Lexer *l) {
  Token t = {0};

  read_char(l);

  switch (l->ch) {
  case 'd':
    t.type = DICT_START;
    break;
  case 'l':
    t.type = LIST_START;
    break;
  case 'i':
    t.type = INT_START;
    break;
  case ':':
    t.type = COLON;
    break;
  case 'e':
    if (l->prev.type != COLON) {
      t.type = END;
      break;
    }
  default:
    if (isdigit(l->ch)) {
      char buf[500];
      memset(buf, 0, sizeof(buf));
      size_t i = 0;
      while (true) {
        buf[i] = l->ch;
        if (!isdigit(buf[i])) {
          t.type = ILLEGAL;
          return t;
        }

        if (peek_char(l) == 'e' || peek_char(l) == ':') {
          break;
        }

        i++;
        read_char(l);
      }

      if (l->prev.type == INT_START) {
        t.type = INT;
      } else if (peek_char(l) == ':') {
        t.type = STRING_SIZE;
      } else {
        t.type = ILLEGAL;
      }

      t.asInt = strtol(buf, NULL, 10);
    } else if (l->prevprev.type == STRING_SIZE && isalnum(l->ch)) {
      t.type = STRING;
      size_t n = l->prevprev.asInt;
      t.asString = calloc(n, sizeof(char));

      size_t i = 0;
      for (; i < n - 1; i++, read_char(l)) {
        t.asString[i] = l->ch;
      }

      t.asString[i] = l->ch;
    }
  }

  l->prevprev = l->prev;
  l->prev = t;

  return t;
}
