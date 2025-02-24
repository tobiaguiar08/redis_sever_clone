#include "test_protocol.hpp"

#include <CppUTest/UtestMacros.h>
#include "CppUTest/TestHarness.h"

extern "C"
{
#include <string.h>
#include "protocol.h"
}

static struct resp_protocol_hdlr *result;

void test_parse_frame_array(const char *buffer, const void *expected_res, long unsigned int expected_sz)
{
    const char **expected_array = (const char **)expected_res;
    const char *result_array;
    int array_sz = 0;

    if (buffer[0] == '*') {
        array_sz = strtol(&buffer[1], NULL, 10);
    }

    parse_frame(buffer, result);

    UNSIGNED_LONGS_EQUAL(expected_sz, result->size);

    if (!result->data.p_data_arr) {
        FAIL("NULL pointer result->data.p_data_arr");
    } else if (!expected_sz){
        STRCMP_EQUAL("", (const char *)result->data.p_data_arr[0]);
    } else {
        for (int i = 0; i < array_sz; i++) {
            result_array = (const char *)result->data.p_data_arr[i];
            STRCMP_EQUAL(expected_array[i], result_array);
        }

    }
}

void test_parse_frame_str(const char *buffer, const void *expected_str, long unsigned int expected_sz)
{
    const char *str_res = (const char *)expected_str;
    parse_frame(buffer, result);

    STRCMP_EQUAL(str_res, (const char *)result->data.p_data);

    UNSIGNED_LONGS_EQUAL(expected_sz, result->size);
}

void test_parse_frame_int(const char *buffer, const void *expected_res, long unsigned int expected_sz)
{
    int result_int = *(int *)expected_res;
    parse_frame(buffer, result);
    UNSIGNED_LONGS_EQUAL(result_int, *(int *)result->data.p_data);
    UNSIGNED_LONGS_EQUAL(expected_sz, result->size);
}

static const struct test_case_str TestCases_Str[STR_MAX_TESTS] = {
    {"SimpleStringPartial", "+Part", "", 0},
    {"SimpleStringWhole", "+Full\r\n", "Full", strlen("+Full\r\n")},
    {"SimpleStringWholeEmpty", "+\r\n", "", strlen("+\r\n")},
    {"SimpleStringWholePartial", "+Full\r\n+Part", "Full", strlen("+Full\r\n")},
    {"ErrorStringPartial", "-Err", "", 0},
    {"ErrorStringWhole", "-Err\r\n", "Err", strlen("-Err\r\n")},
    {"ErrorStringWhole Partial", "-Err\r\n+Part", "Err", strlen("-Err\r\n")}
};

static int test_int100 = 100;
static int test_int0 = 0;

const struct test_case_int TestCases_Int[INT_MAX_TESTS] = {
    {"IntegerPartial", ":10", "", 0},
    {"IntegerWhole", ":100\r\n", &test_int100, strlen(":100\r\n")},    
    {"IntegerWholePartial", ":100\r\n+OK", &test_int100, strlen(":100\r\n")},
    {"IntegerWholeZero", ":0\r\n", &test_int0, strlen(":0\r\n")}
};

static const struct test_case_str TestCases_BulkStr[STR_MAX_TESTS] = {
    {"BulkStringNull", "$-1\r\n", "NULL", strlen("$-1\r\n")},
    {"BulkStringPartial", "$5\r\nHel", "", 0},
    {"BulkStringPartial2", "$5\r\nHello\r", "", 0},
    {"BulkStringWhole", "$5\r\nHello\r\n", "Hello", strlen("$5\r\nHello\r\n")},
    {"BulkStringWholeEmpty", "$12\r\nHello, World\r\n", "Hello, World", strlen("$12\r\nHello, World\r\n")},
    {"BulkStringWholePartial", "$12\r\nHello\r\nWorld\r\n", "Hello\r\nWorld", strlen("$12\r\nHello\r\nWorld\r\n")},
    {"BulkStringEmpty", "$0\r\n\r\n", "", strlen("$0\r\n\r\n")}
};

static const char *TestCaseArray1[1] = {""};
static const char *TestCaseArray2[2] = {"Hello", "World"};
static const char *TestCaseArray3[1] = {"SingleString"};
static const char *TestCaseArray4[1] = {""};
static const char *TestCaseArray5[2] = {"NULL", "Hello"};

static const struct test_case_array TestCases_ArrayStr[ARRAY_STR_MAX_TESTS] = {
    {"ArrayStringPartial", "*2\r\n$5\r\nhello\r\n$5\r\n", &TestCaseArray1[0], 0},
    {"ArrayStringWhole", "*2\r\n$5\r\nHello\r\n$5\r\nWorld\r\n", &TestCaseArray2[0], strlen("*2\r\n$5\r\nHello\r\n$5\r\nWorld\r\n")},
    {"ArrayStringSingleElement", "*1\r\n$12\r\nSingleString\r\n", &TestCaseArray3[0], strlen("*1\r\n$11\r\nSingleString\r\n")},
    {"ArrayStringEmpty", "*0\r\n", &TestCaseArray4[0], strlen("*0\r\n")},
    {"ArrayStringWithNullElement", "*2\r\n$-1\r\n$5\r\nHello\r\n", &TestCaseArray5[0], strlen("*2\r\n$-1\r\n$5\r\nHello\r\n")}
};

static struct test_case_itf TestCases[STR_MAX_TESTS*2 + INT_MAX_TESTS + ARRAY_STR_MAX_TESTS];
static int num_test_cases = 0;

TEST_GROUP(TestGroupParseFrame)
{

  void setup()
	{
      int i;

      result = create_protocol_handler();
      if (!result)
          FAIL("Protocol handler allocation failed");

      for (i = 0; i < STR_MAX_TESTS; i++, num_test_cases++) {
          TestCases[num_test_cases].name = TestCases_Str[i].name;
          TestCases[num_test_cases].test_case_fn = test_parse_frame_str;
          TestCases[num_test_cases].input = TestCases_Str[i].input;
          TestCases[num_test_cases].expected_output = TestCases_Str[i].expected_out;
          TestCases[num_test_cases].expected_size = TestCases_Str[i].expected_size;
      }

      for (i = STR_MAX_TESTS; i < STR_MAX_TESTS + INT_MAX_TESTS; i++, num_test_cases++) {
          TestCases[num_test_cases].name = TestCases_Int[i - STR_MAX_TESTS].name;
          TestCases[num_test_cases].test_case_fn = (i - STR_MAX_TESTS == 0) ? test_parse_frame_str : test_parse_frame_int;
          TestCases[num_test_cases].input = TestCases_Int[i - STR_MAX_TESTS].input;
          TestCases[num_test_cases].expected_output = TestCases_Int[i - STR_MAX_TESTS].expected_out;
          TestCases[num_test_cases].expected_size = TestCases_Int[i - STR_MAX_TESTS].expected_size;
      }

      for (i = STR_MAX_TESTS + INT_MAX_TESTS; i < STR_MAX_TESTS*2 + INT_MAX_TESTS; i++, num_test_cases++) {
          TestCases[num_test_cases].name = TestCases_BulkStr[i - (STR_MAX_TESTS + INT_MAX_TESTS)].name;
          TestCases[num_test_cases].test_case_fn = test_parse_frame_str;
          TestCases[num_test_cases].input = TestCases_BulkStr[i - (STR_MAX_TESTS + INT_MAX_TESTS)].input;
          TestCases[num_test_cases].expected_output = TestCases_BulkStr[i - (STR_MAX_TESTS + INT_MAX_TESTS)].expected_out;
          TestCases[num_test_cases].expected_size = TestCases_BulkStr[i - (STR_MAX_TESTS + INT_MAX_TESTS)].expected_size;
      }

      for (i = STR_MAX_TESTS*2 + INT_MAX_TESTS; i < STR_MAX_TESTS*2 + INT_MAX_TESTS + ARRAY_STR_MAX_TESTS; i++, num_test_cases++) {
          TestCases[num_test_cases].name = TestCases_ArrayStr[i - (STR_MAX_TESTS*2 + INT_MAX_TESTS)].name;
          TestCases[num_test_cases].test_case_fn = test_parse_frame_array;
          TestCases[num_test_cases].input = TestCases_ArrayStr[i - (STR_MAX_TESTS*2 + INT_MAX_TESTS)].input;
          TestCases[num_test_cases].expected_output = TestCases_ArrayStr[i - (STR_MAX_TESTS*2 + INT_MAX_TESTS)].expected_out;
          TestCases[num_test_cases].expected_size = TestCases_ArrayStr[i - (STR_MAX_TESTS*2 + INT_MAX_TESTS)].expected_size;
      }
	}

	void teardown()
	{
      destroy_protocol_handler(result);
	}

};

TEST(TestGroupParseFrame, TestParseFrameRunner)
{
    printf("\n");
    for (int i = 0; i < num_test_cases; i++) {
        const struct test_case_itf *tc = &TestCases[i];
        printf("Running test %s...\n",tc->name);
        tc->test_case_fn(tc->input, tc->expected_output, tc->expected_size);
        destroy_protocol_handler_data(result);
    }
}

