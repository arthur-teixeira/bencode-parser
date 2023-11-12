#include "parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define RUN_TEST(fn)                                                           \
  do {                                                                         \
    printf("%s: ", #fn);                                                       \
    if (fn()) {                                                                \
      printf(" SUCCESS\n");                                                    \
    } else {                                                                   \
      printf("FAIL\n");                                                        \
    }                                                                          \
  } while (0);

bool test_integer() {
  char *test = "i3e";

  BencodeType *type = parse(test);

  if (!type) {
    printf("expected a value, got NULL");
    return false;
  }

  if (type->kind != INTEGER) {
    printf("expected INTEGER, got %i\n", type->kind);
    return false;
  }

  if (type->asInt != 3) {
    printf("expected 3, got %ld\n", type->asInt);
    return false;
  }

  return true;
}

bool test_bytestring() {
  char *test = "4:spam";

  BencodeType *type = parse(test);

  if (!type) {
    printf("expected a value, got NULL");
    return false;
  }

  if (type->kind != BYTESTRING) {
    printf("expected BYTESTRING, got %i\n", type->kind);
    return false;
  }

  if (strcmp(type->asString, "spam") != 0) {
    printf("expected 'spam', got '%s'\n", type->asString);
    return false;
  }

  return true;
}

int main() {
  RUN_TEST(test_integer);
  RUN_TEST(test_bytestring);
  return 0;
}
