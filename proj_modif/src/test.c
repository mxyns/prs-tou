#include <stdlib.h>
#include <stdio.h>

int main() {


    char msg[7 + 4 + 1] = "SYN-ACK";
    int port = 1235;

    msg[7] = 48 + (port / 1000) % 10;
    msg[8] = 48 + (port / 100) % 10;
    msg[9] = 48 + (port / 10) % 10;
    msg[10] = 48 + (port / 1) % 10;

    printf("%s\n", msg);
}