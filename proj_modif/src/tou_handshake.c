#include "tou_handshake.h"

// syn is sent by client & received by server
int tou_recv_handshake_syn(
    tou_socket* sock
) {
    char syn_msg[TOU_LEN_SYN + 1] = { 0 };
    int err = 0;
    if ((err=recvfrom(sock->fd, syn_msg, TOU_LEN_SYN, 0, sock->peer_addr, &sock->peer_addr_len)) < 0) {
        printf("ERR = %d\n", err);
        return -1;
    }

    printf("got : SYN = %c%c%c\n", (int)(syn_msg[0]), (int)(syn_msg[1]), (int)(syn_msg[2]));
    struct sockaddr_in* client_addr = (struct sockaddr_in*) sock->peer_addr;
    printf("from IP: %s and port: %i\n",
    inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
    return (syn_msg[0] == 'S' && syn_msg[1] == 'Y' && syn_msg[2] == 'N');
}

// syn : client => server
int tou_send_handshake_syn(
    tou_socket* sock
) {

    /*
     *  packet_type (char)
     *  flags       (char)
     */
    
    char syn_msg[4] = "SYN";

    printf("[tou][tou_send_handshake_syn] %d\n", sock->fd);
    if (sendto(sock->fd, syn_msg, TOU_LEN_SYN, 0, sock->peer_addr, sock->peer_addr_len) < 0) {
        printf("[tou][tou_send_handshake_syn] send SYN failed\n");
        perror("SYN\n");
    }
}

// synack : server => client
tou_socket* tou_send_handshake_synack(
    tou_socket* sock // listen socket
) {
    /*
     *  packet_type (char)
     *  flags       (char)
     *  target_port (uint16_t)
     */

    char flags = TOU_FLAG_SYNACK;

    char synack_msg[TOU_LEN_SYNACK + 1] = "SYN-ACK";

    int client_id = sock->id + 1;
    sock->id++;
    uint16_t new_port = (uint16_t)sock->port + client_id;
    
    synack_msg[7] = 48 + (new_port / 1000) % 10;
    synack_msg[8] = 48 + (new_port / 100) % 10;
    synack_msg[9] = 48 + (new_port / 10) % 10;
    synack_msg[10] = 48 + (new_port / 1) % 10;

    printf("[tou][tou_send_handshake_synack] fd=%d\n", sock->fd);
    printf("[tou][tou_send_handshake_synack] listen_port=%d\n", sock->port);
    printf("[tou][tou_send_handshake_synack] msg=%s\n", synack_msg);
    
    // TODO get ip from listen_sock
    tou_socket* new_sock = tou_make_socket("127.0.0.1", new_port, 1); // listen for client ack
    new_sock->id = client_id;
    if(bind(new_sock->fd, new_sock->my_addr, new_sock->my_addr_len) < 0){
        printf("Couldn't bind to the port of new_sock\n");
        return NULL;
    }

    if (sendto(sock->fd, synack_msg, TOU_LEN_SYNACK + 1, 0, sock->peer_addr, sock->peer_addr_len) < 0) {
        printf("[tou][tou_send_handshake_synack] send SYNACK failed\n");
        return NULL;
    }

    printf("[tou][tou_send_handshake_synack] send success\n");

    return new_sock;
}

// synack : server => client
uint16_t tou_recv_handshake_synack(
    tou_socket* sock
) {
    char synack_msg[TOU_LEN_SYNACK + 1] = { 0 };
    int err = 0;
    if ((err=recvfrom(sock->fd, synack_msg, TOU_LEN_SYNACK, 0, sock->peer_addr, &sock->peer_addr_len)) < 0) {
        printf("ERR = %d\n", err);
        return 0;
    } else {

        printf("[tou][tou_recv_handshake_synack] got %s\n", synack_msg);
        uint16_t new_port = (synack_msg[7] - 48) * 1000 + (synack_msg[8] - 48) * 100 + (synack_msg[9] - 48) * 10 + (synack_msg[10] - 48);
        printf("[tou][tou_recv_handshake_synack] port %d\n", (int)(new_port));
        return new_port;
    }

    return 0;
}

// ack : client => server on newly assigned port
int tou_send_handshake_ack(
    tou_socket* sock
) {
    /*
     *  packet_type (char)
     *  flags       (char)
     *  check_value (char)
     */
    
    char flags = TOU_FLAG_HANDSHAKE_ACK;

    char ack_msg[TOU_LEN_HANDSHAKE_ACK] = "ACK";

    if (sendto(sock->fd, ack_msg, TOU_LEN_HANDSHAKE_ACK, 0, sock->peer_addr, sock->peer_addr_len) < 0) {
        printf("[tou][tou_send_handshake_ack] send ACK failed\n");
    }
}

// ack : client => server
int tou_recv_handshake_ack(
    tou_socket* sock,
    int timeout // TODO use
) {
    char ack_msg[TOU_LEN_HANDSHAKE_ACK + 1] = { 0 };
    int err = 0;

    if ((err=recvfrom(sock->fd, ack_msg, TOU_LEN_HANDSHAKE_ACK, 0, sock->peer_addr, &sock->peer_addr_len)) < 0) {
        printf("ERR = %d\n", err);
        return -1;
    } else {
        printf("[tou][tou_recv_handshake_ack] got %s\n", ack_msg);
        return (ack_msg[0] == 'A' && ack_msg[1] == 'C' && ack_msg[2] == 'K') ? TOU_VALUE_HANDSHAKE_ACK_CHECK : -1;
    }

    return -1;
}