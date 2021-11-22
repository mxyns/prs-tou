#include "tou.h"
#include "tou_socket.h"
#include "tou_utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/signal.h>

int main(void) {

    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    tou_socket* listen_sock = tou_open_socket("127.0.0.1", 2000);

    int count = 0;
    int pid = -1;
    while (count < 3 && pid != 0) {

        tou_conn* conn = tou_accept_conn(listen_sock);
        count++;

        if ((pid = fork()) == 0) {
            prctl(PR_SET_PDEATHSIG, SIGHUP);
            count = 0;
            tou_socket* sock = conn->socket;
            while (count < 4) {

                TOU_DEBUG(printf("Listening for incoming messages...\n\n"));

                // Receive client's message:
                if (recvfrom(sock->fd, client_message, sizeof(client_message), 0,
                             sock->peer_addr, &sock->peer_addr_len) < 0) {
                    printf("Couldn't receive\n");
                    return -1;
                }

                TOU_DEBUG(printf("Msg from client [%d]: %s\n", sock->id, client_message));

                // Respond to client:
                strcpy(server_message, client_message);

                if (sendto(sock->fd, server_message, strlen(server_message), 0,
                           sock->peer_addr, sock->peer_addr_len) < 0) {
                    TOU_DEBUG(printf("Can't send\n"));
                    return -1;
                }
                count++;
            }
            tou_free_conn(conn);
        }
    }
    TOU_DEBUG(printf("I'm done with this\n"));
    // Close the socket:
    tou_free_socket(listen_sock);

    return 0;
}
