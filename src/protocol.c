#include "protocol.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

struct resp_protocol_hdlr *create_protocol_handler() {
    struct resp_protocol_hdlr *hdlr = (struct resp_protocol_hdlr *)malloc(sizeof(struct resp_protocol_hdlr));
    if (!hdlr)
        return NULL;

    memset(hdlr, 0, sizeof(struct resp_protocol_hdlr));

    return hdlr;
}

static void free_array_str(void **array, long int size)
{
    if (array) {
        for (long int i = 0; i < size; i++) {
            if (array[i])
                free(array[i]);
        }

        free(array);
    }
}

void destroy_protocol_handler_data(struct resp_protocol_hdlr *hdlr)
{
    if (!hdlr)
        return;

    if (hdlr->type == '*' && hdlr->data.p_data_arr) {
        int array_size = 0;
        if (hdlr->data.p_data_arr[0]) {
            const char *buf = (const char *)hdlr->data.p_data_arr[0];
            array_size = strtol(buf + 1, NULL, 10);
        }

        if (array_size <= 0)
            free_array_str(hdlr->data.p_data_arr, 1);
        else
            free_array_str(hdlr->data.p_data_arr, array_size);

        hdlr->data.p_data_arr = NULL;
    } else {
        if (hdlr->data.p_data) {
            free(hdlr->data.p_data);
            hdlr->data.p_data = NULL;
        }
    }
}

void destroy_protocol_handler(struct resp_protocol_hdlr *hdlr) {
    destroy_protocol_handler_data(hdlr);

    free(hdlr);
    hdlr = NULL;
}

static bool is_digit(const char digit)
{
    return ((digit >= '0') && (digit <= '9')) ? true : false;
}

static void *_parse_int(const char *buffer, long int *size)
{
    void *result;
    int *p_int_result;
    char *str_result;
    const char *p_buf = buffer + 1;
    const char *end = strstr(buffer + 1, "\r\n");
    if (!end) {
        result = strdup("");
        *size = 0;
        goto out;
    }

    result = malloc(sizeof(int));
    if (!result) {
        result = NULL;
        *size = 0;
        goto out;
    } 

    p_int_result = (int *)result;
    *p_int_result = 0;
    while (is_digit(*p_buf)) {
        *p_int_result = (*p_int_result * 10) + (*p_buf - '0');
        p_buf++;
    }

    *size = (long int)(end - buffer) + 2;

out:
    return result;
}

static char *_parse_strings(const char *buffer, long int *tot_size, long int delim_sz)
{
    char *str_result;
    long int size;
    const char *end = strstr(buffer + 1, "\r\n");
    if (!end) {
        str_result = strdup("");
        *tot_size = 0;
        goto out;
    }

    size = (long int)(end - (buffer + 1));
    str_result = (char *)malloc(sizeof(char) * (size + 1));
    if (!str_result) {
        str_result = NULL;
        *tot_size = 0;
        goto out;
    }

    memset(str_result, 0, size + 1);
    memcpy(str_result, buffer + 1, size);
    str_result[size] = '\0';

    *tot_size = size + delim_sz + 1;

out:
    return str_result;
}

static long int __get_num_digits(const char *digits)
{
    long int result = 0;

    while (is_digit(*digits++))
        result++;

    return result;
}

static char *_parse_bulk_str(const char *buf, long int *count, long int *len)
{
    long unsigned int fixed_offset = 3;
    long int variable_offset = __get_num_digits(buf + 1);
    char *str_result;
    const char*delim = "\r\n";
    long int buf_len, tot_buf_len;

    *count = strtol(buf + 1, NULL, 10);
    if (*count < 0) {
        str_result = strdup("NULL");
        if (!str_result) {
            str_result = NULL;
        }

        *len = strlen("$-1\r\n");
    } else {
        long int total_jump = fixed_offset + variable_offset + *count;
        tot_buf_len = strlen(buf);

        const char *end = buf + total_jump;
        if ((strncmp(end, delim, strlen(delim)) || !(strstr(end, delim)))) {
            str_result = strdup("");
            if (!str_result)
                str_result = NULL;

            if (len)
                *len = 0;

            goto out;
        }

        if (len)
            *len = (long unsigned int)(end - buf) + strlen(delim);

        str_result = (char *)malloc(sizeof(char) * (*count + 1));
        if (!str_result) {
            str_result = NULL;
            goto out;
        }

        memset(str_result, 0, *count + 1);
        memcpy(str_result, buf + fixed_offset + variable_offset, *count);
        str_result[*count] = '\0';
    }

out:
    return str_result;
}

static void create_empty_str(void **array_str)
{
    *array_str = malloc(sizeof(void *));
    if (*array_str)
        *array_str = strdup("");
}

static void __parse_array_fail(void **array, long int size, long int *len)
{
    free_array_str(array, size);
    create_empty_str(array);
    *len = 0;
}

static void **_parse_array_str(const char *buffer, long int *len)
{
    long int array_size = strtol(buffer + 1, NULL, 10);
    void **parsed_array;
    long int bulk_size, i;
    const char *pos_next, *delim = "\r\n";
    

    if (array_size <= 0) {
        parsed_array = malloc(sizeof(void *));
        *parsed_array = (void *)strdup("NULL");
        if (len)
            *len = strlen(buffer);

        goto out;
    } else {
        parsed_array = malloc(sizeof(void *) * array_size);
        if (!parsed_array) {
            parsed_array = NULL;
            if (len)
                *len = 0;

            goto out;
        }

        const char *pos_now = strstr(buffer + 1, delim) + 2;
        if (!pos_now) {
            __parse_array_fail(parsed_array, array_size ,len);
            goto out;
        }

        long int len_now = 0;
        long int tot_len = (long unsigned int)(pos_now - buffer);
        for (i = 0; i < array_size; i++) {
            pos_next = strstr(pos_now, delim);
            if (!pos_next)
                break;

            switch (*pos_now) {
                case '+':
                case '-':
                    parsed_array[i] = _parse_strings(pos_now, &len_now, 2);
                    if (!len_now || !parsed_array[i]) {
                        __parse_array_fail(parsed_array, i ,len);
                        goto out;
                    }

                    pos_now = pos_next + 2;
                    break;
                case ':':
                    parsed_array[i] = _parse_int(pos_now, &len_now);
                    if (!len_now || !parsed_array[i]) {
                        __parse_array_fail(parsed_array, i ,len);
                        goto out;
                    }

                    pos_now = pos_next + 2;
                    break;
                case '$':
                    parsed_array[i] = _parse_bulk_str(pos_now, &bulk_size, &len_now);
                    if ((bulk_size >= 0 && !len_now) || !parsed_array[i]) {
                        __parse_array_fail(parsed_array, i ,len);
                        goto out;
                    }

                    if (bulk_size < 0) {
                        pos_now = pos_next + 2;
                    } else
                        pos_now = pos_next + 2*2 + bulk_size;
                    break;
                default:
                    break;
            }

            tot_len += len_now;
        }

        *len = tot_len;
    }

out:
    return parsed_array;
}

void parse_frame(const char *buffer, struct resp_protocol_hdlr *hdlr)
{
    const char *delim = "\r\n";
    char *str_result = NULL;
    char *end;
    long int result_len;
    long int bulk_count = 0;

    if (!hdlr)
        return;

    hdlr->type = *buffer;
    switch (hdlr->type) {
        case '+':
        case '-':
            hdlr->data.p_data = (void *)_parse_strings(buffer, &result_len, strlen(delim));
            if (!hdlr->data.p_data) {
                hdlr->data.p_data = NULL;
                return;
            }

            break;
        case ':':
            hdlr->data.p_data = (void *)_parse_int(buffer, &result_len);
            if (!hdlr->data.p_data) {
                hdlr->data.p_data = NULL;
                return;
            }

            break;
        case '$':
            hdlr->data.p_data = (void *)_parse_bulk_str(buffer, &bulk_count, &result_len);
            if (!hdlr->data.p_data) {
                hdlr->data.p_data = NULL;
                return;
            }

            break;
        case '*':
            hdlr->data.p_data_arr = (void **)_parse_array_str(buffer, &result_len);
            if (!hdlr->data.p_data_arr) {
                hdlr->data.p_data_arr = NULL;
                return;
            }

            break;
        default:
            return;
    }

    hdlr->size = result_len;
}
