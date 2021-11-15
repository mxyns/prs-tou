#include "tou_conn.h"
#include "tou_rel.h"
#include "tou_utils.h"
#include "tou_io.h"

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

void tou_acknowledge_packets(
    tou_conn* conn,
    ack_id_t* ack_list,
    uint32_t ack_count
) {

    printf("[tou][tou_acknowledge_packet] ack_list = [ ");
    for (int i = 0; i < ack_count; i++) {
        printf("%d ", ack_list[i]);
    }
    printf("]\n");

    tou_packet_dtp* removed_pkts[ack_count];
    memset(removed_pkts, 0, (int)ack_count * sizeof(tou_packet_dtp*));

    printf("[tou][tou_acknowledge_packet] removed_pkts = [ ");
    for (int i = 0; i < ack_count; i++) {
        printf("%p ", removed_pkts[i]);
    }
    printf("]\n");

    int removed = tou_sll_remove_keys(conn->send_window->list, ack_list, (void**)removed_pkts, sizeof(tou_packet_dtp*), ack_count);
    printf("[tou][tou_acknowledge_packet] removed_pkts = [ ");

    for (int i = 0; i < ack_count; i++) {
        printf("%p ", removed_pkts[i]);
    }
    printf("]\n");

    if (removed > ack_count) {
        printf("[tou][tou_acknowledge_packet] removed some duplicates\n");
    }

    printf("[tou][tou_acknowledge_packet] removed %d packets : [ ", removed);
    for (int i = 0; i < ack_count; i++) {
        tou_packet_dtp* pkt = removed_pkts[i];
        if (pkt != NULL) {
            printf("%d ", pkt->packet_id);
        }
    }
    printf("]\n");
}


// await for ACKs on ctrl_socket and Acknowledge packets ids given in received ACK packets
// read from ctrl_socket into ctrl_buffer, peek sizes, parse into tou_packet_ack
// acknowledge packet ids in send_window
void tou_recv_ack(
    tou_conn* conn
) {

    int read = tou_cbuffer_read(conn->ctrl_socket, conn->ctrl_buffer, conn->ctrl_buffer->cap);
    if (read < 0) {
        printf("[tou][tou_recv_ack] can't read from ctrl socket\n");
        return;
    }

    // not enough to parse header
    if (conn->ctrl_buffer->cnt < TOU_LEN_PKT_ACK) {
        printf("[tou][tou_recv_ack] not enough data to parse header yet, have %d < %ld bytes\n", conn->ctrl_buffer->cnt, TOU_LEN_PKT_ACK);
        return ;
    }

    while (conn->ctrl_buffer->cnt >= TOU_LEN_PKT_ACK) {
        uint32_t* ack_count_ptr;
        if ((ack_count_ptr = (uint32_t*) tou_cbuffer_peek(conn->ctrl_buffer, TOU_HOFFSET_PKT_ACK_ACK_COUNT)) == NULL) {
            printf("[tou][tou_recv_ack] CANT PEEK ack_count_ptr\n");
            return ;
        }
        uint32_t ack_count = *ack_count_ptr;
        printf("[tou][tou_recv_ack] ack_count=%d\n", ack_count);

        // not enough to read full packet
        if (TOU_LEN_PKT_ACK + (*ack_count_ptr * sizeof(ack_id_t)) > conn->ctrl_buffer->cnt) {
            return ;
        }
        printf("[tou][tou_recv_ack] enough to read full ACK packet\n");

        uint32_t* ack_list_ptr;
        if ((ack_list_ptr = (uint32_t*) tou_cbuffer_peek(conn->ctrl_buffer, TOU_HOFFSET_PKT_ACK_ACK_LIST)) == NULL) {
            printf("[tou][tou_recv_ack] CANT PEEK ack_list_ptr\n");
            return ;
        }
        printf("[tou][tou_recv_ack] *ack_list_ptr=%d\n", *ack_list_ptr);

        int pkt_size = TOU_LEN_PKT_ACK + ack_count * sizeof(ack_id_t);
        char ack_buff[pkt_size];
        
        int n = tou_cbuffer_pop(conn->ctrl_buffer, ack_buff, pkt_size);
        if (n < pkt_size) {
            printf("[tou][tou_recv_ack] lost %d bytes while trying to pop %d bytes ACK packet\n", pkt_size - n, pkt_size);
            return ;
        }

        ack_list_ptr = (uint32_t*)(ack_buff + TOU_HOFFSET_PKT_ACK_ACK_LIST);
        
        printf("[tou][tou_recv_ack] ack packet dump :\n");
        compact_print_buffer(ack_buff, pkt_size);
        
        printf("[tou][tou_recv_ack] packet list dump :\n");
        compact_print_buffer((char*)ack_list_ptr, ack_count);

        printf("[tou][tou_recv_ack] checking count: %d, buffer: %p\n", ack_count, ack_list_ptr);

        tou_acknowledge_packets(conn, ack_list_ptr, ack_count);
    }
}