#include "stb_hashtable.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unity/unity.h>

#define BENCODE_IMPLEMENTATION
#include "stb_bencode.h"

void setUp(void) {}

void tearDown(void) {}

#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])

void validate_type(BencodeType *expected, BencodeType *actual);

void validate_list(BencodeList *expected, BencodeList *actual) {
  if (expected->len != actual->len) {
    printf("expected %ld elements, got %ld\n", expected->len, actual->len);
  }

  for (size_t i = 0; i < expected->len; i++) {
    validate_type(&expected->values[i], &actual->values[i]);
  }
}

void validate_type(BencodeType *expected, BencodeType *actual) {
  if (!actual && expected) {
    TEST_FAIL_MESSAGE("expected a value, got NULL");
  }

  TEST_ASSERT_EQUAL(expected->kind, actual->kind);

  switch (expected->kind) {
  case INTEGER:
    TEST_ASSERT_EQUAL(expected->asInt, actual->asInt);
    break;
  case BYTESTRING:
    TEST_ASSERT_EQUAL_STRING(expected->asString, actual->asString);
    break;
  case LIST:
    validate_list(&expected->asList, &actual->asList);
    break;
  default:
    break;
  }
}

void test_lexer() {
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

  for (size_t i = 0; i < ARRAY_LEN(expected_tokens); i++) {
    Token expected_t = expected_tokens[i];
    Token t = next_token(&l);
    char msg[100];
    sprintf(msg, "At i = %ld\n", i);

    TEST_ASSERT_EQUAL_MESSAGE(expected_t.type, t.type, msg);
    switch (t.type) {
    case STRING_SIZE:
    case INT:
      TEST_ASSERT_EQUAL_INT_MESSAGE(expected_t.asInt, t.asInt, msg);
      break;
    case STRING:
      TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_t.asString, t.asString, msg);
      break;
    default:
      break;
    }
  }
}

Parser get_parser(char *input) {
  Parser p = {0};

  Lexer l = {0};
  l.buf = input;
  l.prevprev = (Token){ILLEGAL, .asString = ""};
  l.prev = (Token){ILLEGAL, .asString = ""};

  p.l = l;
  p.cur_token = next_token(&p.l);
  p.peek_token = next_token(&p.l);

  return p;
}

BencodeType parse_and_check_for_errors(char *input) {
  Parser p = get_parser(input);
  BencodeType result = parse_item(&p);

  if (p.error_index > 0) {
    printf("Parser has errors: \n");
    for (size_t i = 0; i < p.error_index; i++) {
      printf("%s\n", p.errors[i]);
      free(p.errors[i]);
    }
  }

  return result;
}

typedef struct {
  char *in;
  long expected;
} int_test;

void test_integers() {
  int_test tests[] = {
      {"i3e", 3},
      {"i-3e", -3},
      {"i304968e", 304968},
  };

  for (size_t i = 0; i < ARRAY_LEN(tests); i++) {
    Parser p = get_parser(tests[i].in);
    BencodeType type = parse_item(&p);

    BencodeType expected = (BencodeType){
        .kind = INTEGER,
        .asInt = tests[i].expected,
    };

    validate_type(&expected, &type);
  }
}

void test_bytestring() {
  char *test = "4:spam";
  BencodeType type = parse_and_check_for_errors(test);

  BencodeType expected = (BencodeType){
      .kind = BYTESTRING,
      .asString = "spam",
  };

  return validate_type(&expected, &type);
}

void test_lists() {
  char *test = "l4:spam4:eggs3:hame";
  Parser p = get_parser(test);

  BencodeType type = parse_item(&p);

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

void test_empty_list() {
  char *test = "le";
  Parser p = get_parser(test);
  BencodeType type = parse_item(&p);

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

void test_dict() {
  char *test = "d3:cow3:moo4:spam4:eggse";
  Parser p = get_parser(test);
  BencodeType type = parse_item(&p);

  char *expected_keys[] = {
      "cow",
      "spam",
  };

  BencodeType expected_values[] = {
      (BencodeType){.kind = BYTESTRING, .asString = "moo"},
      (BencodeType){.kind = BYTESTRING, .asString = "eggs"},
  };

  TEST_ASSERT_EQUAL(DICTIONARY, type.kind);

  for (size_t i = 0; i < ARRAY_LEN(expected_keys); i++) {
    BencodeType *t = hash_table_lookup(&type.asDict, expected_keys[i]);
    char msg[100];
    sprintf(msg, "expected key %s to be in the dict at i = %ld\n",
            expected_keys[i], i);
    TEST_ASSERT_NOT_NULL_MESSAGE(t, msg);

    sprintf(msg, "at i = %ld\n", i);

    TEST_ASSERT_EQUAL_MESSAGE(BYTESTRING, t->kind, msg);

    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_values[i].asString, t->asString,
                                     msg);
  }
}

void test_multiple_values() {
  char *test = "5:helloi1230eli1ei2ei3ei4ee";
  Parser p = get_parser(test);
  BencodeList actual = parse(&p);

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
  UNITY_BEGIN();
  RUN_TEST(test_lexer);
  RUN_TEST(test_integers);
  RUN_TEST(test_bytestring);
  RUN_TEST(test_lists);
  RUN_TEST(test_empty_list);
  RUN_TEST(test_dict);
  RUN_TEST(test_multiple_values);
  return UNITY_END();
}
