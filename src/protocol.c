#include "protocol.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

struct resp_protocol_hdlr *create_protocol_handler() {
    struct resp_protocol_hdlr *hdlr = (struct resp_protocol_hdlr *)malloc(sizeof(struct resp_protocol_hdlr));
    if (!hdlr)
        return NULL;

    memset(hdlr, 0, sizeof(struct resp_protocol_hdlr));

    return hdlr;
}

void destroy_protocol_handler_data(struct resp_protocol_hdlr *hdlr)
{
    if (!hdlr)
        return;

    if (hdlr->type == '*') {
        if (hdlr->arr_size > 0 && hdlr->buf_arr) {
            for (int i = 0; i < hdlr->arr_size; i++) {
                if (hdlr->buf_arr[i].data) {
                    free(hdlr->buf_arr[i].data);
                    hdlr->buf_arr[i].data = NULL;
                }
            }

            free(hdlr->buf_arr);
            hdlr->buf_arr = NULL;
        }
    } else {
        if (hdlr->buf_arr) {
            free(hdlr->buf_arr);
            hdlr->buf_arr = NULL;
        }
    }
}

void destroy_protocol_handler(struct resp_protocol_hdlr *hdlr) {
    destroy_protocol_handler_data(hdlr);

    free(hdlr);
    hdlr = NULL;
}

static void __create_empty_str(struct resp_protocol_hdlr *hdlr)
{
    hdlr->arr_size = 1;
    hdlr->buf_arr = calloc(hdlr->arr_size, sizeof(struct str_type_buffer));
    if (!hdlr->buf_arr) {
        hdlr->arr_size = 0;
        hdlr->arr_raw_length = 0;
        return;
    }

    hdlr->buf_arr[0].data = strdup("");
    hdlr->buf_arr[0].raw_length = 0;
    hdlr->buf_arr[0].length = 0;
    hdlr->arr_raw_length = 0;
}

static void __parse_fail(struct resp_protocol_hdlr *hdlr)
{
    destroy_protocol_handler_data(hdlr);
    __create_empty_str(hdlr);
}

static bool is_digit(const char digit)
{
    return ((digit >= '0') && (digit <= '9')) ? true : false;
}

static void _parse_int(const char *buffer, struct resp_protocol_hdlr *hdlr)
{
    struct str_type_buffer *str_result;
    int *p_int_result;
    const char *p_buf;
    const char *end;

    hdlr->arr_size = 1;
    hdlr->buf_arr = calloc(hdlr->arr_size, sizeof(struct str_type_buffer));
    if (!hdlr->buf_arr)
        return;

    str_result = &hdlr->buf_arr[0];
    p_buf = buffer + 1;
    end = strstr(buffer + 1, "\r\n");
    if (!end) {
        __parse_fail(hdlr);
        return;
    }

    str_result->data = calloc(1, sizeof(int));
    if (!str_result->data) {
        str_result->data = NULL;
        str_result->raw_length = 0;
        str_result->length = 0;
        return;
    } 

    p_int_result = (int *)str_result->data;
    *p_int_result = 0;
    while (p_buf < end) {
        if (!is_digit(*p_buf)) {
            __parse_fail(hdlr);
            return;
        }

        if (*p_int_result > (INT_MAX / 10) ||
            (*p_int_result == (INT_MAX / 10) && (*p_buf - '0') > (INT_MAX % 10))) {
            __parse_fail(hdlr);
            return;
        }

        *p_int_result = (*p_int_result * 10) + (*p_buf - '0');
        p_buf++;
    }

    str_result->length = (long int)(end - (buffer + 1));
    str_result->raw_length = str_result->length + 3;
    hdlr->arr_raw_length = str_result->raw_length;
}

static void _parse_strings(const char *buffer, struct resp_protocol_hdlr *hdlr)
{
    struct str_type_buffer *str_result;

    hdlr->arr_size = 1;
    hdlr->buf_arr = calloc(hdlr->arr_size, sizeof(struct str_type_buffer));
    if (!hdlr->buf_arr)
        return;

    str_result = &hdlr->buf_arr[0];
    const char *end = strstr(buffer + 1, "\r\n");
    if (!end) {
        __parse_fail(hdlr);
        return;
    }

    str_result->length = (long int)(end - (buffer + 1));
    str_result->data = calloc(str_result->length + 1, sizeof(char));
    if (!str_result->data) {
        str_result->data = NULL;
        str_result->raw_length = 0;
        str_result->length = 0;
        hdlr->arr_raw_length = str_result->raw_length;
        return;
    }

    memcpy(str_result->data, buffer + 1, str_result->length);
    str_result->data[str_result->length] = '\0';

    str_result->raw_length = str_result->length + 3;
    hdlr->arr_raw_length = str_result->raw_length;
}

static long int __get_num_digits(const char *digits)
{
    long int result = 0;

    while (is_digit(*digits++))
        result++;

    return result;
}

static bool valid_str(const char *buffer, char *end_ptr)
{
    long int addr_off = (long int)(end_ptr - (buffer + 1));
    bool valid_end = strncmp(end_ptr, "\r\n", 2) == 0 ? true : false;

    return ((addr_off == 0) || (valid_end == false)) ? false : true;
}

static void _parse_bulk_str(const char *buf, struct resp_protocol_hdlr *hdlr)
{
    struct str_type_buffer *str_result;
    long unsigned int fixed_offset;
    long int variable_offset;
    const char*delim;
    long int buf_len;
    char *save_ptr;

    hdlr->arr_size = 1;
    hdlr->buf_arr = calloc(hdlr->arr_size, sizeof(struct str_type_buffer));
    if (!hdlr->buf_arr)
        return;

    str_result = &hdlr->buf_arr[0];
    fixed_offset = 3;
    variable_offset = __get_num_digits(buf + 1);
    delim = "\r\n";

    str_result->length = strtol(buf + 1, &save_ptr, 10);

    if (!valid_str(buf, save_ptr)) {
        __parse_fail(hdlr);
        return;
    }

    if (str_result->length < 0) {
        str_result->data = strdup("NULL");
        if (!str_result->data)
            str_result->data = NULL;

        str_result->raw_length = strlen("$-1\r\n");
    } else {
        long int total_jump = fixed_offset + variable_offset + str_result->length;

        const char *end = buf + total_jump;
        if ((strncmp(end, delim, strlen(delim)) || !(strstr(end, delim)))) {
            __parse_fail(hdlr);
            return;
        }

        str_result->raw_length = (int)(end - buf) + strlen(delim);

        str_result->data = calloc(str_result->length + 1, sizeof(char));
        if (!str_result->data) {
            str_result->data = NULL;
            str_result->raw_length = 0;
            str_result->length = 0;
            hdlr->arr_raw_length = str_result->raw_length;
            return;
        }

        memcpy(str_result->data, buf + fixed_offset + variable_offset, str_result->length);
        str_result->data[str_result->length] = '\0';
    }

    hdlr->arr_raw_length = str_result->raw_length;
}

static void _parse_array_str(const char *buffer, struct resp_protocol_hdlr *hdlr)
{
    long int array_size;
    long int bulk_size, i;
    const char *pos_next, *delim = "\r\n";
    char *end_size_ptr;
    struct resp_protocol_hdlr subresult;
    
    array_size = strtol(buffer + 1, &end_size_ptr, 10);
    if (!valid_str(buffer, end_size_ptr)) {
        __create_empty_str(hdlr);
        return;
    }

    if (array_size <= 0) {
        hdlr->arr_size = 1;
        hdlr->buf_arr = calloc(hdlr->arr_size, sizeof(struct str_type_buffer));
        if (!hdlr->buf_arr) {
            hdlr->arr_size = 0;
            return;
        }

        hdlr->buf_arr[0].data = array_size == 0 ? strdup("") : strdup("NULL");
        hdlr->buf_arr[0].raw_length = array_size == 0 ? strlen("*0\r\n") : strlen("*-1\r\n");
        hdlr->buf_arr[0].length = -1;
        hdlr->arr_raw_length = hdlr->buf_arr[0].raw_length;

        return;
    } else {
        hdlr->arr_size = array_size;
        hdlr->buf_arr = calloc(hdlr->arr_size, sizeof(struct str_type_buffer));
        if (!hdlr->buf_arr) {
            hdlr->buf_arr = NULL;
            hdlr->arr_size = 0;
            hdlr->arr_raw_length = 0;

            return;
        }

        const char *pos_now = strstr(buffer + 1, delim) + 2;
        if (!pos_now) {
            __parse_fail(hdlr);
            return;
        }

        long int len_now = 0;
        long int tot_len = (long unsigned int)(pos_now - buffer);
        for (i = 0; i < array_size; i++) {
            memset(&subresult, 0, sizeof(subresult));
            pos_next = strstr(pos_now, delim);
            if (!pos_next)
                break;

            parse_frame(pos_now, &subresult);
            if (!subresult.buf_arr || !subresult.buf_arr[0].raw_length || !subresult.buf_arr[0].data) {
                __parse_fail(hdlr);
                return;
            }

            if((!subresult.buf_arr[0].raw_length && subresult.buf_arr[0].length >= 0) ||  !subresult.buf_arr[0].data) {
                __parse_fail(hdlr);
                return;
            }

            hdlr->type = subresult.type;
            hdlr->buf_arr[i].length = subresult.buf_arr[0].length;
            hdlr->buf_arr[i].raw_length = subresult.buf_arr[0].raw_length;
            if (subresult.buf_arr[0].length >= 0) {
                hdlr->buf_arr[i].data = calloc(1, sizeof(char) * subresult.buf_arr[0].length);
                memcpy(hdlr->buf_arr[i].data, subresult.buf_arr[0].data, hdlr->buf_arr[i].length);
            } else {
                hdlr->buf_arr[i].data = calloc(1, sizeof(char) * subresult.buf_arr[0].raw_length);
                memcpy(hdlr->buf_arr[i].data, subresult.buf_arr[0].data, hdlr->buf_arr[i].raw_length);
            }

            pos_now = pos_now + subresult.buf_arr[0].raw_length;             
      
            tot_len += subresult.buf_arr[0].raw_length;
        }

        hdlr->arr_raw_length = tot_len;
    }
}

void parse_frame(const char *buffer, struct resp_protocol_hdlr *hdlr)
{
    const char *delim = "\r\n";
    long int result_len;
    long int bulk_count = 0;

    if (!hdlr)
        return;

    hdlr->type = *buffer;
    switch (hdlr->type) {
        case '+':
        case '-':
            _parse_strings(buffer, hdlr);
      
            break;
        case ':':
            _parse_int(buffer, hdlr);
      
            break;
        case '$':
            _parse_bulk_str(buffer, hdlr);
      
            break;
        case '*':
            _parse_array_str(buffer, hdlr);

            break;
        default:
            __create_empty_str(hdlr);
            break;
    }
}
