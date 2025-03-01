#ifndef _PROTOCOL_H
#define _PROTOCOL_H

struct str_type_buffer {
    char *data;
    int length;
    int raw_length;
};

struct resp_protocol_hdlr {
    char type;
    int arr_size;
    int arr_raw_length;
    struct str_type_buffer *buf_arr;
};

struct resp_protocol_hdlr *create_protocol_handler(void);
void destroy_protocol_handler(struct resp_protocol_hdlr *hdlr);
void destroy_protocol_handler_data(struct resp_protocol_hdlr *hdlr);

void parse_frame(const char *buffer, struct resp_protocol_hdlr *hdlr);

#endif // !_PROTOCOL_H
