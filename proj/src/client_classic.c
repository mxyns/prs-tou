#include "tou.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(void) {

    char server_message[2000], client_message[2000];
    
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    tou_conn* conn = tou_connect("127.0.0.1", 2000);
    tou_socket* mysock = conn->socket;
    
    while (1) {
        // Get input from the user:
        printf("Enter message: ");
        *fgets(client_message, 2000, stdin);
        if (client_message[0] == '\n') break;
        
        // Send the message to server:
        if(sendto(mysock->fd, client_message, strlen(client_message), 0,
            mysock->peer_addr, mysock->peer_addr_len) < 0) {
            printf("Unable to send message\n");
            return -1;
        }
        
        // Receive the server's response:
        if(recvfrom(mysock->fd, server_message, sizeof(server_message), 0,
            mysock->peer_addr, &mysock->peer_addr_len) < 0) {
            printf("Error while receiving server's msg\n");
            return -1;
        }
        
        printf("Server's response: %s\n", server_message);    
    }

    // Close the socket:
    tou_free_conn(conn);
    
    return 0;
}

