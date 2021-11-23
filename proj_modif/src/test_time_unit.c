//
// Created by mxpr on 23/11/2021.
//
#include "tou_utils.h"
#include <unistd.h>
#include <stdio.h>

int main() {

    useconds_t u = 1000000;
    double secs = 1;
    useconds_t usecs = (useconds_t) (u * secs);

    long start = tou_time_ms();

    usleep(usecs);

    long stop = tou_time_ms();

    printf("%ld -> %ld, delta = %ld\n", start, stop, (stop-start)/1000);

    return 0;
}