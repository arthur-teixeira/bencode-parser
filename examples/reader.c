#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define BENCODE_IMPLEMENTATION
#include "../stb_bencode.h"

void print_bencode(BencodeType t, size_t indent);

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: %s [filename]\n", argv[0]);
    return 0;
  }

  Lexer l = new_lexer(argv[1]);
  Parser p = new_parser(l);

  BencodeList items = parse(&p);

  if (p.error_index > 0) {
    printf("ERROR: Parser encountered errors:\n");
    for (size_t i = 0; i < p.error_index; i++) {
      printf("%s\n", p.errors[i]);
    }

    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < items.len; i++) {
    print_bencode(items.values[i], 0);
  }
}

void indent(size_t indent) {
  for (size_t i = 0; i < indent; i++) {
    printf(" ");
  }
}

void print_bencode(BencodeType t, size_t indent_size) {
  indent(indent_size);
  switch (t.kind) {
  case INTEGER:
    printf("INT = %ld\n", t.asInt);
    break;
  case BYTESTRING:
    printf("BYTESTRING = %s\n", t.asString);
    break;
  case LIST:
    printf("LIST: \n");
    for (size_t i = 0; i < t.asList.len; i++) {
      print_bencode(t.asList.values[i], indent_size + 2);
    }
    printf("END_LIST\n");
    break;
  case DICTIONARY:
    printf("DICTIONARY: \n");
    for (size_t i = 0; i < t.asDict.size; i++) {
      if (t.asDict.values[i].in_use) {
        indent(indent_size + 2);
        printf("key %s\n", (char *)t.asDict.values[i].key);
        BencodeType *value = t.asDict.values[i].value;
        print_bencode(*value, indent_size + 2);
      }
    }
    break;
  default:
    break;
  }
}
