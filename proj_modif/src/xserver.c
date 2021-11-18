#include "tou.h"
#include "tou_io.h"
#include "tou_socket.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
#include "tou_utils.h"

int getport(struct sockaddr* sockaddr) {

    struct sockaddr_in* addr = (struct sockaddr_in*) sockaddr;
    return ntohs(addr->sin_port); 
}

int main() {

    tou_socket* listen_sock = tou_open_socket("0.0.0.0", 2000);

    tou_conn* conn = tou_accept_conn(listen_sock);

    const int MSS = TOU_DEFAULT_MSS - TOU_LEN_DTP;
    char buffer[3*MSS+1]; // + 1 so that we always have at least a \0 to terminate our string
    int size = 3*MSS;
    printf("i must read file '%s'\n", conn->filename);


    for (int i = 0; i < 3*MSS; i++) {
        buffer[i] = 48 + i % (69 + 6);
    }

    printf("[tou][tou_send] MSS=%d\n", MSS);
    fd_set readset;
    struct timeval ack_timeout = {
        .tv_sec = 0,
        .tv_usec = 0,
    };

    int written = 0;
    while (size - written > 0 || !TOU_SLL_ISEMPTY(conn->send_window->list)) {

        // write maximum to out buffer
        int new_write = tou_cbuffer_insert(conn->out, buffer + written, size - written);
        written += new_write;

        // process out buffer into packets => send window
        int sent = tou_send(conn);
        if (sent > 0) 
            printf("[xserver] buffer part sent\n");
        else
            printf("[xserver] couldn't send more\n");

        printf("Waiting for events\n");
        FD_ZERO(&readset);
        FD_SET(conn->ctrl_socket->fd, &readset);
        FD_SET(conn->socket->fd, &readset);
        int ret_select = select(MAX(conn->ctrl_socket->fd, conn->socket->fd) + 1, &readset, NULL, NULL, &ack_timeout);

        if (ret_select <= 0) {

            printf("[xserver] timeout expired\n");
            goto continue_loop;
        }

        int new_ack = FD_ISSET(conn->socket->fd, &readset); // new data is only ack in this scenario
        if (!new_ack) {

            printf("[xserver] unexpected behaviour\n");
            goto continue_loop;
        } else {
        
            // await for some acks
            printf("[xserver] checking for ack\n");
            tou_set_nonblocking(conn, TOU_FLAG_NONBLOCKING_DATA | TOU_FLAG_NONBLOCKING_DATA_ENABLE);
            int new_ack = tou_recv_ack(conn);
            tou_set_nonblocking(conn, TOU_FLAG_NONBLOCKING_DATA);

            if (new_ack < 0) {
                printf("[xserver] some packet dropped, resend when ?\n"); // TODO
            }
        }

        continue_loop : {
            tou_packet_dtp* pkt = (tou_packet_dtp*) conn->send_window->list->head->val;

            long delta = pkt->ack_expire - tou_time_ms();
            if (TOU_SLL_ISEMPTY(conn->send_window->list)) {
                printf("[xserver] no packets waiting for ack\n");
                printf("[xserver] left to write = %d\n", size - written);
                delta = 0;
            }
            if (delta > 0) {
                ack_timeout.tv_sec = delta / 1000;
                ack_timeout.tv_usec = (delta - 1000 * ack_timeout.tv_usec) * 1000;
                printf("[xserver] next ack timeout is in %lds %ldµs\n", ack_timeout.tv_sec, ack_timeout.tv_usec);
            }
        }
    }

    // TODO send FIN
    if (sendto(conn->ctrl_socket->fd, "FIN", 4, 0, conn->ctrl_socket->peer_addr, conn->ctrl_socket->peer_addr_len) < 0) {
        printf("[tou][tou_send_handshake_syn] send FIN failed\n");
        perror("FIN\n");
        return -1;
    }

    printf("I'm done with this\n");
    printf("End state :\n");
    printf("send window : ");
    tou_sll_dump(conn->send_window->list);
    printf("recv buffer : ");
    tou_cbuffer_cdump(conn->recv_work_buffer);    
    printf("[xserver] TOTAL SENT = %d\n", written);
    

    tou_free_conn(conn);
    
    return 0;
}