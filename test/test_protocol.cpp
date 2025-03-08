#include "test_protocol.hpp"

#include <CppUTest/UtestMacros.h>
#include <cstdio>
#include "CppUTest/TestHarness.h"
#include "protocol.h"

static void run_test_cases(const struct test_case_itf *test_cases, int num_cases)
{
    printf("\n");
    for (int i = 0; i < num_cases; i++) {
        const struct test_case_itf *tc = &test_cases[i];
        printf("Running test %s...\n", tc->name);
        tc->test_case_fn(tc->input, tc->expected_output, tc->size);
    }
}

void test_parse_frame_str(const void *buffer, const void *expected_str, long unsigned int expected_sz)
{
    struct resp_protocol_hdlr result;
    memset(&result, 0, sizeof(struct resp_protocol_hdlr));
    const char *str_res = (const char *)expected_str;
    parse_frame((const char *)buffer, &result);

    STRCMP_EQUAL(str_res, (const char *)result.buf_arr[0].data);

    UNSIGNED_LONGS_EQUAL(expected_sz, result.arr_raw_length);
    destroy_protocol_handler_data(&result);
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
	}

};

TEST(TestGroupParseFrameStrings, TestParseFrameStringsRunner)
{
    run_test_cases(TestCasesStrings, num_str_test_cases);
}

void test_parse_frame_int(const void *buffer, const void *expected_res, long unsigned int expected_sz)
{
    struct resp_protocol_hdlr result;
    memset(&result, 0, sizeof(struct resp_protocol_hdlr));

    parse_frame((const char *)buffer, &result);
    UNSIGNED_LONGS_EQUAL(expected_sz, result.buf_arr[0].raw_length);
    if (!expected_sz) {
        const char *tmp = (const char *)result.buf_arr[0].data;
        STRCMP_EQUAL("", tmp);
    } else {
        int result_int = *(int *)expected_res;
        UNSIGNED_LONGS_EQUAL(result_int, *(int *)result.buf_arr[0].data);
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
    struct resp_protocol_hdlr result;
    memset(&result, 0, sizeof(struct resp_protocol_hdlr));


    parse_frame((const char *)input_array, &result);

    UNSIGNED_LONGS_EQUAL(expected_sz, result.arr_raw_length);

    if (!result.buf_arr) {
        FAIL("NULL pointer result->data.p_data_arr");
    } else if (!expected_sz){
        const char *tmp = (const char *)result.buf_arr[0].data;
        STRCMP_EQUAL("", tmp);
    } else {
        for (int i = 0; i < array_sz; i++) {
            result_array = (const char *)result.buf_arr[i].data;
            STRCMP_EQUAL(expected_array[i], result_array);
        }
    }

    destroy_protocol_handler_data(&result);
}

void compare_hdlr_arrays(const struct resp_protocol_hdlr *expected, struct resp_protocol_hdlr *parsed)
{
    if (!expected || !parsed)
        FAIL("One of the parsed or expected arrays is NULL");

    UNSIGNED_LONGS_EQUAL(expected->arr_size, parsed->arr_size);
    UNSIGNED_LONGS_EQUAL(expected->arr_raw_length, parsed->arr_raw_length);
    BYTES_EQUAL(expected->type, parsed->type);

    for (int i = 0; i < expected->arr_size; i++) {
        struct str_type_buffer *expected_buf = &expected->buf_arr[i];
        struct str_type_buffer *parsed_buf = &parsed->buf_arr[i];

        BYTES_EQUAL(expected_buf->type, parsed_buf->type);
        UNSIGNED_LONGS_EQUAL(expected_buf->length, parsed_buf->length);
        UNSIGNED_LONGS_EQUAL(expected_buf->raw_length, parsed_buf->raw_length);

        if (expected_buf->type == ':') {
            int expected_int = *(int *)expected_buf->data;
            int parsed_int = *(int *)parsed_buf->data;
            LONGS_EQUAL(expected_int, parsed_int);
        }
        else if (expected_buf->type == '*') {
            struct resp_protocol_hdlr *tmp_expected = (struct resp_protocol_hdlr *)expected_buf->data;
            struct resp_protocol_hdlr *tmp_parsed = (struct resp_protocol_hdlr *)parsed_buf->data;

            compare_hdlr_arrays(tmp_expected, tmp_parsed);
        }
        else {
            MEMCMP_EQUAL(expected_buf->data, parsed_buf->data, expected_buf->length);
        }
    }
}

void test_parse_frame_array_nested(const void *buffer, const void *expected_res, long unsigned int expected_sz)
{
    const struct resp_protocol_hdlr *expected_array = (const struct resp_protocol_hdlr *)expected_res;
    const char *input_array = (const char *)buffer;
    struct resp_protocol_hdlr parsed_result;

    memset(&parsed_result, 0, sizeof(parsed_result));
    parse_frame(input_array, &parsed_result);

    UNSIGNED_LONGS_EQUAL(expected_sz, parsed_result.arr_raw_length);
    compare_hdlr_arrays(expected_array, &parsed_result);

    destroy_protocol_handler_data(&parsed_result);
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

static const int TestCaseArrayNested1[3] = {1, 2, 3};
static const char *TestCaseArrayNested2[2] = {"HELLO", "WORLD"};

static struct str_type_buffer NestedBuf1[3] = {
    { .length = 1, .raw_length = 4, .type = ':', .data = (void *)&TestCaseArrayNested1[0] },
    { .length = 1, .raw_length = 4, .type = ':', .data = (void *)&TestCaseArrayNested1[1] },
    { .length = 1, .raw_length = 4, .type = ':', .data = (void *)&TestCaseArrayNested1[2] }
};

static struct str_type_buffer NestedBuf2[2] = {
    { .length = 5, .raw_length = 8, .type = '+', .data = (void *)TestCaseArrayNested2[0] },
    { .length = 5, .raw_length = 8, .type = '-', .data = (void *)TestCaseArrayNested2[1] }
};

static struct resp_protocol_hdlr NestedArray1 = {
    .type = '*',
    .arr_size = 3,
    .arr_raw_length = 16,
    .buf_arr = NestedBuf1
};

static struct resp_protocol_hdlr NestedArray2 = {
    .type = '*',
    .arr_size = 2,
    .arr_raw_length = 20,
    .buf_arr = NestedBuf2
};

static struct resp_protocol_hdlr ExpectedNestedArray = {
    .type = '*',
    .arr_size = 2,
    .arr_raw_length = 40,
    .buf_arr = (struct str_type_buffer[]){ 
        { .length = 3, .raw_length = 16, .type = '*', .data = &NestedArray1 },
        { .length = 2, .raw_length = 20, .type = '*', .data = &NestedArray2 }
    }
};

static const struct test_case_array_nested TestCases_ArrayNested[] = {
    {
        "ParseNestedArray1",
        "*2\r\n*3\r\n:1\r\n:2\r\n:3\r\n*2\r\n+HELLO\r\n-WORLD\r\n",
        &ExpectedNestedArray,
        strlen("*2\r\n*3\r\n:1\r\n:2\r\n:3\r\n*2\r\n+HELLO\r\n-WORLD\r\n")
    }
};

static struct test_case_itf TestCasesArray[ARRAY_STR_MAX_TESTS + 1];
static int num_array_test_cases = 0;

TEST_GROUP(TestGroupParseFrameArray)
{

  void setup()
	{
      int i;

      for (i = 0; i < ARRAY_STR_MAX_TESTS; i++, num_array_test_cases++) {
          TestCasesArray[num_array_test_cases].name = TestCases_ArrayStr[i].name;
          TestCasesArray[num_array_test_cases].test_case_fn = test_parse_frame_array;
          TestCasesArray[num_array_test_cases].input = TestCases_ArrayStr[i].input;
          TestCasesArray[num_array_test_cases].expected_output = TestCases_ArrayStr[i].expected_out;
          TestCasesArray[num_array_test_cases].size = TestCases_ArrayStr[i].expected_size;
      }

      TestCasesArray[num_array_test_cases].name = TestCases_ArrayNested[0].name;
      TestCasesArray[num_array_test_cases].test_case_fn = &test_parse_frame_array_nested;
      TestCasesArray[num_array_test_cases].input = TestCases_ArrayNested[0].input;
      TestCasesArray[num_array_test_cases].expected_output = TestCases_ArrayNested[0].expected_out;
      TestCasesArray[num_array_test_cases].size = TestCases_ArrayNested[0].expected_size;
      num_array_test_cases++;
	}

	void teardown()
	{
	}

};

TEST(TestGroupParseFrameArray, TestParseFrameRunner)
{
    run_test_cases(TestCasesArray, num_array_test_cases);
}

static void run_encode_test_cases(const struct test_case_itf *test_cases, int num_cases)
{
    printf("\n");
    for (int i = 0; i < num_cases; i++) {
        const struct test_case_itf *tc = &test_cases[i];
        printf("Running test %s...\n", tc->name);
        tc->test_case_fn(tc->input, tc->expected_output, tc->size);
    }
}

/* Simple strings */
const char * simple_data = "OK";
const char * simple_error = "ERROR";
static struct str_type_buffer simple_buf = { .length = 2, .raw_length = 2, .type = '+', .data = (char *)simple_data};
static struct str_type_buffer error_buf = { .length = 5, .raw_length = 5, .type = '-',  .data = (char *)simple_error};
static struct resp_protocol_hdlr simple_hdlr = { .type = '+', .arr_size = 1, .arr_raw_length = simple_buf.raw_length, .buf_arr = &simple_buf };
static struct resp_protocol_hdlr error_hdlr = { .type = '-', .arr_size = 1, .arr_raw_length = error_buf.raw_length, .buf_arr = &error_buf };

char buffer[128];

void test_encode_frame(const void *input, const void *expected_out, long unsigned int expected_sz)
{
    int len;
    const char *expected_res = (const char *)expected_out;
    struct resp_protocol_hdlr *test_input = (struct resp_protocol_hdlr *)input;

    memset(buffer, 0, 128);
    len = encode_frame(buffer, 128, test_input);

    UNSIGNED_LONGS_EQUAL(expected_sz, len);
    STRCMP_EQUAL(buffer, expected_res);
}

static const struct test_case_itf TestCasesEncodeStr[] = {
    {"EncodeSimpleString",&test_encode_frame,  &simple_hdlr, "+OK\r\n", strlen("+OK\r\n")},
    {"EncodeErrorString", &test_encode_frame, &error_hdlr, "-ERROR\r\n", strlen("-ERROR\r\n")}
};

TEST_GROUP(TestGroupEncodeFrameSimple)
{
  void setup()
  {}

  void teardown()
  {}

};

TEST(TestGroupEncodeFrameSimple, TestEncodeFrameSimpleRunner)
{
    run_encode_test_cases((const struct test_case_itf *)TestCasesEncodeStr, sizeof(TestCasesEncodeStr) / sizeof(TestCasesEncodeStr[0]));
}

/* Integer strings */
int int_value = 1000;
struct str_type_buffer int_buf = { .length = sizeof(int), .raw_length = sizeof(int) , .type = ':', .data = (char *)&int_value};
struct resp_protocol_hdlr int_hdlr = { .type = ':', .arr_size = 1, .arr_raw_length = int_buf.raw_length, .buf_arr = &int_buf };

static const struct test_case_itf TestCasesEncodeInt[] = {
    {"EncodeInteger", &test_encode_frame,  &int_hdlr, ":1000\r\n", strlen(":1000\r\n")},
};

TEST_GROUP(TestGroupEncodeFrameInt)
{
  void setup()
  {}

  void teardown()
  {}

};

TEST(TestGroupEncodeFrameInt, TestGroupEncodeFrameIntRunner)
{
    run_encode_test_cases((const struct test_case_itf *)TestCasesEncodeInt, sizeof(TestCasesEncodeInt) / sizeof(TestCasesEncodeInt[0]));
}

/* Bulk strings */
const char *bulk_str = "hello";
struct str_type_buffer bulk_buf = { .length = 5, .raw_length = 5, .type = '$', .data = (char *)bulk_str };
struct resp_protocol_hdlr bulk_hdlr = { .type = '$', .arr_size = 1, .arr_raw_length = bulk_buf.raw_length, .buf_arr = &bulk_buf };

const char *empty_bulk_str = "";
struct str_type_buffer empty_bulk_buf = { .length = 0,  .raw_length = 5, .type = '$', .data = (char *)empty_bulk_str};
struct resp_protocol_hdlr empty_bulk_hdlr = { .type = '$', .arr_size = 1, .arr_raw_length = empty_bulk_buf.raw_length, .buf_arr = &empty_bulk_buf };

static const struct test_case_itf TestCasesEncodeBulk[] = {
    {"EncodeBulkStr", &test_encode_frame,  &bulk_hdlr, "$5\r\nhello\r\n", strlen("$5\r\nhello\r\n")},
    {"EncodeEmptyBulkStr", &test_encode_frame,  &empty_bulk_hdlr, "$0\r\n\r\n", strlen("$0\r\n\r\n")},
};

TEST_GROUP(TestGroupEncodeFrameBulk)
{
  void setup()
  {}

  void teardown()
  {}

};

TEST(TestGroupEncodeFrameBulk, TestGroupEncodeFrameBulkRunner)
{
    run_encode_test_cases((const struct test_case_itf *)TestCasesEncodeBulk, sizeof(TestCasesEncodeBulk) / sizeof(TestCasesEncodeBulk[0]));
}

/* Array Test Data (["foo", "bar"]) */
const char *arr_str1 = "foo";
const char *arr_str2 = "bar";
struct str_type_buffer arr_buf1 = { .length = 3, .raw_length = 3, .type = '$', .data = (char *)arr_str1};
struct str_type_buffer arr_buf2 = { .length = 3, .raw_length = 3, .type = '$', .data = (char *)arr_str2};
struct str_type_buffer arr_bufs[] = { arr_buf1, arr_buf2 };
struct resp_protocol_hdlr arr_hdlr = { .type = '*', .arr_size = 2, .arr_raw_length = 22, .buf_arr = arr_bufs };

const char *emtpy_array = "*0\r\n";
struct str_type_buffer empty_array_buf = { .length = -1, .raw_length = 4, .type = '*', .data = (char *)emtpy_array };
/* Empty Array */ 
struct resp_protocol_hdlr empty_arr_hdlr = { .type = '*', .arr_size = 0, .arr_raw_length = 4, .buf_arr = &empty_array_buf };

/* Array with Mixed Types (["OK", ":1000"]) */
STR_TYPE_CREATE_INSTANCE(simple_bulk, .length = 2, .raw_length = 2, .type = '$', .data = ((char *)simple_data));
struct str_type_buffer arr_mixed_bufs[] = { simple_bulk, int_buf };
struct resp_protocol_hdlr arr_mixed_hdlr = { .type = '*', .arr_size = 2, .arr_raw_length = 19, .buf_arr = arr_mixed_bufs };

static int val1 = 1, val2 = 2, val3 = 3;
struct str_type_buffer int_bufs[] = {
    {  .length = 1, .raw_length = 4, .type = ':', .data = (void *)&val1},
    {  .length = 1, .raw_length = 4, .type = ':', .data = (void *)&val2},
    {  .length = 1, .raw_length = 4, .type = ':', .data = (void *)&val3}
};
struct resp_protocol_hdlr nested_arr_3 = { .type = '*', .arr_size = 3, .arr_raw_length = 12, .buf_arr = int_bufs };

const char *hello_str = "HELLO";
const char *world_err = "WORLD";
struct str_type_buffer str_bufs[] = {
    {  .length = 5, .raw_length = 8, .type = '+', .data = (void *)hello_str},
    {  .length = 5, .raw_length = 8, .type = '-', .data = (void *)world_err}
};
struct resp_protocol_hdlr nested_arr_2 = { .type = '*', .arr_size = 2, .arr_raw_length = 16, .buf_arr = str_bufs};

struct str_type_buffer nested_arr[] = {
  {
      .length = 3,
      .raw_length = 12,
      .type = '*',
      .data = (void *)&nested_arr_3
  },
  {
      .length = 2,
      .raw_length = 16,
      .type = '*',
      .data = (void *)&nested_arr_2
  }
};

struct resp_protocol_hdlr arr_mixed_hdlr_2 = {
    .type = '*',
    .arr_size = 2,
    .arr_raw_length = nested_arr[0].raw_length + nested_arr[1].raw_length,
    .buf_arr = nested_arr
};

static const struct test_case_itf TestCasesEncodeArray[] = {
    {"EncodeArray", &test_encode_frame, &arr_hdlr, "*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n", strlen("*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n")},
    {"EcondeEmptyArray", &test_encode_frame, &empty_arr_hdlr, "*0\r\n", strlen("*0\r\n")},
    {"EncodeMixedArray", &test_encode_frame, &arr_mixed_hdlr, "*2\r\n$2\r\nOK\r\n:1000\r\n", strlen("*2\r\n$2\r\nOK\r\n:1000\r\n")}, 
    {
        "EncodeMixedArray2",
        &test_encode_frame,
        &arr_mixed_hdlr_2,
        "*2\r\n*3\r\n:1\r\n:2\r\n:3\r\n*2\r\n+HELLO\r\n-WORLD\r\n",
        strlen("*2\r\n*3\r\n:1\r\n:2\r\n:3\r\n*2\r\n+HELLO\r\n-WORLD\r\n")
    } 
};

TEST_GROUP(TestGroupEncodeFrameArray)
{
  void setup()
  {}

  void teardown()
  {}

};

TEST(TestGroupEncodeFrameArray, TestGroupEncodeFrameArrayRunner)
{
    run_encode_test_cases((const struct test_case_itf *)TestCasesEncodeArray, sizeof(TestCasesEncodeArray) / sizeof(TestCasesEncodeArray[0]));
}

