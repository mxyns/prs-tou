#ifndef TOU_IO_H
#define TOU_IO_H

#include "tou.h"

void tou_write_packet(
        tou_socket* socket,
        char* header,
        int header_size,
        char* buffer,
        int size
);

// returns 1 if packets received
int tou_recv_packet(
        tou_conn* conn
);

int tou_send(
        tou_conn* conn
);

void tou_send_2(
        tou_conn* conn,
        char* buffer,
        int size,
        int await_ack
);

void tou_send_force_id(
        tou_conn* conn,
        char* buffer,
        int size,
        int id
);


#endif