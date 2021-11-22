#ifndef TOU_WINDOW_H
#define TOU_WINDOW_H

struct tou_conn;

#include "tou_conn.h"
#include "tou_datastructs.h"

typedef struct tou_window {
    tou_sll* list;
    uint32_t expected;
} tou_window;

tou_window* tou_make_window(
        int window_size,
        int max_packet_size,
        uint32_t expected
);

void tou_free_window(
        tou_window* window
);

int tou_window_accept(
        struct tou_conn* conn,
        tou_window* window,
        tou_cbuffer* stream
);

#endif