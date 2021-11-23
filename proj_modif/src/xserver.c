
#include "tou_utils.h"
#include "tou.h"
#include "tou_io.h"
#include "tou_socket.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>

int getport(struct sockaddr* sockaddr) {

    struct sockaddr_in* addr = (struct sockaddr_in*) sockaddr;
    return ntohs(addr->sin_port);
}

int main() {

    tou_socket* listen_sock = tou_open_socket("0.0.0.0", 2000);

    tou_conn* conn = tou_accept_conn(listen_sock);

    const int MSS = TOU_DEFAULT_MSS - TOU_LEN_DTP;
    char data[10 * MSS + 1]; // + 1 so that we always have at least a \0 to terminate our string
    char* buffer = data;
    int size = 10 * MSS;
    TOU_DEBUG(printf("i must read file '%s'\n", conn->filename));

    FILE* f = fopen(conn->filename, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char* string = malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);
    string[fsize] = 0;

    buffer = string;
    size = fsize;

    // for (int i = 0; i < 10*MSS; i++) {
    //     buffer[i] = 48 + i % (69 + 6);
    // }

    TOU_DEBUG(
            printf("[tou][tou_send] MSS=%d\n", MSS);
            printf("[tou][tou_send] swindowsize=%d\n", conn->send_window->list->cap);
            printf("[tou][tou_send] sendbuffersize=%d\n", conn->out->cap)
    );

    fd_set readset;
    struct timeval ack_timeout = {
            .tv_sec = 0,
            .tv_usec = 2*TOU_DEFAULT_ACK_TIMEOUT_MS,
    };

    tou_set_nonblocking(conn, TOU_FLAG_NONBLOCKING_DATA | TOU_FLAG_NONBLOCKING_DATA_ENABLE);

    long start = tou_time_ms();

    int written = 0;
    while (size - written > 0 || conn->out->cnt != 0 || !TOU_SLL_ISEMPTY(conn->send_window->list)) {

        // write maximum to out buffer
        int new_write = tou_cbuffer_insert(conn->out, buffer + written, size - written);
        TOU_DEBUG(
                printf("[xserver] wrote %d to out\n", new_write);
                tou_cbuffer_dump(conn->out);
        );
        if (new_write > 0) {
            // new_write < 0 when an error occurs
            written += new_write;
        }

        // process out buffer into packets => send window
        TOU_DEBUG(printf("SENT AT %ld\n", tou_time_ms()));
        int sent = tou_send(conn);
        if (sent > 0) {
            TOU_DEBUG(
                    printf("[xserver] buffer part sent\n");
                    printf("[xserver] send window %d/%d\n", conn->send_window->list->count,
                           conn->send_window->list->cap)
            );
        }

        tou_packet_dtp* pkt = (tou_packet_dtp*) conn->send_window->list->head->val;
        TOU_DEBUG(printf("MUST RECV %d AT %ld\n", pkt->packet_id, pkt->ack_expire));

        long timeout = MAX(0, pkt->ack_expire - tou_time_ms());
        ack_timeout.tv_sec = timeout / 1000;
        ack_timeout.tv_usec = (timeout % 1000) * 1000;

        TOU_DEBUG(printf("TIMEOUT IN %ld : %lds %ldusec\n", timeout, (long) ack_timeout.tv_sec,
                         (long) ack_timeout.tv_usec));

        // printf("Waiting for events\n");
        FD_ZERO(&readset);
        FD_SET(conn->ctrl_socket->fd, &readset);
        FD_SET(conn->socket->fd, &readset);
        int range = MAX(conn->ctrl_socket->fd, conn->socket->fd) + 1;
        int ret_select = select(range, &readset, NULL, NULL, &ack_timeout);

        TOU_DEBUG(
                printf("ret_select = %d\n", ret_select);
                for (int i = 0; i < range; i++) {
                    if (FD_ISSET(i, &readset)) {
                        printf("FLAG %d\n", i);
                    }
                }
        );

        if (ret_select == 0) {

            TOU_DEBUG(
                    printf("ACK EXPIRED %d\n", pkt->packet_id);
                    compact_print_buffer(pkt->buffer, pkt->data_packet_size);
            );
            tou_retransmit(conn, pkt);
            // tou_acknowledge_packets(conn, (int)conn->send_window->expected);

            continue;
        }

        int ack_flag = FD_ISSET(conn->socket->fd, &readset); // new data is only ack in this scenario

        if (ack_flag) {
            TOU_DEBUG(printf("[xserver] checking for ack\n"));

            // await for some acks
            int new_ack = tou_recv_ack(conn);
            if (new_ack > 0) {
                TOU_DEBUG(printf("[xserver] new acked %d, at %d\n", new_ack, conn->send_window->expected - 1));
            }
            if (new_ack < 0) {
                TOU_DEBUG(
                        printf("[xserver] some packet dropped, resend when ?\n");
                        printf("[xserver] packed dropped seq : %d\n", -new_ack)
                );
                tou_retransmit(conn, pkt);
            }
        }
    }

    if (sendto(conn->ctrl_socket->fd, "FIN", 4, 0, conn->ctrl_socket->peer_addr, conn->ctrl_socket->peer_addr_len) <
        0) {
        TOU_DEBUG(
                printf("[tou][tou_send_handshake_syn] send FIN failed\n");
                perror("FIN\n");
        );

        return -1;
    }

    TOU_DEBUG(
            printf("I'm done with this\n");
            printf("End state :\n");
            printf("file pos : %d / %d\n", written, size);
            printf("send window : ");
            tou_sll_dump(conn->send_window->list);
            printf("recv buffer : ");
            tou_cbuffer_cdump(conn->recv_work_buffer);
    );

    printf("[xserver] TOTAL SENT = %d\n", written);
    printf("[xserver] SEQ = %d\n", conn->last_packet_id);
    printf("[xserver] TIME = %ld\n", tou_time_ms() - start);
    printf("[xserver] TIME = %ld\n", tou_time_ms() - tou_time_ms());

    tou_free_conn(conn);

    return 0;
}