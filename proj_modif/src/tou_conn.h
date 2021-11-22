#ifndef TOU_CONN_H
#define TOU_CONN_H

#include "tou_datastructs.h"
#include "tou_window.h"

typedef struct tou_conn {
    tou_socket* socket;
    tou_socket* ctrl_socket;

    char filename[128];

    // reception buffer containing wire bytes
    tou_cbuffer* recv_work_buffer;

    // unused
    tou_cbuffer* send_work_buffer;

    // buffer the user reads from
    tou_cbuffer* in;
    tou_cbuffer* out;
    tou_cbuffer* ctrl_buffer;

    tou_window* recv_window;
    tou_window* send_window;
    uint32_t last_packet_id;
    long rtt;
} tou_conn;

tou_conn* tou_make_conn(
        tou_socket* ctrl_sock,
        tou_socket* socket
);

void tou_free_conn(
        tou_conn* conn
);

#endif