#include "parser.h"
#include "stb_hashtable.h"
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

bool test_lexer() {
  char *input = "5:helloi1230eli1ei2ei3ei4eed3:cow3:moo4:spam4:eggse";

  Token expected_tokens[] = {
      {STRING_SIZE, .asInt = 5},
      {COLON, .asString = ""},
      {STRING, .asString = "hello"},
      {INT_START, .asString = ""},
      {INT, .asInt = 1230},
      {END, .asString = ""},
      {LIST_START, .asString = ""},
      {INT_START, .asString = ""},
      {INT, .asInt = 1},
      {END, .asString = ""},
      {INT_START, .asString = ""},
      {INT, .asInt = 2},
      {END, .asString = ""},
      {INT_START, .asString = ""},
      {INT, .asInt = 3},
      {END, .asString = ""},
      {INT_START, .asString = ""},
      {INT, .asInt = 4},
      {END, .asString = ""},
      {END, .asString = ""},
      {DICT_START, .asString = ""},
      {STRING_SIZE, .asInt = 3},
      {COLON, .asString = ""},
      {STRING, .asString = "cow"},
      {STRING_SIZE, .asInt = 3},
      {COLON, .asString = ""},
      {STRING, .asString = "moo"},
      {STRING_SIZE, .asInt = 4},
      {COLON, .asString = ""},
      {STRING, .asString = "spam"},
      {STRING_SIZE, .asInt = 4},
      {COLON, .asString = ""},
      {STRING, .asString = "eggs"},
      {END, .asString = ""},
  };

  Lexer l = {0};
  l.prevprev = (Token){ILLEGAL, .asString = ""};
  l.prev = (Token){ILLEGAL, .asString = ""};
  l.buf = input;

  size_t i = 0;
  for (; i < ARRAY_LEN(expected_tokens); i++) {
    Token expected_t = expected_tokens[i];
    Token t = next_token(&l);

    if (t.type != expected_t.type) {
      printf("Expected type to be %i, got %i\n", expected_t.type, t.type);
      goto fail;
    }

    switch (t.type) {
    case STRING_SIZE:
    case INT:
      if (t.asInt != expected_t.asInt) {
        printf("Expected asInt to be %ld, got %ld\n", expected_t.asInt,
               t.asInt);
        goto fail;
      }
      break;
    case STRING:
      if (strcmp(t.asString, expected_t.asString) != 0) {
        printf("Expected asString to be %s, got %s\n", expected_t.asString,
               t.asString);
        goto fail;
      }
      break;
    default:
      break;
    }
  }

  return true;

fail:
 printf("at i = %ld\n", i);
 return false;
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
    BencodeType type = parse_item(tests[i].in, NULL);

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

  BencodeType type = parse_item(test, NULL);

  BencodeType expected = (BencodeType){
      .kind = BYTESTRING,
      .asString = "spam",
  };

  return validate_type(&expected, &type);
}

bool test_lists() {
  char *test = "l4:spam4:eggs3:hame";

  BencodeType type = parse_item(test, NULL);

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

  BencodeType type = parse_item(test, NULL);

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

bool test_dict() {
  char *test = "d3:cow3:moo4:spam4:eggse";

  BencodeType type = parse_item(test, NULL);

  char *expected_keys[] = {
      "cow",
      "spam",
  };

  BencodeType expected_values[] = {
      (BencodeType){.kind = BYTESTRING, .asString = "moo"},
      (BencodeType){.kind = BYTESTRING, .asString = "eggs"},
  };

  if (type.kind != DICTIONARY) {
    printf("expected %i, got %i\n", DICTIONARY, type.kind);
    return false;
  }

  for (size_t i = 0; i < ARRAY_LEN(expected_keys); i++) {
    BencodeType *t = hash_table_lookup(&type.asDict.table, expected_keys[i]);
    if (!t) {
      printf("expected key %s to be in the dict\n", expected_keys[i]);
      return false;
    }

    if (t->kind != BYTESTRING) {
      printf("expected %i, got %i\n", DICTIONARY, t->kind);
      return false;
    }

    if (strcmp(t->asString, expected_values[i].asString) != 0) {
      printf("expected key %s to have value %s, got %s\n", expected_keys[i],
             expected_values[i].asString, t->asString);
      return false;
    }
  }

  return true;
}

bool test_multiple_values() {
  char *test = "5:helloi1230eli1ei2ei3ei4ee";

  BencodeList actual = parse(test);

  BencodeType expected_list_values[] = {
      (BencodeType){.kind = INTEGER, .asInt = 1},
      (BencodeType){.kind = INTEGER, .asInt = 2},
      (BencodeType){.kind = INTEGER, .asInt = 3},
      (BencodeType){.kind = INTEGER, .asInt = 4},
  };

  BencodeList expected_list = {
      .len = 4,
      .values = expected_list_values,
  };

  BencodeType expected_values[] = {
      (BencodeType){.kind = BYTESTRING, .asString = "hello"},
      (BencodeType){.kind = INTEGER, .asInt = 1230},
      (BencodeType){.kind = LIST, .asList = expected_list},
  };

  BencodeList expected = {
      .len = 3,
      .values = expected_values,
  };

  return validate_list(&expected, &actual);
}

int main() {
  TEST_BEGIN();
  RUN_TEST(test_lexer);
  RUN_TEST(test_integers);
  RUN_TEST(test_bytestring);
  RUN_TEST(test_lists);
  RUN_TEST(test_empty_list);
  RUN_TEST(test_dict);
  RUN_TEST(test_multiple_values);
  return TEST_END();
}
