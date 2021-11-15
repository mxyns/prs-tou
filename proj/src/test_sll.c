#include "tou_datastructs.h"
#include <stdio.h>
#include <stdlib.h>

#define N 4

void test_tou_sll_insert_overwrite(tou_sll* list, int* arr) {
    
    tou_sll_node* c = list->head;
    for (int i = 0; i < N; i++) {

        printf("[%d] {%d} == {%p}\n", i, arr[i], c->val);
        c = c->next;
    }

    for (int i = 0; i < N + 1; i++) {
        if (!tou_sll_insert_overwrite(list, i % 3, arr + i))
            tou_sll_dump(list);
        else
            printf("insert failed : sll full ? %s\n", TOU_SLL_ISFULL(list) ? "YES" : "NO");
    }

    char err;
    for (int i = 0; i < N + 1; i++) {
        void* pop = tou_sll_pop(list, &err);
        if (!err)
            printf("popped : %d\n", *(int*)pop);
        tou_sll_dump(list);
    }
}

void test_tou_sll_insert(tou_sll* list, int* arr) {

    tou_sll_node* c = list->head;
    for (int i = 0; i < N; i++) {

        printf("[%d] {%d} == {%p}\n", i, arr[i], c->val);
        c->val = arr + i;
        c = c->next;
    }

    for (int i = 0; i < N + 1; i++) {
        void* full = tou_sll_insert(list, i % 3);
        if (full != NULL)
            tou_sll_dump(list);
        else
            printf("insert failed : sll full ? %s\n", TOU_SLL_ISFULL(list) ? "YES" : "NO");
    }

    char err;
    for (int i = 0; i < N + 1; i++) {
        void* pop = tou_sll_pop(list, &err);
        if (!err)
            printf("popped : %d\n", *(int*)pop);
        tou_sll_dump(list);
    }
}

void test_tou_sll_remove(tou_sll* list, int* arr) {

    tou_sll_node* c = list->head;
    for (int i = 0; i < N; i++) {

        printf("[%d] {%d} == {%p}\n", i, arr[i], c->val);
        c->val = arr + i;
        c = c->next;
    }

    for (int i = 0; i < N + 1; i++) {
        void* full = tou_sll_insert(list, i % 3);
        if (full != NULL) {
            *(int*)full = arr[i];
            tou_sll_dump(list);
        } else {
            printf("insert failed : sll full ? %s\n", TOU_SLL_ISFULL(list) ? "YES" : "NO");
        }
    }


    tou_sll_dump(list);
    
    printf("\n\n----- removing\n");
    char err;
    #define n 2
    uint32_t keys[n] = {0, 1};
    int* vals[n] = {NULL, NULL};
    int removed = tou_sll_remove_keys(list, keys, (void**)vals, sizeof(int), n);
    printf("removed %d : [ ", removed);
    for (int i = 0 ; i < n; i++) {
        if (vals[i] != NULL) printf("%d ", *vals[i]);
        else printf("(nil) ");
    }
    printf("]\n");
    tou_sll_dump(list);
}

int main() {

    tou_sll* list = tou_sll_new(N);
    int* pute = (int*) calloc(1, N * sizeof(int));

    for (int i = 0; i < N; i++) {
        pute[i] = 2*N-i;
    }

    test_tou_sll_insert(list, pute);

    printf("\n\n\n\n\n\n==== test_tou_sll_remove\n");
    test_tou_sll_remove(list, pute);

    // free(pute);
    tou_free_sll(list, TOU_SLL_FREE_FIRST);
}