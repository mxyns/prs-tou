#include "tou_conn.h"
#include "tou_rel.h"
#include "tou_utils.h"
#include "tou_io.h"
#include <stdlib.h>

tou_packet_ack tou_make_packet_ack(
    ack_id_t* ack_list
) {

    tou_packet_ack ack;
    ack.packet_id = TOU_ID_PKT_ACK;
    ack.ack_count = 0;
    ack.ack_list = ack_list;

    return ack;
}

void tou_send_ack(
    tou_conn* conn,
    tou_packet_ack* ack
) {

    printf("[tou_send_ack] IN THIS PROJECT I SHOUD NEVER GET TO THIS PART OF THE CODE \n");
    exit(1);
    return; 

    if (ack->ack_count == 0)
        return; 

    char header[TOU_LEN_PKT_ACK];
    TOU_ARR_INS(header, 0, char, TOU_ID_PKT_ACK);
    TOU_ARR_INS(header, TOU_HOFFSET_PKT_ACK_ACK_COUNT, uint32_t, ack->ack_count);

    // ack dont need to be in a window, direct write to ctrl_socket
    tou_write_packet(conn->ctrl_socket, header, TOU_LEN_PKT_ACK, (char*)ack->ack_list, ack->ack_count * sizeof(ack_id_t));

    printf("[tou][tou_send_ack] sent ack for %d packets : [ ", ack->ack_count);
    for (int i = 0; i < ack->ack_count; i++) printf("%d ", ack->ack_list[i]);
    printf("]\n");
}

int tou_acknowledge_packets(
    tou_conn* conn,
    int seq
) {
    
    int removed = tou_sll_remove_under(conn->send_window->list, seq);
    conn->send_window->expected = seq + 1;

    printf("[tou][tou_acknowledge_packet] removed %d packets\n", removed);

    return removed;
}


// await for ACKs on ctrl_socket and Acknowledge packets ids given in received ACK packets
// read from ctrl_socket into ctrl_buffer, peek sizes, parse into tou_packet_ack
// acknowledge packet ids in send_window

char ack_buff[TOU_LEN_PKT_ACK];
void tou_recv_ack(
    tou_conn* conn
) {

    tou_socket* ack_socket = conn->socket;
    
    int read = tou_cbuffer_read(conn->socket, conn->recv_work_buffer, conn->recv_work_buffer->cap);
    if (read < 0) {
        printf("[tou][tou_recv_ack] can't read from ctrl socket\n");
        return;
    }

    printf("[tou][tou_recv_ack] read %d\n", read);
    // not enough to parse header
    if (conn->recv_work_buffer->cnt < TOU_LEN_PKT_ACK + 1) {
        printf("[tou][tou_recv_ack] not enough data to parse header yet, have %d < %ld bytes\n", conn->recv_work_buffer->cnt, TOU_LEN_PKT_ACK + 1);
        return ;
    }
    
    printf("[tou][tou_recv_ack] got %d in buffer\n", conn->recv_work_buffer->cnt);
    const int pkt_size = TOU_LEN_PKT_ACK + 1;
    while (conn->recv_work_buffer->cnt >= TOU_LEN_PKT_ACK + 1) {

        int n = tou_cbuffer_pop(conn->recv_work_buffer, ack_buff, pkt_size);
        if (n < pkt_size) {
            printf("[tou][tou_recv_ack] lost %d bytes while trying to pop %d bytes ACK packet\n", pkt_size - n, pkt_size);
            return ;
        }

        printf("[tou][tou_recv_ack] ack packet dump :\n");
        compact_print_buffer(ack_buff, pkt_size);

        char* seq = ack_buff + 3;
        int seq_val = (seq[0] - 48) * 100000
                + (seq[1] - 48) * 10000
                + (seq[2] - 48) * 1000
                + (seq[3] - 48) * 100
                + (seq[4] - 48) * 10
                + (seq[5] - 48) * 1;
        
        printf("recv seq value %d\n", seq_val);

        int acknowledged = tou_acknowledge_packets(conn, seq_val);
        if (acknowledged > 0) {
            printf("acked %d more packets, next is %d\n", acknowledged, conn->send_window->expected);
        }
    }
}