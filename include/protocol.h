#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <stdlib.h>

struct resp_protocol_hdlr {
    char type;
    long unsigned int size;
    void *data;
};

struct resp_protocol_hdlr *parse_frame(const char *buffer);

#endif // !_PROTOCOL_H
