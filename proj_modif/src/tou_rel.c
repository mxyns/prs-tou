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
    tou_write_packet(conn->ctrl_socket, header, TOU_LEN_PKT_ACK, (char*) ack->ack_list,
                     ack->ack_count * sizeof(ack_id_t));

    printf("[tou][tou_send_ack] sent ack for %d packets : [ ", ack->ack_count);
    for (int i = 0; i < ack->ack_count; i++) printf("%d ", ack->ack_list[i]);
    printf("]\n");
}

void tou_estimate_rtt(
    tou_conn* conn,
    long* rtts,
    int rttscnt
) {

    const double k = 0.9;
    for (int i = 0 ; i < rttscnt; i++) { 
        TOU_DEBUG(printf("[tou][tou_estimate_rtt] packet rtt %lf\n", rtts[i]));
        conn->rtt = (double)(k * conn->rtt + (1-k) * rtts[i]);
        TOU_DEBUG(printf("[tou][tou_estimate_rtt] new rtt %lf\n", conn->rtt));
    }
}

// if some packets are acknowledged, return new seq
// return -1 if same ack as last ack seq
// return 0 if its just a late ack packet
int tou_acknowledge_packets(
        tou_conn* conn,
        int seq
) {
   
    int expected = (int)conn->send_window->expected;
    if (seq >= expected) {
        tou_sll_node* removed[conn->send_window->list->cap];
        int result = tou_sll_remove_under(conn->send_window->list, seq, removed);

        int old_expected = expected;
        conn->send_window->expected = seq + 1;

        long rtts[result];
        long now = tou_time_ms();
        for (int i = 0 ; i < result; i++) {
            rtts[i] = now - ((tou_packet_dtp*)removed[i]->val)->timestamp;
        }
        tou_estimate_rtt(conn, rtts, result);


        TOU_DEBUG(
            printf("[tou][tou_acknowledge_packet] from %d removed %d packets. expecting : %d\n", old_expected, result,
                         (int) conn->send_window->expected);
        );

        return seq;

    } else if (seq == expected - 1) {

        TOU_DEBUG(
            printf("[tou][tou_acknowledge_packet] some packets were dropped, last_acked = %d\n", seq);
        );
        
        return -1;

    } else { // less than expected but not last (a late ack : ack 1 -> ack 3 -> ack 2 )
        TOU_DEBUG(printf("[tou][tou_acknowledge_packet] %d late ack, no worries\n", seq));
        return 0;
    }
}


// await for ACKs on ctrl_socket and Acknowledge packets ids given in received ACK packets
// read from ctrl_socket into ctrl_buffer, peek sizes, parse into tou_packet_ack
// acknowledge packet ids in send_window

static char ack_buff[TOU_LEN_PKT_ACK];

// return -1 for error, 0 for succcess
// set *ack_count to number of times last ack was acknowledged
int tou_recv_ack(
        tou_conn* conn,
        int* ack_count
) {
    tou_socket* ack_socket = conn->socket;

    int read = tou_cbuffer_read(ack_socket, conn->recv_work_buffer, conn->recv_work_buffer->cap);
    if (read < 0) {
        TOU_DEBUG(printf("[tou][tou_recv_ack] can't read from socket\n"));
        perror("read error: ");
        return -1;
    }

    TOU_DEBUG(printf("[tou][tou_recv_ack] read %d\n", read));
    // not enough to parse header
    if (conn->recv_work_buffer->cnt < TOU_LEN_PKT_ACK + 1) {
        TOU_DEBUG(printf("[tou][tou_recv_ack] not enough data to parse header yet, have %d < %d bytes\n",
                         conn->recv_work_buffer->cnt, TOU_LEN_PKT_ACK + 1));
        return -1;
    }

    TOU_DEBUG(printf("[tou][tou_recv_ack] got %d in buffer\n", conn->recv_work_buffer->cnt));

    const int pkt_size = TOU_LEN_PKT_ACK + 1;
    while (conn->recv_work_buffer->cnt >= pkt_size) {

        int n = tou_cbuffer_pop(conn->recv_work_buffer, ack_buff, pkt_size);
        if (n < pkt_size) {
            TOU_DEBUG(
                    printf("[tou][tou_recv_ack] lost %d bytes while trying to pop %d bytes ACK packet\n", pkt_size - n,
                           pkt_size));
            return -1;
        }
        TOU_DEBUG(
                printf("[tou][tou_recv_ack] ack packet dump :\n");
                compact_print_buffer(ack_buff, pkt_size);
        );

        char* seq = ack_buff + 3;
        int seq_val = (seq[0] - 48) * 100000
                      + (seq[1] - 48) * 10000
                      + (seq[2] - 48) * 1000
                      + (seq[3] - 48) * 100
                      + (seq[4] - 48) * 10
                      + (seq[5] - 48) * 1;

        TOU_DEBUG(
            printf("recv seq value %d\n", seq_val);
        );

        int new_seq = tou_acknowledge_packets(conn, seq_val);
        if (new_seq > 0) {
            *ack_count = 0;
            TOU_DEBUG(
                printf("acked more packets until %d, next is %d\n", new_seq, conn->send_window->expected);
            );
        } else if (new_seq < 0) {
            *ack_count = *ack_count + 1;
            TOU_DEBUG(printf("duplicated ack for %d\n", conn->send_window->expected - 1));
        } else {
            TOU_DEBUG(printf("old ack %d\n", new_seq));
        }
    }

    return 0;
}