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

struct resp_protocol_hdlr *create_protocol_handler(void);
void destroy_protocol_handler(struct resp_protocol_hdlr *hdlr);
void destroy_protocol_handler_data(struct resp_protocol_hdlr *hdlr);

void parse_frame(const char *buffer, struct resp_protocol_hdlr *hdlr);

#endif // !_PROTOCOL_H
