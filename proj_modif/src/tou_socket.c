#include "tou_socket.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

tou_socket* tou_make_socket(
    const char* ip,
    int port,
    int listen
) {
    tou_socket* sock = (tou_socket*) calloc(1, sizeof(tou_socket));

    // Create UDP socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return NULL;
    }
    printf("Socket created successfully\n");
    
    int reuse = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed");
    
    sock->fd = socket_desc;
    struct sockaddr_in* server_addr = (struct sockaddr_in*) calloc(1, sizeof(struct sockaddr));
    if (server_addr == NULL) {
        printf("[tou][make_socket] couldn't allocate memory for sockaddr\n");
        return NULL;
    }
    sock->my_addr = (struct sockaddr*) server_addr;
    sock->my_addr_len = sizeof(*server_addr);

    sock->port = port;
    sock->id = 0;

    struct sockaddr* client_addr = (struct sockaddr*) calloc(1, sizeof(struct sockaddr));
    if (client_addr == NULL) {
        printf("[tou][make_socket] couldn't allocate memory for sockaddr\n");
        return NULL;
    }
    sock->peer_addr = client_addr;
    sock->peer_addr_len = sizeof(*client_addr);

    struct sockaddr_in* to_fill_addr = (struct sockaddr_in*) (listen ? sock->my_addr : sock->peer_addr);

    // Set port and IP:
    to_fill_addr->sin_family = AF_INET;
    to_fill_addr->sin_port = htons(port);
    to_fill_addr->sin_addr.s_addr = inet_addr(ip);

    return sock;
}

void tou_free_socket(
    tou_socket* sock
) {
    free(sock->peer_addr);
    free(sock->my_addr);
    close(sock->fd);
    free(sock);
}
