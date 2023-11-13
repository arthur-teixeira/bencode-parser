#include "parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  size_t passed;
  size_t failed;
} test_suite;

test_suite suite = {0};

#define RUN_TEST(fn)                                                           \
  do {                                                                         \
    if (fn()) {                                                                \
      printf("%s: SUCCESS\n", #fn);                                            \
      suite.passed++;                                                          \
    } else {                                                                   \
      printf("%s: FAIL\n", #fn);                                               \
      suite.failed++;                                                          \
    }                                                                          \
  } while (0);

#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])

void TEST_BEGIN() {
  printf("\nrunning suite %s\n", __FILE__);
  printf("\n------------------------\n");
  suite.passed = 0;
  suite.failed = 0;
}

int TEST_END() {
  printf("\n------------------------\n");
  printf("%ld passed, %ld failed\n", suite.passed, suite.failed);

  if (suite.failed > 0) {
    printf("FAILED\n");
    return 1;
  }

  printf("PASSED\n");
  return 0;
}

bool validate_type(BencodeType *expected, BencodeType *actual);

bool validate_list(BencodeList *expected, BencodeList *actual) {
  if (expected->len != actual->len) {
    printf("expected %ld elements, got %ld\n", expected->len, actual->len);
  }

  for (size_t i = 0; i < expected->len; i++) {
    if (!validate_type(&expected->values[i], &actual->values[i])) {
      return false;
    }
  }

  return true;
}

bool validate_type(BencodeType *expected, BencodeType *actual) {
  if (!expected && actual) {
    printf("expected a value, got NULL");
    return false;
  }

  if (expected->kind != actual->kind) {
    printf("expected %i, got %i\n", expected->kind, actual->kind);
    return false;
  }

  switch (expected->kind) {
  case INTEGER:
    if (expected->asInt != actual->asInt) {
      printf("expected %ld, got %ld\n", expected->asInt, actual->asInt);
      return false;
    }
    break;
  case BYTESTRING:
    if (strcmp(expected->asString, actual->asString) != 0) {
      printf("expected %ld, got %ld\n", expected->asInt, actual->asInt);
      return false;
    }
    break;
  case LIST:
    if (!validate_list(&expected->asList, &actual->asList)) {
      return false;
    }
    break;
  default:
    return false;
  }

  return true;
}

typedef struct {
  char *in;
  long expected;
} int_test;

bool test_integers() {
  int_test tests[] = {
      {"i3e", 3},
      {"i-3e", -3},
      {"i304968e", 304968},
  };

  for (size_t i = 0; i < ARRAY_LEN(tests); i++) {
    BencodeType type = parse(tests[i].in);

    BencodeType expected = (BencodeType){
        .kind = INTEGER,
        .asInt = tests[i].expected,
    };

    if (!validate_type(&expected, &type)) {
      return false;
    }
  }

  return true;
}

bool test_bytestring() {
  char *test = "4:spam";

  BencodeType type = parse(test);

  BencodeType expected = (BencodeType){
      .kind = BYTESTRING,
      .asString = "spam",
  };

  return validate_type(&expected, &type);
}

bool test_lists() {
  char *test = "l4:spam4:eggs3:hame";

  BencodeType type = parse(test);

  BencodeType expected_list[] = {
      (BencodeType){.kind = BYTESTRING, .asString = "spam"},
      (BencodeType){.kind = BYTESTRING, .asString = "eggs"},
      (BencodeType){.kind = BYTESTRING, .asString = "ham"},
  };

  BencodeType expected = (BencodeType){
      .kind = LIST,
      .asList =
          (BencodeList){
              .len = 3,
              .values = expected_list,
          },
  };

  return validate_type(&expected, &type);
}

bool test_empty_list() {
  char *test = "le";

  BencodeType type = parse(test);

  BencodeType expected_list[] = {};

  BencodeType expected = (BencodeType){
      .kind = LIST,
      .asList =
          (BencodeList){
              .len = 0,
              .values = expected_list,
          },
  };

  return validate_type(&expected, &type);
}

int main() {
  TEST_BEGIN();
  RUN_TEST(test_integers);
  RUN_TEST(test_bytestring);
  RUN_TEST(test_lists);
  RUN_TEST(test_empty_list);
  return TEST_END();
}
