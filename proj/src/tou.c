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

// TODO add alloc checks

tou_socket* tou_open_socket(
    const char* listen_ip,
    int listen_port
) {

    tou_socket* sock = tou_make_socket(listen_ip, listen_port, 1);
    if (sock == NULL) {
        printf("[tou][open_socket] couldn't make_tou_socket\n");
        return NULL;
    }

    // Bind to the set port and IP: (use my_addr bc done on server side and tou_make_socket with listen=1)
    if(bind(sock->fd, sock->my_addr, sock->my_addr_len) < 0){
        printf("[tou][open_socket] couldn't bind to the port\n");
        return NULL;
    }

    printf("Done with binding\n");
    return sock;
}

tou_conn* tou_accept_conn(
   tou_socket* listen_sock
) {
    printf("Accepting\n");

    if (tou_recv_handshake_syn(listen_sock) < 0) {
        printf("[tou][recv_handshake_syn] failed\n");
        return NULL;
    }
    printf("[tou][open] got syn\n");

    long rtt = tou_time_ms();
    tou_socket* client_sock = tou_send_handshake_synack(listen_sock);
    if (client_sock == NULL || client_sock->fd <= listen_sock->fd) {
        printf("[tou][recv_handshake_synack] failed\n");
        return NULL;
    }
    printf("[tou][open] sent synack\n");

    if (tou_recv_handshake_ack(client_sock, TOU_HANDSHAKE_ACK_DEFAULT_TIMEOUT) != TOU_VALUE_HANDSHAKE_ACK_CHECK) {
        printf("[tou][recv_handshake_ack] failed\n");
        return NULL;
    }
    rtt = tou_time_ms() - rtt;
    printf("[tou][open] got ack\n");
    
    tou_conn* client_conn = tou_make_conn(listen_sock, client_sock);
    client_conn->rtt = rtt;
    printf("[tou][open] connection established, rtt=%ld\n", rtt);

    return client_conn;
}

tou_conn* tou_connect(
    const char* target_ip,
    int target_port
) {
    
    tou_socket* conn_sock = tou_make_socket(target_ip, target_port, 0); // listen=0 so ip in filled in sock->peer_addr

    long start = tou_time_ms();

    if (tou_send_handshake_syn(conn_sock) < 0) {
        printf("[tou][tou_send_handshake_syn] failed\n");
        return NULL;
    }
    printf("[tou][connect] sent SYN\n");

    printf("[tou][connect] await SYNACK\n");

    uint16_t new_port = tou_recv_handshake_synack(conn_sock);
    if (new_port == 0) {
        printf("[tou][tou_send_handshake_synack] failed\n");
        return NULL;
    }

    long rtt = start - tou_time_ms();

    // tou_free_socket(conn_sock); now conn_sock is needed

    tou_socket* mysock = tou_make_socket(target_ip, new_port, 0); // same, listen=0, sock->peer_addr

    printf("[tou][connect] sending ACK on new port\n");
    if (tou_send_handshake_ack(mysock) < 0) {
        printf("[tou][tou_send_handshake_ack] failed\n");
        return NULL;
    }

    tou_conn* conn = tou_make_conn(conn_sock, mysock);
    conn->rtt = rtt;
    printf("6ix9ine on the block on the regular, rtt=%ld\n", rtt);

    return conn;
}

int tou_read(
    tou_conn* conn,
    char* buffer,
    int n,
    int mode
) {
    while(conn->in->cnt < n) {
        printf("[tou][tou_read] need at least %d more bytes, awaiting for new packets\n", n - conn->in->cnt);
        int recv_packet = tou_recv_packet(conn);
    }
    printf("[tou][tou_read] i got enough : %d in stream, needed %d\n", conn->in->cnt, n);

    int popped = tou_cbuffer_pop(conn->in, buffer, mode == TOU_READ_EXACT ? n : MIN(mode, conn->in->cnt));
    printf("[tou][tou_read] popped %d from stream to buffer, still got %d left\n", popped, conn->in->cnt);
    tou_cbuffer_cdump(conn->in);
    
    return popped;
}