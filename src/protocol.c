#include "protocol.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

static bool is_digit(const char digit)
{
    return ((digit >= '0') && (digit <= '9')) ? true : false;
}

static int *_parse_int(const char *buf_to_copy)
{
    int *p_int_result = (int *)malloc(sizeof(int));
    const char *p_buf = buf_to_copy + 1;

    *p_int_result = 0;
    while (is_digit(*p_buf)) {
        *p_int_result = (*p_int_result * 10) + (*p_buf - '0');
        p_buf++;
    }

    return p_int_result;
}

static char *_parse_strings(const char *buf_to_copy, size_t tot_size, size_t delim_sz)
{
    char *str_result = (char *)malloc(sizeof(char) * tot_size);
    if (!str_result)
        return NULL;

    memcpy(str_result, buf_to_copy + 1, tot_size - delim_sz - 1);
    str_result[tot_size - delim_sz - 1] = '\0';

    return str_result;
}

struct resp_protocol_hdlr *parse_frame(const char *buffer)
{
    const char *delim = "\r\n";
    char *str_result = NULL;
    struct resp_protocol_hdlr *result = NULL;
    char *end;
    long unsigned int result_len;

    result = (struct resp_protocol_hdlr *)malloc(sizeof(struct resp_protocol_hdlr));
    if (!result) {
        result = NULL;
        goto err_mem;
    }

    result->data = NULL;
    result->size = 0;
    result->type = 0;

    end = strstr(buffer + 1, delim);
    if (!end) {
        result->data = (void *)strdup("");
        goto err_parse;
    }

    result_len = (long unsigned int)(end - buffer) + strlen(delim);
    result->type = *buffer;
    switch (result->type) {
        case '+':
        case '-':
            result->data = (void *)_parse_strings(buffer, result_len, strlen(delim));
            break;
        case ':':
            result->data = (void *)_parse_int(buffer);
            break;
        default:
            goto err_parse;
            break;
    }
    
    if (!result->data)
        goto err_data;

    result->size = result_len;
    return result;

err_data:
err_parse:
err_mem:
    return result;
}
