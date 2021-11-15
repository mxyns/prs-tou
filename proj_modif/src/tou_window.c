#include "tou.h"
#include "tou_datastructs.h"
#include "tou_window.h"
#include "tou_packet.h"
#include "tou_rel.h"
#include <stdlib.h>

tou_window* tou_make_window(
    int window_size,
    int max_packet_size,
    uint32_t expected
) {
    // allocate window, N packet data buffers and N packet structs
    tou_window* window = (tou_window*) calloc(1, sizeof(tou_window) + window_size * (max_packet_size * sizeof(char) + sizeof(tou_packet_dtp)));

    char* packet_buffers = (char*)(window + 1); // start of packets buffers
    tou_packet_dtp* packets = (tou_packet_dtp*) (packet_buffers + window_size * max_packet_size * sizeof(char)); // start of packets structs
    tou_sll* list = tou_sll_new(window_size); // make list of size N

    tou_sll_node* current = list->head;
    for (int i = 0; i < window_size; i++) {
        packets[i].buffer = packet_buffers + i * max_packet_size; // set packet buffer location
        packets[i].packet_id = 0; // default id
        packets[i].data_packet_size = 0; // packet is empty
        packets[i].buffer_size = max_packet_size; // buffer size
        packets[i].data_start = TOU_HOFFSET_DTP_PACKET_DATA; // packet data offset in buffer
        current->val = packets + i; // store packet in list
        current = current->next;
    }

    window->list = list;
    window->expected = expected;
    tou_sll_dump(list);

    return window;
}

void tou_free_window(
    tou_window* window
) {
    tou_free_sll(window->list, TOU_SLL_FREE_NONE);
    free(window); // frees tou_window struct, all preallocated tou_packet_dtp structs and their underlying buffers
}


void tou_window_accept_ack_packet_subroutine(
    tou_conn* conn,
    tou_packet_ack* ack,
    tou_packet_dtp* packet
) {

    if (ack->ack_count == TOU_PKT_ACK_MAX_ACK_COUNT) {
        printf("[tou][tou_window_accept_ack_packet_subroutine] ack full sending and reset\n");
        tou_send_ack(conn, ack);
        ack->ack_count = 0; // no need to reset buffer, we'll just read in range [buffer, buffer+ack_count]
    }
    if (!packet->acked) {
        printf("[tou][tou_window_accept_ack_packet_subroutine] mark id=%d at pos=%d ptr=%p\n", packet->packet_id, ack->ack_count, packet);
        ack->ack_list[ack->ack_count++] = packet->packet_id; // register packet in ack message
        packet->acked = 1;
    } else {
        printf("[tou][tou_window_accept_ack_packet_subroutine] already acked id=%d ptr=%p\n", packet->packet_id, packet);
    }
}

// accepts all expected packed in window and appends their payload to stream
// assumes packets in window->list are of type tou_packet_dtp
// returns the number of packets processed
int tou_window_accept(
    tou_conn* conn,
    tou_window* window,
    tou_cbuffer* stream
) {

    char poperr = 0;
    int popped = 0;
    printf("[tou][tou_window_accept] while : !ISEMPTY = %d, HEAD!=NULL = %d, VAL!=NULL = %d\n", 
        !TOU_SLL_ISEMPTY(window->list),
        window->list->head != NULL,
        window->list->head->val != NULL
    );

    ack_id_t ack_list[TOU_PKT_ACK_MAX_ACK_COUNT] = {0};
    tou_packet_ack ack = tou_make_packet_ack(ack_list); // ack packet that will be used to acknowledge accepted packets

    while (!TOU_SLL_ISEMPTY(window->list) && window->list->head != NULL && window->list->head->val != NULL) {

        tou_packet_dtp* next_packet = (tou_packet_dtp*) window->list->head->val;
        printf("[tou][tou_window_accept] processing packet %d\n", next_packet->packet_id);

        if (next_packet->packet_id < window->expected) {
            printf("[tou][tou_window_accept] got unexpected packet, it may be a duplicate\n");
            next_packet = tou_sll_pop(window->list, &poperr);
            if (poperr || next_packet == NULL) { printf("next_packet poperr packet_id<expected\n"); break; } // early stop
            tou_packet_dtp_dump(*next_packet, 0);
        
        } else if (next_packet->packet_id > window->expected) {
            
            printf("[tou][tou_window_accept] intermediate packet missing for %d packet\n", next_packet->packet_id);
            tou_window_accept_ack_packet_subroutine(conn, &ack, next_packet);
            break;

        } else {

            printf("[tou][tou_window_accept] packet was expected\n");
            next_packet = window->list->head->val; // peek packet
            if (next_packet->data_packet_size > TOU_CBUFFER_AVAILABLE(stream)) {
                printf("[tou][tou_window_accept] not enough space in stream to write packet id=%d, missing=%d\n", next_packet->packet_id, TOU_CBUFFER_AVAILABLE(stream) - next_packet->data_packet_size);
            }

            // pop packet from window
            next_packet = tou_sll_pop(window->list, &poperr);
            if (poperr || next_packet == NULL) { printf("next_packet poperr expected\n"); break; }
            printf("[tou][tou_window_accept] packet was popped\n");

            // serialize packet to stream
            int packet_inserted = tou_packet_dtp_tostream(window, next_packet, stream);
            if (!packet_inserted) { // if not inserted into stream (lack of space in stream), packet is added back to window
                printf("[tou][tou_window_accept] couldn't insert packet into stream, adding it back to the wait list until enough space is available\n");
                tou_sll_insert(window->list, next_packet->packet_id); // no need to modify node value bc it was left intact
                break;
            }
            printf("[tou][tou_window_accept] packet was written\n");

            popped++;

            tou_window_accept_ack_packet_subroutine(conn, &ack, next_packet);
        }
    }

    if (poperr != 0) {
        printf("[tou][tou_window_accept] error while popping next packet, empty window?\n");
    }

    // send ack before leaving
    tou_send_ack(conn, &ack);

    return popped;
}