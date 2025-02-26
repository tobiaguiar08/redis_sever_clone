#ifndef _TEST_PROTOCOL_H
#define _TEST_PROTOCOL_H

struct test_case_itf {
    const char *name;
    void (*test_case_fn)(const void *input, const void *expected_out, long unsigned int size);
    const void *input;
    const void *expected_output;
    long unsigned int size;
};

#define STR_MAX_TESTS 7

struct test_case_str {
    const char *name;
    const char *input;
    const char *expected_out;
    long unsigned int expected_size;
};

#define INT_MAX_TESTS 4

struct test_case_int {
    const char *name;
    const char *input;
    const void *expected_out;
    long unsigned int expected_size;
};

#define ARRAY_STR_MAX_TESTS 5

struct test_case_array {
    const char *name;
    const char *input;
    const char **expected_out;
    long unsigned int expected_size;
};

#endif // !_TEST_PROTOCOL_H
