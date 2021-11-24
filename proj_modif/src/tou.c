#include "tou.h"
#include "tou_socket.h"
#include "tou_handshake.h"
#include "tou_io.h"
#include "tou_utils.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>

// TODO add alloc checks

tou_socket* tou_open_socket(
        const char* listen_ip,
        int listen_port
) {

    tou_socket* sock = tou_make_socket(listen_ip, listen_port, 1);
    if (sock == NULL) {
        TOU_DEBUG(printf("[tou][open_socket] couldn't make_tou_socket\n"));
        return NULL;
    }

    // Bind to the set port and IP: (use my_addr bc done on server side and tou_make_socket with listen=1)
    if (bind(sock->fd, sock->my_addr, sock->my_addr_len) < 0) {
        TOU_DEBUG(printf("[tou][open_socket] couldn't bind to the port\n"));
        return NULL;
    }

    TOU_DEBUG(printf("Done with binding\n"));
    return sock;
}

tou_conn* tou_accept_conn(
        tou_socket* listen_sock
) {
    TOU_DEBUG(printf("Accepting\n"));

    if (tou_recv_handshake_syn(listen_sock) < 0) {
        TOU_DEBUG(printf("[tou][recv_handshake_syn] failed\n"));
        return NULL;
    }
    TOU_DEBUG(printf("[tou][open] got syn\n"));

    long rtt = tou_time_ms();
    tou_socket* client_sock = tou_send_handshake_synack(listen_sock);
    if (client_sock == NULL || client_sock->fd <= listen_sock->fd) {
        TOU_DEBUG(printf("[tou][recv_handshake_synack] failed\n"));
        return NULL;
    }
    TOU_DEBUG(printf("[tou][open] sent synack\n"));

    if (tou_recv_handshake_ack(listen_sock, TOU_HANDSHAKE_ACK_DEFAULT_TIMEOUT) != TOU_VALUE_HANDSHAKE_ACK_CHECK) {
        TOU_DEBUG(printf("[tou][recv_handshake_ack] failed\n"));
        return NULL;
    }
    rtt = tou_time_ms() - rtt;
    TOU_DEBUG(printf("[tou][open] got ack\n"));

    tou_conn* client_conn = tou_make_conn(listen_sock, client_sock); // 2000 | 2000 + 1
    client_conn->rtt = rtt;
    TOU_DEBUG(printf("[tou][open] connection established, rtt=%ld\n", rtt));

    int err;
    if ((err = recvfrom(client_sock->fd, client_conn->filename, 128, 0, client_sock->peer_addr,
                        &client_sock->peer_addr_len)) < 0) {
        TOU_DEBUG(printf("ERR = %d\n", err));
        return NULL;
    } else {
        TOU_DEBUG(printf("[tou][open] got filename=%s\n", client_conn->filename));
    }

    return client_conn;
}

tou_conn* tou_connect(
        const char* target_ip,
        int target_port
) {

    tou_socket* conn_sock = tou_make_socket(target_ip, target_port, 0); // listen=0 so ip in filled in sock->peer_addr

    long start = tou_time_ms();

    if (tou_send_handshake_syn(conn_sock) < 0) {
        TOU_DEBUG(printf("[tou][tou_send_handshake_syn] failed\n"));
        return NULL;
    }
    TOU_DEBUG(
            printf("[tou][connect] sent SYN\n");
            printf("[tou][connect] await SYNACK\n");
    );

    uint16_t new_port = tou_recv_handshake_synack(conn_sock);
    if (new_port == 0) {
        TOU_DEBUG(printf("[tou][tou_send_handshake_synack] failed\n"));
        return NULL;
    }

    long rtt = start - tou_time_ms();

    // tou_free_socket(conn_sock); now conn_sock is needed

    tou_socket* mysock = tou_make_socket(target_ip, new_port, 0); // same, listen=0, sock->peer_addr

    TOU_DEBUG(printf("[tou][connect] sending ACK on new port\n"));
    if (tou_send_handshake_ack(conn_sock) < 0) {
        TOU_DEBUG(printf("[tou][tou_send_handshake_ack] failed\n"));
        return NULL;
    }

    tou_conn* conn = tou_make_conn(conn_sock, mysock);
    conn->rtt = rtt;
    TOU_DEBUG(printf("6ix9ine on the block on the regular, rtt=%ld\n", rtt));

    return conn;
}

void tou_set_nonblocking(
        tou_conn* conn,
        char flags
) {

    if (flags & TOU_FLAG_NONBLOCKING_DATA) {
        int curr_flags = fcntl(conn->socket->fd, F_GETFL, 0);
        fcntl(conn->socket->fd, F_SETFL, curr_flags | (O_NONBLOCK * (flags & TOU_FLAG_NONBLOCKING_DATA_ENABLE)));
    }

    if (flags & TOU_FLAG_NONBLOCKING_CTRL) {
        int curr_flags = fcntl(conn->ctrl_socket->fd, F_GETFL, 0);
        fcntl(conn->ctrl_socket->fd, F_SETFL, curr_flags | (O_NONBLOCK * (flags & TOU_FLAG_NONBLOCKING_CTRL_ENABLE)));
    }
}

int tou_read(
        tou_conn* conn,
        char* buffer,
        int n,
        int mode
) {
    while (conn->in->cnt < n) {
        TOU_DEBUG(printf("[tou][tou_read] need at least %d more bytes, awaiting for new packets\n", n - conn->in->cnt));
        tou_recv_packet(conn);
    }
    TOU_DEBUG(printf("[tou][tou_read] i got enough : %d in stream, needed %d\n", conn->in->cnt, n));

    int popped = tou_cbuffer_pop(conn->in, buffer, mode == TOU_READ_EXACT ? n : MIN(mode, conn->in->cnt));
    TOU_DEBUG(printf("[tou][tou_read] popped %d from stream to buffer, still got %d left\n", popped, conn->in->cnt));
    tou_cbuffer_cdump(conn->in);

    return popped;
}

int tou_retransmit(
        tou_conn* conn,
        tou_packet_dtp* pkt
) {

    char header[6];
    tou_packet_set_header(pkt, header, pkt->packet_id, pkt->buffer, pkt->data_packet_size);
    tou_write_packet(conn->socket, header, 6, pkt->buffer, pkt->data_packet_size);
    tou_packet_set_expiration(pkt, tou_time_ms() + TOU_DEFAULT_ACK_TIMEOUT_MS);

    return 0;
}

int tou_retransmit_all(
        tou_conn* conn
) {
    tou_packet_dtp* pkt = NULL;
    TOU_SLL_ITER_USED(conn->send_window->list,
                      pkt = (tou_packet_dtp*) curr->val;
                              tou_retransmit(conn, pkt);
    );

    return 0;
}

int tou_retransmit_n(
        tou_conn* conn,
        int n
) {

    tou_packet_dtp* pkt = NULL;
    int i = 0;
    TOU_SLL_ITER_USED(conn->send_window->list,
                      if (i >= n) return n;
                              pkt = (tou_packet_dtp*) curr->val;
                              tou_retransmit(conn, pkt);
                              i++;
    );
}

int tou_retransmit_expired(
        tou_conn* conn,
        long expire_time
) {
    tou_packet_dtp* pkt = NULL;
    TOU_SLL_ITER_USED(conn->send_window->list,
                      pkt = (tou_packet_dtp*) curr->val;
                              if (pkt->ack_expire < expire_time)
                                  tou_retransmit(conn, pkt);
                              else return 0;
    );

    return 0;
}