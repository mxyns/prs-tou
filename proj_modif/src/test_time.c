#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

int main() {

    long            ms; // Milliseconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        ms = 1000;
    }
    ms += spec.tv_sec * 1.0e6;

    printf("%d sec %ld ms\n", 0, ms);
}