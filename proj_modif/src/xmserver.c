
#include "tou_utils.h"
#include "tou.h"
#include "tou_io.h"
#include "tou_socket.h"
#include "tou_consts.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>

int getport(struct sockaddr* sockaddr) {

    struct sockaddr_in* addr = (struct sockaddr_in*) sockaddr;
    return ntohs(addr->sin_port);
}

typedef struct tou_stats {

    long transmits;
    
    long detected_drops;
    long ack_detect_retransmits;

    long timeout_retransmits;
    long timeouts;
    
    double average_timeout;
    
    long total_loop;
    long total_time;
    long total_sent;

    long last_sent_id;
    long last_seq;

    /*
        ====
        computed
    */

    /*
        ====
        estimates
    */
   double estimated_rtt;
   double estimated_throughput;

} tou_stats;

void dump_stats(tou_stats* stats) {

    printf("stats {\n\
            long transmits = %ld\n\
            long detected_drops = %ld\n\
            long ack_detect_retransmits = %ld\n\
            long timeout_retransmits = %ld\n\
            long timeouts = %ld\n\
            double average_timeout = %lf\n\
            long total_loop = %ld\n\
            long total_sent = %ld\n\
            long total_time = %ld\n\
            long last_sent_id = %ld\n\
            long last_seq = %ld\n\
            ====\n\
            computed\n\
            ====\n\
            estimates\n\
            double estimated_rtt = %lf\n\
            double estimated_throughput = %lf\n\
        }\n", 
            stats->transmits,
            stats->detected_drops,
            stats->ack_detect_retransmits,
            stats->timeout_retransmits,
            stats->timeouts,
            stats->average_timeout,
            stats->total_loop,
            stats->total_sent,
            stats->total_time,
            stats->last_sent_id,
            stats->last_seq,
            stats->estimated_rtt,
            stats->estimated_throughput
        );
}

int main(int argc, char* argv[]) {

    int port = atoi(argv[1]);
    tou_stats stats;
    memset(&stats, 0, sizeof(tou_stats));

    tou_socket* listen_sock = tou_open_socket("0.0.0.0", port);
    tou_conn* conn;

    accept :
    if (port + listen_sock->id >= 9999) {
        listen_sock->id = 0;
    }
    conn = tou_accept_conn(listen_sock);
    printf("new client on port %d\n", conn->socket->port);

    if (fork() != 0) {
        tou_free_conn(conn, 0);
        goto accept;
    }
    else {
        TOU_DEBUG(printf("i must read file '%s'\n", conn->filename));

        FILE* f = fopen(conn->filename, "rb");
        if (f == NULL) {
            printf("file not found\n");
            goto ErrorExit;
        }
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

        // char* string = malloc(size + 1);
        // fread(string, 1, size, f);
        // fclose(f);
        // string[size] = 0;

        // char* buffer = string;


        TOU_DEBUG(
                printf("[tou][tou_send] MSS=%d\n", MSS);
                printf("[tou][tou_send] swindowsize=%d\n", conn->send_window->list->cap);
                printf("[tou][tou_send] sendbuffersize=%d\n", conn->out->cap)
        );

        fd_set readset;
        struct timeval ack_timeout = {
                .tv_sec = 0,
                .tv_usec = 2*1000*TOU_DEFAULT_ACK_TIMEOUT_MS,
        };

        tou_set_nonblocking(conn, TOU_FLAG_NONBLOCKING_DATA | TOU_FLAG_NONBLOCKING_DATA_ENABLE);

        long start = tou_time_ms();

        int last_acked_n = 0;
        int written = 0; // TODO sizes to size_t / long type
        while (size - written > 0 || conn->out->cnt != 0 || !TOU_SLL_ISEMPTY(conn->send_window->list)) {

            // write maximum to out buffer
            int new_write = tou_cbuffer_fread(f, conn->out, size - written);
            // int new_write = tou_cbuffer_insert(conn->out, buffer + written, size - written);
            TOU_DEBUG(
                    // printf("[xserver] wrote %d to out\n", new_write);
                    //tou_cbuffer_dump(conn->out);
            );
            if (new_write > 0) {
                // new_write < 0 when an error occurs
                written += new_write;
                stats.total_sent += new_write;
            }

            // process out buffer into packets => send window
            TOU_DEBUG(
                printf("SENT AT %ld\n", tou_time_ms());
                printf("[xserver] send %d\n", conn->last_packet_id)
            );

            int sent = tou_send(conn);
            if (sent > 0) {
                stats.transmits += sent;
                TOU_DEBUG(
                        printf("[xserver] buffer part sent %d\n", sent);
                        printf("[xserver] send window %d/%d\n", conn->send_window->list->count,
                            conn->send_window->list->cap)
                );
            }

            tou_packet_dtp* pkt = (tou_packet_dtp*) conn->send_window->list->head->val;
            TOU_DEBUG(printf("MUST RECV %d AT %ld\n", pkt->packet_id, pkt->ack_expire));

            TOU_SLL_ITER_USED(conn->send_window->list,

                if (((tou_packet_dtp*)curr->val)->ack_expire < pkt->ack_expire)
                            pkt = (tou_packet_dtp*) curr->val;
                            );

            long timeout = MAX(0, pkt->ack_expire - tou_time_ms());
            // printf("timeout %ld\n", timeout);
            // timeout = 1;
            ack_timeout.tv_sec = timeout / 1000;
            ack_timeout.tv_usec = (timeout % 1000) * 1000;

            stats.average_timeout = (stats.average_timeout * stats.total_loop + timeout) / (stats.total_loop + 1);

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

                stats.timeouts++;
                stats.timeout_retransmits += tou_retransmit(conn, pkt);
                // tou_acknowledge_packets(conn, (int)conn->send_window->expected);

                continue;
            }

            int ack_flag = FD_ISSET(conn->socket->fd, &readset); // new data is only ack in this scenario

            if (ack_flag) {
                TOU_DEBUG(printf("[xserver] checking for ack\n"));

                // await for some acks
                tou_recv_ack(conn, &last_acked_n);
                TOU_DEBUG(printf("[xserver] ack result new count %d\n", last_acked_n));

                if (last_acked_n >= 10) {
                    stats.detected_drops++;

                    last_acked_n = 0;


                    TOU_DEBUG(
                        int dropped = conn->send_window->expected;
                        printf("[xserver] some packet dropped, resend when ?\n");
                        printf("[xserver] packed dropped seq : %d\n", dropped);
                    );

                    
                    // tou_retransmit_all(conn);
                    int retransmit = tou_retransmit_all(conn);
                    stats.ack_detect_retransmits += retransmit;
                }
            }

            stats.total_loop++;
            stats.total_time = tou_time_ms() - start;
            stats.last_seq = conn->send_window->expected - 1;
            stats.last_sent_id = conn->last_packet_id;
            stats.estimated_rtt = conn->rtt;
            stats.estimated_throughput = stats.total_sent * 1.0e-6 / (stats.total_time * 1.0e-3);
        }


        ErrorExit :
            if (sendto(conn->ctrl_socket->fd, "FIN", 4, 0, conn->ctrl_socket->peer_addr, conn->ctrl_socket->peer_addr_len) <
                0) {
                TOU_DEBUG(
                        printf("[tou][tou_send_handshake_syn] send FIN failed\n");
                        perror("FIN\n");
                );

                return -1;
            }
            printf("done\n");


        TOU_DEBUG(
                printf("I'm done with this\n");
                printf("End state :\n");
                printf("file pos : %d / %d\n", written, size);
                printf("send window : ");
                tou_sll_dump(conn->send_window->list);
                printf("recv buffer : ");
                tou_cbuffer_cdump(conn->recv_work_buffer);
        );

        dump_stats(&stats);
    }

    printf("free conn\n");

    TOU_DEBUG(
            printf("I'm done with this\n");
            printf("End state :\n");
            printf("file pos : %d / %d\n", written, size);
            printf("send window : ");
            tou_sll_dump(conn->send_window->list);
            printf("recv buffer : ");
            tou_cbuffer_cdump(conn->recv_work_buffer);
    );

    dump_stats(&stats);

    tou_free_conn(conn, 1);

    return 0;
}