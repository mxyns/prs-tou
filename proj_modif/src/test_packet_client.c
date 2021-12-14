#include "tou.h"
#include "tou_packet.h"
#include "tou_consts.h"
#include "tou_io.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(void) {

#define MSS (TOU_DEFAULT_MSS - TOU_LEN_DTP)

    char input[2 * MSS] = {0};
    tou_conn* conn = tou_connect("127.0.0.1", 2000);
    tou_socket* mysock = conn->socket;

    tou_send_force_id(conn, "va", 2, 3);
    tou_send_force_id(conn, "comment", 7, 1);
    tou_send_force_id(conn, "ca", 2, 2);

    while (1) {
        // Get input from the user:
        printf("Enter message: ");

        tou_sll_dump(conn->send_window->list);

        *fgets(input, 2 * MSS, stdin);
        if (input[0] == '\n') break;
        int trimlen = strlen(input) - (input[strlen(input) - 1] == '\n');

        printf("SENDING %d bytes for : %s\n", trimlen, input);
        int base_id = conn->last_packet_id + 1;
        tou_send_2(conn, input, trimlen, 1);

        memset(input, 0, MSS);
    }

    // Close the socket:
    tou_free_conn(conn, 1);

    return 0;
}

