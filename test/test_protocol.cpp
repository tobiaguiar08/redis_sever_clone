#include "test_protocol.hpp"

#include <CppUTest/UtestMacros.h>
#include "CppUTest/TestHarness.h"

extern "C"
{
#include <string.h>
#include "protocol.h"
#include <stdio.h>
}

static struct resp_protocol_hdlr *result;

static void run_test_cases(const struct test_case_itf *test_cases, int num_cases)
{
    printf("\n");
    for (int i = 0; i < num_cases; i++) {
        const struct test_case_itf *tc = &test_cases[i];
        printf("Running test %s...\n", tc->name);
        memset(result, 0, sizeof(struct resp_protocol_hdlr));
        tc->test_case_fn(tc->input, tc->expected_output, tc->size);
        destroy_protocol_handler_data(result);
    }
}

void test_parse_frame_str(const void *buffer, const void *expected_str, long unsigned int expected_sz)
{
    const char *str_res = (const char *)expected_str;
    parse_frame((const char *)buffer, result);

    STRCMP_EQUAL(str_res, result->buf_arr[0].data);

    UNSIGNED_LONGS_EQUAL(expected_sz, result->arr_raw_length);
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

static struct test_case_itf TestCasesStrings[STR_MAX_TESTS];
static int num_str_test_cases = 0;

TEST_GROUP(TestGroupParseFrameStrings)
{

  void setup()
	{
      int i;

      result = create_protocol_handler();
      if (!result)
          FAIL("Protocol handler allocation failed");

      for (i = 0; i < STR_MAX_TESTS; i++, num_str_test_cases++) {
          TestCasesStrings[num_str_test_cases].name = TestCases_Str[i].name;
          TestCasesStrings[num_str_test_cases].test_case_fn = test_parse_frame_str;
          TestCasesStrings[num_str_test_cases].input = TestCases_Str[i].input;
          TestCasesStrings[num_str_test_cases].expected_output = TestCases_Str[i].expected_out;
          TestCasesStrings[num_str_test_cases].size = TestCases_Str[i].expected_size;
      }
	}

	void teardown()
	{
      destroy_protocol_handler(result);
	}

};

TEST(TestGroupParseFrameStrings, TestParseFrameStringsRunner)
{
    run_test_cases(TestCasesStrings, num_str_test_cases);
}

void test_parse_frame_int(const void *buffer, const void *expected_res, long unsigned int expected_sz)
{
    parse_frame((const char *)buffer, result);
    UNSIGNED_LONGS_EQUAL(expected_sz, result->buf_arr[0].raw_length);
    if (!expected_sz) {
        STRCMP_EQUAL("", result->buf_arr[0].data);
    } else {
        int result_int = *(int *)expected_res;
        UNSIGNED_LONGS_EQUAL(result_int, *(int *)result->buf_arr[0].data);
    }
}

static int test_int100 = 100;
static int test_int0 = 0;

const struct test_case_int TestCases_Int[INT_MAX_TESTS] = {
    {"IntegerPartial", ":10", "", 0},
    {"IntegerWhole", ":100\r\n", &test_int100, strlen(":100\r\n")},    
    {"IntegerWholePartial", ":100\r\n+OK", &test_int100, strlen(":100\r\n")},
    {"IntegerWholeZero", ":0\r\n", &test_int0, strlen(":0\r\n")}
};

static struct test_case_itf TestCasesIntegers[INT_MAX_TESTS];
static int num_int_test_cases = 0;

TEST_GROUP(TestGroupParseFrameInt)
{

  void setup()
	{
      int i;

      result = create_protocol_handler();
      if (!result)
          FAIL("Protocol handler allocation failed");

      for (i = 0; i < INT_MAX_TESTS; i++, num_int_test_cases++) {
          TestCasesIntegers[num_int_test_cases].name = TestCases_Int[i].name;
          TestCasesIntegers[num_int_test_cases].test_case_fn = test_parse_frame_int;
          TestCasesIntegers[num_int_test_cases].input = TestCases_Int[i].input;
          TestCasesIntegers[num_int_test_cases].expected_output = TestCases_Int[i].expected_out;
          TestCasesIntegers[num_int_test_cases].size = TestCases_Int[i].expected_size;
      }
	}

	void teardown()
	{
      destroy_protocol_handler(result);
	}

};

TEST(TestGroupParseFrameInt, TestParseFrameIntRunner)
{
    run_test_cases(TestCasesIntegers, num_int_test_cases);
}

static const struct test_case_str TestCases_BulkStr[STR_MAX_TESTS] = {
    {"BulkStringNull", "$-1\r\n", "NULL", strlen("$-1\r\n")},
    {"BulkStringPartial", "$5\r\nHel", "", 0},
    {"BulkStringPartial2", "$5\r\nHello\r", "", 0},
    {"BulkStringWhole", "$5\r\nHello\r\n", "Hello", strlen("$5\r\nHello\r\n")},
    {"BulkStringWholeEmpty", "$12\r\nHello, World\r\n", "Hello, World", strlen("$12\r\nHello, World\r\n")},
    {"BulkStringWholePartial", "$12\r\nHello\r\nWorld\r\n", "Hello\r\nWorld", strlen("$12\r\nHello\r\nWorld\r\n")},
    {"BulkStringEmpty", "$0\r\n\r\n", "", strlen("$0\r\n\r\n")}
};

static struct test_case_itf TestCasesBulk[STR_MAX_TESTS];
static int num_bulk_test_cases = 0;

TEST_GROUP(TestGroupParseFrameBulk)
{

  void setup()
	{
      int i;

      result = create_protocol_handler();
      if (!result)
          FAIL("Protocol handler allocation failed");

      for (i = 0; i < STR_MAX_TESTS; i++, num_bulk_test_cases++) {
          TestCasesBulk[num_bulk_test_cases].name = TestCases_BulkStr[i].name;
          TestCasesBulk[num_bulk_test_cases].test_case_fn = test_parse_frame_str;
          TestCasesBulk[num_bulk_test_cases].input = TestCases_BulkStr[i].input;
          TestCasesBulk[num_bulk_test_cases].expected_output = TestCases_BulkStr[i].expected_out;
          TestCasesBulk[num_bulk_test_cases].size = TestCases_BulkStr[i].expected_size;
      }
	}

	void teardown()
	{
      destroy_protocol_handler(result);
	}

};

TEST(TestGroupParseFrameBulk, TestParseFrameBulkRunner)
{
    run_test_cases(TestCasesBulk, num_bulk_test_cases);
}

void test_parse_frame_array(const void *buffer, const void *expected_res, long unsigned int expected_sz)
{
    const char **expected_array = (const char **)expected_res;
    const char *input_array = (const char *)buffer;
    const char *result_array;
    int array_sz = 0;

    parse_frame((const char *)input_array, result);

    UNSIGNED_LONGS_EQUAL(expected_sz, result->arr_raw_length);

    if (!result->buf_arr) {
        FAIL("NULL pointer result->data.p_data_arr");
    } else if (!expected_sz){
        STRCMP_EQUAL("", result->buf_arr[0].data);
    } else {
        for (int i = 0; i < array_sz; i++) {
            result_array = result->buf_arr[i].data;
            STRCMP_EQUAL(expected_array[i], result_array);
        }
    }
}

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

static struct test_case_itf TestCasesArray[ARRAY_STR_MAX_TESTS];
static int num_array_test_cases = 0;

TEST_GROUP(TestGroupParseFrameArray)
{

  void setup()
	{
      int i;

      result = create_protocol_handler();
      if (!result)
          FAIL("Protocol handler allocation failed");

      for (i = 0; i < ARRAY_STR_MAX_TESTS; i++, num_array_test_cases++) {
          TestCasesArray[num_array_test_cases].name = TestCases_ArrayStr[i].name;
          TestCasesArray[num_array_test_cases].test_case_fn = test_parse_frame_array;
          TestCasesArray[num_array_test_cases].input = TestCases_ArrayStr[i].input;
          TestCasesArray[num_array_test_cases].expected_output = TestCases_ArrayStr[i].expected_out;
          TestCasesArray[num_array_test_cases].size = TestCases_ArrayStr[i].expected_size;
      }
	}

	void teardown()
	{
      destroy_protocol_handler(result);
	}

};

TEST(TestGroupParseFrameArray, TestParseFrameRunner)
{
    run_test_cases(TestCasesArray, num_array_test_cases);
}

