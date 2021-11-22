#include <stdlib.h>
#include "tou.h"
#include "tou_utils.h"

#define N 8

void insert_test() {

    printf("\n=== insert_test\n");
    char buffer[N];
    memset(buffer, 0, N);

    tou_cbuffer cb = {0};
    cb.buffer = buffer;
    cb.cap = N;

    tou_cbuffer_dump(&cb);

    for (int i = 0; i < N + 1; i++) {
        char insert[1] = {1 + i};
        tou_cbuffer_insert(&cb, insert, 1);
        tou_cbuffer_dump(&cb);
    }
}

void pop_test() {

    printf("\n=== pop_test\n");
    char buffer[N];
    for (int i = 0; i < N; i++) buffer[i] = i;

    tou_cbuffer cb = {0};
    cb.buffer = buffer;
    cb.cap = N;
    cb.cnt = N;

    tou_cbuffer_dump(&cb);

    for (int i = 0; i < N + 1; i++) {
        char pop[1] = {-1};
        int n = tou_cbuffer_pop(&cb, pop, 1);
        if (n > 0) printf("popped %d\n", pop[0]);
        else printf("cbuff empty cant pop\n");
        tou_cbuffer_dump(&cb);
    }
}

void string_test() {

    printf("\n=== string_test\n");
    char pute[6] = "pute\n";
    char salope[8] = "salope\n";
    char string[100];

    char buffer[25];
    memset(buffer, 0, 25);

    tou_cbuffer cb = {0};
    cb.buffer = buffer;
    cb.cap = 25;
    cb.cnt = 0;

    printf("pute = [");
    for (int i = 0; i < strlen(pute) + 1; i++) printf("%d ", pute[i]);
    printf("]\n");
    printf("salope = [");
    for (int i = 0; i < strlen(salope) + 1; i++) printf("%d ", salope[i]);
    printf("]\n");

    tou_cbuffer_dump(&cb);
    tou_cbuffer_insert(&cb, pute, strlen(pute) + 1);
    tou_cbuffer_dump(&cb);
    tou_cbuffer_pop(&cb, string,
            // strlen(pute)
                    4
    );
    tou_cbuffer_dump(&cb);
    printf("%s\n", string);
    memset(string, 0, 100);
    tou_cbuffer_insert(&cb, salope, strlen(salope) + 1);
    tou_cbuffer_dump(&cb);
    tou_cbuffer_pop(&cb, string,
            // strlen(salope)
                    4
    );
    tou_cbuffer_dump(&cb);
    printf("%s\n", string);
    memset(string, 0, 100);
    tou_cbuffer_pop(&cb, string,
            // strlen(salope)
                    4
    );
    tou_cbuffer_dump(&cb);
    printf("%s\n", string);
    memset(string, 0, 100);
}

void bounds_test() {

    int n = 5;
    char buffer[n];
    memset(buffer, 0, n);
    for (int i = 0; i < n; i++) buffer[i] = i;

    tou_cbuffer cb = {0};
    cb.buffer = buffer;
    cb.cap = n;
    cb.cnt = 0;

    char sink[1024];

    tou_cbuffer_dump(&cb);
    tou_cbuffer_insert(&cb, "99999999", 8);
    tou_cbuffer_dump(&cb);
    tou_cbuffer_insert(&cb, "88888", 5);
    tou_cbuffer_dump(&cb);
    int k = tou_cbuffer_pop(&cb, sink, 3);
    printf("popped %d = %d\n", k, 3);
    tou_cbuffer_dump(&cb);
    tou_cbuffer_insert(&cb, "88888", 5);
    tou_cbuffer_dump(&cb);
    k = tou_cbuffer_pop(&cb, sink, 3);
    printf("popped %d = %d\n", k, 3);
    k = tou_cbuffer_pop(&cb, sink, 3);
    printf("popped %d = %d\n", k, 3);
    k = tou_cbuffer_pop(&cb, sink, 3);
    printf("popped %d = %d\n", k, 3);

}

int main() {

    // insert_test();
    // pop_test();
    //string_test();

    bounds_test();
}