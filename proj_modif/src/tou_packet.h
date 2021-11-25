#ifndef TOU_PACKET_H
#define TOU_PACKET_H

#include <stdint.h>
#include "tou_conn.h"

/*
    packet_type(char)
    packet_id(uint32_t) unique id used for ordering and acking
    data_packet_size(uint32_t) size of payload
*/
#define TOU_ID_DTP (3) // datapacket id
#define TOU_HOFFSET_DTP_PACKET_ID (TOU_LEN_BASEHEADER)
#define TOU_HOFFSET_DTP_PACKET_SIZE (TOU_LEN_BASEHEADER + sizeof(uint32_t))
#define TOU_HOFFSET_DTP_PACKET_DATA (TOU_LEN_BASEHEADER + sizeof(uint32_t) + sizeof(uint32_t))
#define TOU_LEN_DTP (TOU_LEN_BASEHEADER + 6)

/*
    packet_type(char)
    packet_id(uint32_t) unique id used for ordering and acking
    data_packet_size(uint32_t) size of payload
*/
typedef struct {

    uint32_t packet_id;
    char acked;
    long ack_expire;
    long timestamp;

    // number of payload bytes after data_start
    // ie last payload byte is at buffer + data_start + data_packet_size - 1
    //  header       payload
    // [ 1 2 3 | 10 11 12 13 14 15 ]
    // data_start = 3 // data_packet_size = 6 => last byte is at buffer + 3 + 6 - 1 = buffer + 8
    uint32_t data_packet_size;

    // position of first data byte in buffer
    // ie if data was a buffer containing only the payload we'd have data[0] = *(buffer + data_start)
    int data_start;
    int buffer_size;
    char* buffer;
} tou_packet_dtp;

tou_packet_dtp* tou_make_packet_dtp(
        uint32_t packet_id,
        int buffer_size,
        char* buffer
);

int tou_parse_dtp(
        tou_conn* conn
);

int tou_packet_dtp_tostream(
        tou_window* window,
        tou_packet_dtp* packet,
        tou_cbuffer* stream
);

void tou_packet_dtp_dump(
        tou_packet_dtp packet,
        int dump_buffer
);

void tou_packet_dtp_reset(
        tou_packet_dtp* packet
);

void tou_packet_set_header(
        tou_packet_dtp* pkt,
        char* header,
        int id,
        char* payload,
        uint32_t data_packet_size
);

void tou_packet_set_expiration(
        tou_packet_dtp* pkt,
        long now,
        long timeout
);

#endif