#ifndef _PROTOCOL_H
#define _PROTOCOL_H

struct resp_protocol_hdlr {
    char type;
    long unsigned int size;
    union {
        void *p_data;
        void **p_data_arr;
    } data;
};

struct resp_protocol_hdlr *parse_frame(const char *buffer);

#endif // !_PROTOCOL_H
