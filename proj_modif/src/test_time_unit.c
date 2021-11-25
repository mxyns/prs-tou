//
// Created by mxpr on 23/11/2021.
//
#include "tou_utils.h"
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

int main() {

    struct timespec tsp = {

        .tv_sec = 0,
        .tv_nsec = 0,
    };
    int res = clock_getres(CLOCK_REALTIME, &tsp);
    printf("clock res = %d %lds %ldus\n", res, tsp.tv_sec, tsp.tv_nsec);

    struct timespec spec;

    
    useconds_t u = 1000000;
    double secs = 1.234;
    useconds_t usecs = (useconds_t) (u * secs);

    long start = tou_time_ms();
    clock_gettime(CLOCK_REALTIME, &spec);
    long ms = spec.tv_sec * 1000.0 + spec.tv_nsec / 1000000.0;

    usleep(usecs);

    long stop = tou_time_ms();
    clock_gettime(CLOCK_REALTIME, &spec);

    long ms2 = spec.tv_sec * 1000.0 + spec.tv_nsec / 1000000.0;


    printf("%ld -> %ld, delta = %lf\n", start, stop, (stop-start)/1000.0);
    printf("%ld -> %ld, delta = %ld\n", ms, ms2, (ms2-ms));

    return 0;
}