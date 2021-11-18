#include "tou.h"

int main() {

    tou_conn* conn = tou_connect("127.0.0.1", 2000);

    char buffer[1500];

    strcpy(buffer, "filename");
    sendto(conn->socket->fd, buffer, strlen(buffer), 0, conn->socket->peer_addr, conn->socket->peer_addr_len); // 2001
    
    recvfrom(conn->socket->fd, buffer, 3 + 6, 0, conn->socket->peer_addr, &conn->socket->peer_addr_len);
    printf("got %s\n", buffer);

    strcpy(buffer, "ACK000001");
    sendto(conn->ctrl_socket->fd, buffer, strlen(buffer), 0, conn->ctrl_socket->peer_addr, conn->ctrl_socket->peer_addr_len); // 2001
}