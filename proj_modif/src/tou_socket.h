#ifndef TOU_SOCKET_H
#define TOU_SOCKET_H

#include <sys/socket.h>

typedef struct {
    int fd;
    int port;
    int id;
    struct sockaddr* my_addr;
    socklen_t my_addr_len;
    struct sockaddr* peer_addr;
    socklen_t peer_addr_len;
} tou_socket;

tou_socket* tou_make_socket(
    const char* ip,
    int port,
    int listen_1_connect_0
);

void tou_free_socket(
    tou_socket* socket
);

#endif
