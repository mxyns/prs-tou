#include "tou_io.h"
#include "tou_utils.h"
#include "tou_datastructs.h"
#include "tou_handshake.h"
#include "tou_packet.h"
#include "tou.h"
#include "tou_rel.h"
#include <stdlib.h>

// write header and buffer to socket
void tou_write_packet(
    tou_socket* socket,
    char* header,
    int header_size,
    char* buffer, 
    int size
) {

    static char fullpacket[TOU_DEFAULT_MSS];
    memset(fullpacket, 0, TOU_DEFAULT_MSS);

    // printf("ASSERT %d + %d <= %ld\n", header_size, size, TOU_DEFAULT_MSS);
    printf(TOU_DEFAULT_MSS >= header_size + size ? "true\n" : "false\n");

    memcpy(fullpacket, header, header_size);
    memcpy(fullpacket + header_size, buffer, size);
    // compact_print_buffer(fullpacket, TOU_DEFAULT_MSS);

    if (sendto(socket->fd, fullpacket, header_size + size, 0, 
            socket->peer_addr, socket->peer_addr_len) < 0) {
        printf("[tou][tou_write_packet] can't write packet\n");
    } else {
        printf("[tou][tou_write_packet] %d bytes packet wrote\n", header_size + size);
    }
}


int tou_recv_packet_parse(
    tou_conn* conn
) {
    char* pkt_type_ptr;
    if ((pkt_type_ptr = tou_cbuffer_peek(conn->recv_work_buffer, 0)) == NULL) {
        printf("[tou][tou_recv_packet] CANT PEEK\n");
        return 0;
    }
    char pkt_type = *pkt_type_ptr;
    printf("pkt_type %d\n", pkt_type);

    if(pkt_type == TOU_ID_DTP) {

        printf("type==dtp\n");
        int new = tou_parse_dtp(conn);
        if (!new) return 0;

        int accepted = tou_window_accept(conn, conn->recv_window, conn->in);
        printf(
            "[tou][tou_recv_packet] parsed 1 packet, accepted %d. next expected=%d, current is id=%d\n", 
            accepted, 
            conn->recv_window->expected, 
            TOU_SLL_ISEMPTY(conn->recv_window->list) ? -1 : ((tou_packet_dtp*)conn->recv_window->list->head->val)->packet_id
        );

        return new;

    } else if ( // handshake packet
        pkt_type == TOU_ID_SYN || \
        pkt_type == TOU_ID_SYNACK || \
        pkt_type == TOU_ID_HANDSHAKE_ACK \
    ) {

        printf("[tou][tou_recv_packet] NOT SUPPOSED TO HAVE HANDSHAKE %d PACKET TYPE.", pkt_type);
        exit(1);
        return 0;

    } else {
        printf("[tou][tou_recv_packet] NOT SUPPOSED TO HAVE %d PACKET TYPE.", pkt_type);
        exit(1);
        return 0;
    }
}

// reads from socket into recv_work_buffer
// peek acquired data for header and packet sizes
// parse dtp packets push it to recv_window
// accept (if possible) expected packets from recv_window
// returns amout of new packets parsed
int tou_recv_packet(
    tou_conn* conn
) {

    // read biggest amount available
    int count = tou_cbuffer_read(conn->socket, conn->recv_work_buffer, conn->recv_work_buffer->cap);
    printf("[tou][tou_recv_packet] read %d from socket\n", count);
    
    // TODO drop packets when full
    if (conn->recv_window->list->cap-conn->recv_window->list->count == 0) {
        printf("[tou][tou_recv_packet] recv_window is full\n");
        return 0;
    }
    
    // try to parse as much packets as possible
    while (conn->recv_work_buffer->cnt > 0) {
        tou_recv_packet_parse(conn);
    }
}

// user function
// send a packet while forcing its packet_id(uint32_t) value
void tou_send_force_id(
    tou_conn* conn,
    char* buffer,
    int size,
    int id
) {
    int previous_id = conn->last_packet_id;
    conn->last_packet_id = id - 1;
    tou_send_2(conn, buffer, size, 1);
    conn->last_packet_id = previous_id;
}


// user function
// send content of buffer[0:size] 
// fill send_window, send, and wait for ack before repeating
void tou_send_2(
    tou_conn* conn,
    char* buffer,
    int size,
    int await_ack
) {

   int MSS = TOU_DEFAULT_MSS /* max packet size (header included) */ - TOU_LEN_DTP;
   printf("[tou][tou_send] MSS=%d\n", MSS);

    int written = 0;
    while (size - written > 0) {

        // write maximum to out buffer
        int new_write = tou_cbuffer_insert(conn->out, buffer + written, size - written);
        written += new_write;

        // process out buffer into packets => send window
        tou_send(conn);
        printf("[tou][tou_send_2] buffer sent\n");

        if (await_ack) {
            // await for some acks
            printf("[tou][tou_send_2] awaiting ack\n");
            tou_recv_ack(conn);
        }
    }
}

// pops data from conn->out wraps it into tou_packet_dtp, add it to send_window and send it
// returns number of packets made and pushed to send_window
int tou_send(
    tou_conn* conn
) {

    int MSS = TOU_DEFAULT_MSS /* max packet size (header included) */ - TOU_LEN_DTP;
    // printf("[tou][tou_send] MSS=%d\n", MSS);
   
    int size = conn->out->cnt;
    char payload[MSS];
    tou_sll* send_list = conn->send_window->list;
    int new_packets = 0;
    while((size = conn->out->cnt) > 0 && send_list->count < send_list->cap) {

       /*
           packet_type(char)
           packet_id(uint32_t) unique id used for ordering and acking
           data_packet_size(uint32_t)
       */

       conn->last_packet_id++;
       int id = conn->last_packet_id;

       memset(payload, 0, MSS);
       int popped = tou_cbuffer_pop(conn->out, payload, MSS);
       if (popped <= 0) {
           printf("[tou][tou_send] no more data in out buffer to send\n");
           return new_packets;
       }
       
       tou_packet_dtp* pkt = tou_sll_insert(send_list, id);
       if (pkt == NULL) {
           printf("[tou][tou_send] send window is full\n");
           return new_packets;
       }

        char header[6];
        tou_packet_set_header(pkt, header, id, payload, popped);
        tou_write_packet(conn->socket, header, 6, payload, popped);
        tou_packet_set_expiration(pkt, tou_time_ms() + TOU_DEFAULT_ACK_TIMEOUT_MS);

        new_packets++;
    }

    return new_packets;
}