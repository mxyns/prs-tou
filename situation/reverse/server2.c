#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "utils.c"

#define RCVSIZE 1024
#define MAX(a, b) (((a) > (b)) ? (a) : (b))


int recvMessage(int sockfd, char* buffer, int buffersize, struct sockaddr* client, socklen_t* addrlen) {
  
  memset(buffer, 0, buffersize);
  printf("receiving.\n");
  int msgSize = recvfrom(sockfd, buffer, buffersize, 0, client, addrlen);
  printf("received.\n");
  if (msgSize > 0)
  {
    printf("%s\n", buffer);
  }

  return msgSize;
}

int inputMessage(int sockfd, char* buffer, int buffersize, struct sockaddr* client, socklen_t* addrlen) {

  memset(buffer, 0, buffersize);
  fgets(buffer, buffersize, stdin);
  printf("read %ld bytes from stdin\n", strlen(buffer));
  int sent = sendto(sockfd, buffer, strlen(buffer), 0, client, *addrlen);
  printf("sent %d bytes : %s\n", sent, buffer);

  return sent;
}

int sendMessage(int sockfd, char* buffer, int buffersize, struct sockaddr* client, socklen_t* addrlen) {

  int sent = sendto(sockfd, buffer, strlen(buffer), 0, client, *addrlen);
  printf("sent %d bytes : %s\n", sent, buffer);
  memset(buffer, 0, buffersize);

  return sent;
}

int main(int argc, char *argv[])
{

  if (argc < 2)
  {
    printf("usage ./server <port>\n");
    exit(1);
  }

  int known = argc > 2;

  struct sockaddr_in adresse_udp, client;
  int port = atoi(argv[1]);
  printf("PORT = %d\n", port);
  int valid = 1;
  socklen_t alen = sizeof(client);
  char buffer[RCVSIZE];

  //create socket
  int server_desc_udp = socket(AF_INET, SOCK_DGRAM, 0);

  //handle error
  if (server_desc_udp < 0)
  {
    perror("Cannot create socket\n");
    return -1;
  }

  setsockopt(server_desc_udp, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));
  setsockopt(server_desc_udp, SOL_SOCKET, SO_REUSEPORT, &valid, sizeof(int));

  adresse_udp.sin_family = AF_INET;
  adresse_udp.sin_port = htons(port);
  adresse_udp.sin_addr.s_addr = htonl(INADDR_ANY);

  //initialize socket
  if (
      bind(server_desc_udp, (struct sockaddr *)&adresse_udp, sizeof(adresse_udp)) == -1)
  {
    perror("Bind failed\n");
    return -1;
  }

if (!known) goto evloop;

  printf("=== known situation\n");
  int SYNLEN = recvMessage(server_desc_udp, buffer, 4, (struct sockaddr *)&client, &alen);
  printf("=== SYN OK\n");

  memset(buffer, 0, RCVSIZE);
  strcpy(buffer, "SYN-ACK2002");
  compact_print_buffer(buffer, RCVSIZE);

  sendMessage(server_desc_udp, buffer, strlen(buffer), (struct sockaddr*)&client, &alen);

  int ACKLEN = recvMessage(server_desc_udp, buffer, RCVSIZE, (struct sockaddr *)&client, &alen);

evloop:
  printf("Ev loop start\n");
  while (1)
  {

    printf("Waiting for events\n");
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(0, &readset);
    FD_SET(server_desc_udp, &readset);
    select(MAX(0, server_desc_udp) + 1, &readset, NULL, NULL, 0);

    int input = FD_ISSET(0, &readset);
    int event = input || FD_ISSET(server_desc_udp, &readset);
    if (!event)
    {
      printf("no event wtf\n");
      exit(1);
    }

    if (input)
    {
      inputMessage(server_desc_udp, buffer, RCVSIZE, (struct sockaddr *)&client, &alen);
    }
    else
    {
      recvMessage(server_desc_udp, buffer, sizeof(buffer), (struct sockaddr *)&client, &alen);
    }
  }

  close(server_desc_udp);
  return 0;
}
