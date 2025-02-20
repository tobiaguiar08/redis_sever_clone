
#include <string.h>
#include "test_protocol.hpp"

#include <CppUTest/UtestMacros.h>
#include <cstdio>
#include "CppUTest/TestHarness.h"

extern "C"
{
#include "protocol.h"
}

static struct resp_protocol_hdlr *result;

void test_parse_frame_str(const char *buffer, const void *expected_str, long unsigned int expected_sz)
{
    const char *str_res = (const char *)expected_str;
    result = parse_frame(buffer);
    STRCMP_EQUAL(str_res, (const char *)result->data);
    UNSIGNED_LONGS_EQUAL(expected_sz, result->size);
}

void test_parse_frame_int(const char *buffer, const void *expected_res, long unsigned int expected_sz)
{
    int result_int = *(int *)expected_res;
    result = parse_frame(buffer);
    UNSIGNED_LONGS_EQUAL(result_int, *(int *)result->data);
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

static struct test_case_itf TestCases[STR_MAX_TESTS + INT_MAX_TESTS];
static int num_test_cases = 0;

TEST_GROUP(TestGroupParseFrame)
{

  void setup()
	{
      int i;
      result = NULL;

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
	}

	void teardown()
	{
	}

};

TEST(TestGroupParseFrame, TestParseFrameRunner)
{
    printf("\n");
    for (int i = 0; i < num_test_cases; i++) {
        const struct test_case_itf *tc = &TestCases[i];
        printf("Running test %s...\n",tc->name);
        tc->test_case_fn(tc->input, tc->expected_output, tc->expected_size);
    }
}

