#include "tou.h"
#include "tou_io.h"
#include "tou_socket.h"
#include <arpa/inet.h>

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
        buffer[i] = i % 255;
    }


    // sendto(conn->socket->fd, buffer, strlen(buffer), 0, conn->socket->peer_addr, conn->socket->peer_addr_len); // 2001

    // printf("client_sock port = %d\n", getport(conn->socket->my_addr));
    // printf("client_sock port = %d\n", getport(conn->socket->peer_addr));
    // printf("ctrl_sock port = %d\n", getport(conn->ctrl_socket->my_addr));
    // printf("ctrl_sock port = %d\n", getport(conn->ctrl_socket->peer_addr));
    // memset(buffer, 0, MSS + 1);
    // recvfrom(conn->socket->fd, buffer, 3 + 6, 0, conn->socket->peer_addr, &conn->socket->peer_addr_len); // 2000
    // printf("got %s\n", buffer);


    tou_send_2(conn, buffer, size, 1);
    tou_recv_ack(conn);
    // tou_recv_ack(conn);


    printf("I'm done with this\n");
    printf("End state :\n");
    printf("send window : ");
    tou_sll_dump(conn->send_window->list);
    printf("recv buffer : ");
    tou_cbuffer_cdump(conn->recv_work_buffer);
    

    tou_free_conn(conn);
    
    return 0;
}