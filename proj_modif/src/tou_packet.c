#include <stdlib.h>
#include "tou.h"
#include "tou_packet.h"
#include "tou_conn.h"
#include "tou_utils.h"
#include "tou_io.h"

tou_packet_dtp* tou_make_packet_dtp(
    uint32_t packet_id,
    int buffer_size,
    char* buffer
) {

    tou_packet_dtp* packet = (tou_packet_dtp*) calloc(1, sizeof(tou_packet_dtp));
    packet->buffer = buffer;
    packet->packet_id = packet_id;
    packet->buffer_size = buffer_size;
    packet->data_packet_size = 0;
    packet->acked = 0;
    packet->data_start = TOU_HOFFSET_DTP_PACKET_DATA;

    return packet;
}

void tou_packet_dtp_reset(
    tou_packet_dtp* packet
) {
    packet->packet_id = 0;
    packet->data_packet_size = 0;
    packet->acked = 0;
}

// tries to parse data in recv_work_buffer into packets and pushes them into recv_window
// returns 1 on success, 0 on failure 
int tou_parse_dtp(
    tou_conn* conn
) {

    // not enough to parse header
    if (conn->recv_work_buffer->cnt < TOU_LEN_DTP) {
        printf("[tou][tou_parse_dtp] not enough data to parse header yet, have %d < %ld bytes\n", conn->recv_work_buffer->cnt, (long)(TOU_LEN_DTP));
        return 0;
    }

    uint32_t* pkt_size_ptr;
    if ((pkt_size_ptr = (uint32_t*) tou_cbuffer_peek(conn->recv_work_buffer, TOU_HOFFSET_DTP_PACKET_SIZE)) == NULL) {
        printf("[tou][tou_parse_dtp] CANT PEEK\n");
        return 0;
    }
    uint32_t pkt_size = *pkt_size_ptr;
    printf("[tou][tou_parse_dtp] pkt_size %d\n", pkt_size);

    // not enough to read full packet
    if (TOU_LEN_DTP + pkt_size > conn->recv_work_buffer->cnt) {
        return 0;
    }
    printf("[tou][tou_parse_dtp] enough to read full packet\n");

    uint32_t* pkt_id_ptr;
    if ((pkt_id_ptr = (uint32_t*) tou_cbuffer_peek(conn->recv_work_buffer, TOU_HOFFSET_DTP_PACKET_ID)) == NULL) {
        printf("[tou][tou_parse_dtp] CANT PEEK\n");
        return 0;
    }
    printf("[tou][tou_parse_dtp] *pkt_id_ptr=%d\n", *pkt_id_ptr);

    // TODO direct insert (when expected)

    tou_sll_dump(conn->recv_window->list);
    printf("ISFULL=%d\n", TOU_SLL_ISFULL(conn->recv_window->list));
    tou_packet_dtp* packet = tou_sll_insert(conn->recv_window->list, *pkt_id_ptr); // add packet to recv_window and get packet struct
    if (packet == NULL) {
        printf("[tou][tou_parse_dtp] recv_window is full. data not lost and still in recv_work_buffer\n");
        return 0;
    }

    tou_packet_dtp_reset(packet);
    packet->packet_id = *pkt_id_ptr;
    
    int total_pop = tou_cbuffer_pop(conn->recv_work_buffer, packet->buffer, pkt_size + TOU_LEN_DTP); // pop data from recv buffer to packet buffer

    packet->data_packet_size = total_pop - TOU_LEN_DTP; // we must get at least TOU_LEN_DTP bytes but they dont count as data bc they're part of the header

    if (total_pop != pkt_size + TOU_LEN_DTP) {
        printf("[tou][tou_parse_dtp] GOT %d INSTEAD OF %ld MISSING %ld BYTES\n", packet->data_packet_size, (long)(pkt_size+TOU_LEN_DTP), (long)(pkt_size+TOU_LEN_DTP-packet->data_packet_size));
        exit(1);
        return 0;
    }

    return 1;
}


// appends packet payload to stream and increments window->expected if successful
// may produce fatal error if payload wasn't written completely
int tou_packet_dtp_tostream(
    tou_window* window,
    tou_packet_dtp* packet,
    tou_cbuffer* stream
) {
    if (window->expected != packet->packet_id) {
        printf("[tou][tou_packet_dtp_tostream] packet %d is expected, this should've never been called\n", packet->packet_id);
        return 0;
    }

    if (packet->data_packet_size > stream->cap-stream->cnt) {
        printf("[tou][tou_packet_dtp_tostream] not enough space to write packet %d in stream\n", packet->packet_id);
        return 0;
    }
    
    int n = tou_cbuffer_insert(stream, packet->buffer + packet->data_start, packet->data_packet_size);
    if (n < packet->data_packet_size) {
        printf(
            "[tou][tou_packet_dtp_tostream] fatal error occurred while writing packet %d to stream, %d less bytes were written.\
            \nstream is now corrupted, aborting.\n",
            packet->packet_id,
            packet->data_packet_size - n
        );
        exit(1); // TODO gracefully terminate connection
        return 0;
    }
    printf("[tou][tou_packet_dtp_tostream] written full %d payload bytes. stream size is now %d\n", n, stream->cnt);

    window->expected++;
    return 1;
}

void tou_packet_dtp_dump(
    tou_packet_dtp packet,
    int dump_buffer
) {

    printf("tou_packet_dtp {\n\tid=%d\n\theader=%d bytes\n\tdata_size=%d bytes\n\tstarting at=%d\n\tin %d bytes buffer\n\tfull=%d\n\t\n}\n",
        packet.packet_id,
        packet.data_start,
        packet.data_packet_size,
        packet.data_start,
        packet.buffer_size,
        packet.data_packet_size + packet.data_start == packet.buffer_size
    );

    printf("packet.buffer : [\n\t*");
    int zeros = 0;
    for (int i = 0; i < packet.buffer_size; i++) {

        int last_byte = i == packet.data_start + packet.data_packet_size - 1 || \
                        i == packet.data_start - 1; // if we're at last byte of payload or header 

        if (packet.buffer[i] == 0) {
            zeros++;

            if (i != packet.buffer_size - 1 && !last_byte) continue;
        }

        printNZeros(zeros, 5);
        zeros=0;

        if (packet.buffer[i] != 0)
            printf("%d ", packet.buffer[i]);
        
        if (last_byte) printf("| ");
    }
    printf("\n]\n");
}