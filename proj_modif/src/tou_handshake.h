#ifndef TOU_HANDSHAKE_H
#define TOU_HANDSHAKE_H

#include "tou.h"

#define TOU_VALUE_HANDSHAKE_ACK_CHECK 69

#define TOU_FLAG_SYN (1)
#define TOU_FLAG_HANDSHAKE_ACK (1 << 1)
#define TOU_FLAG_SYNACK (TOU_FLAG_HANDSHAKE_ACK | TOU_FLAG_SYN)

/*
    pakcet_id(char)
    flags(char)
*/
#define TOU_ID_SYN (1) // handshake id 
#define TOU_LEN_SYN (TOU_LEN_BASEHEADER + 3) // SYN \0

/*
    pakcet_id(char)
    flags(char)
    data_port(uint16_t)
*/
#define TOU_ID_SYNACK (1) // handshake id 
#define TOU_LEN_SYNACK (TOU_LEN_BASEHEADER + 7 + 4) // SYN-ACK0123 \0

/*
    pakcet_id(char)
    flags(char)
    check_value(char) // TODO 69^SYN->rnd
*/
#define TOU_ID_HANDSHAKE_ACK (1) // handshake id 
#define TOU_LEN_HANDSHAKE_ACK (TOU_LEN_BASEHEADER + 3) // ACK

int tou_send_handshake_syn(
    tou_socket* sock
);

int tou_recv_handshake_syn(
    tou_socket* sock
);

tou_socket* tou_send_handshake_synack(
    tou_socket* sock
);

uint16_t tou_recv_handshake_synack(
    tou_socket* sock
);

int tou_send_handshake_ack(
    tou_socket* sock
);

int tou_recv_handshake_ack(
    tou_socket* sock,
    int timeout // TODO use
);

#endif