#include "parser.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

signed long parse_integer(char *input, char **end_ptr) {
  char buf[strlen(input)];

  size_t i = 1;
  while (input[i] != 'e') {
    buf[i - 1] = input[i];
    i++;
  }

  *end_ptr = &input[i];

  return strtol(buf, NULL, 10);
}

char *parse_bytestring(char *input, char **end_ptr) {
  size_t n = atoi((char[]){input[0]});

  char *out = calloc(n, sizeof(char));

  size_t i = 2;
  for (; i < n + 2; i++) {
    out[i - 2] = input[i];
  }

  *end_ptr = &input[i];

  return out;
}

BencodeType *parse(char *input) {
  assert(strlen(input) > 0);

  BencodeType *b = malloc(sizeof(BencodeType));

  char type = input[0];
  char *end_ptr;
  switch (type) {
  case 'i':
    b->kind = INTEGER;
    b->asInt = parse_integer(input, &end_ptr);
    break;
  case 'l':
    b->kind = LIST;
    assert(0 && "NOT IMPLEMENTED");
    break;
  case 'd':
    b->kind = DICTIONARY;
    assert(0 && "NOT IMPLEMENTED");
  default:
    b->kind = BYTESTRING;
    b->asString = parse_bytestring(input, &end_ptr);
  }

  return b;
};
