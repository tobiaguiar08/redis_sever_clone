#include "protocol.h"
#include "stddef.h"
#include "string.h"
#include "stdlib.h"

char *parse_frame(const char *buffer)
{
    const char *delim = "\r\n";
    char *empty = "";
    char *result;
    char *end;
    size_t result_len;

    switch (*buffer) {
        case '+':
            end = strstr(buffer + 1, delim);
            if (!end)
                return empty;
            break;
        default:
            return empty;
    }

    result_len = strlen(buffer + 1) - strlen(end);
    result = (char *)malloc(sizeof(char) * (result_len + 1));
    if (!result)
        return empty;

    memcpy(result, buffer + 1, result_len);
    result[result_len] = '\0';

    return result;
}
