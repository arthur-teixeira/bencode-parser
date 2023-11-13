#include "parser.h"
#include <assert.h>
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
    da_append(lp, parse(end_ptr, &end_ptr));
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
    BencodeType value = parse(end_ptr, &end_ptr);
    BencodeType *heap_value = malloc(sizeof(BencodeType));

    memcpy(heap_value, &value, sizeof(BencodeType));
    hash_table_insert(&d.table, key, heap_value);
  }

  if (end_ptr_) {
    *end_ptr_ = end_ptr + 1;
  }

  return d;
}

BencodeType parse(char *input, char **end_ptr) {
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

BencodeList parse_stream(char *input) {
  BencodeList l = { 0 };
  BencodeList *lp = &l;

  da_init(lp, sizeof(BencodeType));

  char *end_ptr = input;
  while (strlen(end_ptr) > 0) {
    BencodeType v = parse(end_ptr, &end_ptr);
    da_append(lp, v);
  }

  return l;
}
