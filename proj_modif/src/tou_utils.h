#ifndef TOU_UTILS_H
#define TOU_UTILS_H

#define TOU_ARR_INS(arr, pos, type, value) *((type*)((arr) + (pos))) = (value)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(x) (((x) < 0 ? (-(a)) : (a)))



#ifdef DEBUG
#define TOU_DEBUG(X) X
#else
#define TOU_DEBUG(X)
#endif

void printNZeros(int n, int maxSeries);

void compact_print_buffer(
        char* buffer,
        int size
);

long tou_time_ms();

#endif