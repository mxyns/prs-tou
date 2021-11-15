#include "tou.h"
#include "tou_utils.h"
#include <time.h>
#include <math.h>

void printNZeros(int n, int maxSeries) {
    
    if (n > 0 && n < maxSeries) {
        for (;n> 0; n--) printf("0 ");
    } else if (n > 0) {
        printf("%d**[0] ", n);
    }

}

void compact_print_buffer(
    char* buffer,
    int size
) {

    printf("[\n\t");    
    int zeros = 0;
    for (int i = 0; i < size; i++) {

        if (buffer[i] == 0) {
            zeros++;

            if (i != size - 1) continue;
        }

        printNZeros(zeros, 5);
        zeros=0;

        if (buffer[i] != 0)
            printf("%d ", buffer[i]);
    }
    printf("\n]\n");
}

long tou_time_ms() {
    
    long            ms; // Milliseconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        ms = 1000;
    }
    ms += spec.tv_sec * 1.0e6;

    return ms;
}