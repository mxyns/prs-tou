#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

int main() {

    long timeout_ms = 0;

    struct timeval t = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = 1000 * (timeout_ms % 1000),
    };

    printf("before\n");
    int r = select(0, NULL, NULL, NULL, &t);
    printf("after %d\n", r);
}