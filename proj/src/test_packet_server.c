#include "tou.h"
#include "tou_io.h"
#include "tou_socket.h"
#include "tou_packet.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>   
#include <sys/prctl.h>
#include <sys/signal.h>

int main(void){

    // TODO debug flag for logging to disable useless logs
    tou_socket* listen_sock = tou_open_socket("127.0.0.1", 2000);

    tou_conn* conn = tou_accept_conn(listen_sock);

    const int MSS = TOU_DEFAULT_MSS - TOU_LEN_DTP;
    char buffer[MSS+1]; // + 1 so that we always have at least a \0 to terminate our string
    while(1) {

        printf("loop\n");
        memset(buffer, 0, MSS);
        int n;
        if ((n = tou_read(conn, buffer, 2, MSS)) > 0) {
            printf("received : %s\n", buffer);
        }
    }


    printf("I'm done with this\n");
    tou_free_conn(conn);
    tou_free_socket(listen_sock);
    
    return 0;
}
