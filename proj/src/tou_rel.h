#ifndef TOU_REL_H
#define TOU_REL_H

#include "tou.h"

/*
    packet_id (char)
    ack_count (uint32_t)
    ---
    ack_list[ack_count] (ack_id*=uint32_t*)
*/
#define TOU_ID_PKT_ACK (2)
#define TOU_LEN_PKT_ACK (TOU_LEN_BASEHEADER + sizeof(uint32_t))
#define TOU_HOFFSET_PKT_ACK_ACK_COUNT (TOU_LEN_BASEHEADER) 
#define TOU_HOFFSET_PKT_ACK_ACK_LIST (TOU_HOFFSET_PKT_ACK_ACK_COUNT + sizeof(uint32_t))
#define TOU_PKT_ACK_MAX_ACK_COUNT ((TOU_DEFAULT_MSS - TOU_LEN_PKT_ACK) / sizeof(uint32_t))



// TODO find a way to effectively pack ACKs together, using timers or every X packet send ack ?
// TODO find why we can't get multiple packets at once with tou_recv_packet : this is what parses packets and then tries to accept them
// TODO ack await timer expire when checking received acks : need to add timestamp to tou_packet_dtp
// TODO store expected ack id (last contiguous received ack) and always send it in ack on top of the ack list so that i have sequential ack + selective ack
//      helps reduce ack count (can maybe optimize to remove ack_list when not needed)
typedef uint32_t ack_id_t;
typedef struct {

    char packet_id;
    uint32_t ack_count;
    ack_id_t* ack_list;
} tou_packet_ack;


tou_packet_ack tou_make_packet_ack(
    ack_id_t* ack_list
);

void tou_send_ack(
    tou_conn* conn,
    tou_packet_ack* packet
);

void tou_recv_ack(
    tou_conn* conn
);

void tou_acknowledge_packets(
    tou_conn* conn,
    ack_id_t* ack_list,
    uint32_t ack_count
);

#endif