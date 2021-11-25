#ifndef TOU_DATASTRUCTS_H
#define TOU_DATASTRUCTS_H

#include <stdint.h>
#include "tou_socket.h"

// circular buffer
typedef struct {

    char* buffer; // data buffer
    int cap; // buffer capacity (size of buffer)
    int cnt; // content count : tail = buffer + (offset + bfsz) % cap
    int head; // current head (next byte to read)

} tou_cbuffer;

#define TOU_CBUFFER_AVAILABLE(buff) (((buff)->cap) - ((buff)->cnt))

// sorted linked list node
typedef struct tou_sll_node tou_sll_node;
struct tou_sll_node {
    tou_sll_node* next;
    void* val;
    uint32_t key;
    char used;
};

typedef tou_sll_node tou_sll_head;
typedef tou_sll_node tou_sll_tail;

// sorted linked list 
typedef struct {
    tou_sll_head* head; // first node
    tou_sll_tail* last; // last node with data
    int count;
    int cap;
} tou_sll;

#define TOU_SLL_ISTAIL(node_ptr) (node_ptr->next == NULL)
#define TOU_SLL_ISFULL(list_ptr) (list_ptr->count == list_ptr->cap && TOU_SLL_ISTAIL(list_ptr->last))
#define TOU_SLL_ISEMPTY(list_ptr) (list_ptr->count == 0 && list_ptr->last == NULL)
#define TOU_SLL_ITER_USED(list, func_call) {tou_sll_node* curr = (list)->head; while(curr != NULL && curr->used) {func_call; curr=curr->next;}}
#define TOU_SLL_ITER_ALL(list, func_call) {tou_sll_node* curr = (list)->head; while(curr != NULL) {func_call; curr=curr->next;}}

// doesnt free anything else that tou_sll struct
#define TOU_SLL_FREE_NONE 0

// run through list and find lowest node->val ptr to free
// used when underlying data has been allocated with malloc/calloc as a single contiguous block
// eg : array
#define TOU_SLL_FREE_FIRST 1

// run through list and call free on each node->val found
#define TOU_SLL_FREE_ALL 2


int tou_cbuffer_read(
        tou_socket* sock,
        tou_cbuffer* cbuff,
        int count
);

int tou_cbuffer_insert(
        tou_cbuffer* cbuff,
        char* buffer,
        int size
);

int tou_cbuffer_pop(
        tou_cbuffer* cbuff,
        char* buffer,
        int size
);

char* tou_cbuffer_peek(
        tou_cbuffer* cbuff,
        int pos
);

void tou_cbuffer_dump(
        tou_cbuffer* buffer
);

void tou_cbuffer_cdump(
        tou_cbuffer* buffer
);

tou_sll* tou_sll_new(
        int size
);

int tou_sll_insert_overwrite(
        tou_sll* list,
        uint32_t priority,
        void* value
);

void* tou_sll_insert(
        tou_sll* list,
        uint32_t priority
);

void* tou_sll_pop(
        tou_sll* list,
        char* err
);

int tou_sll_remove_under(
        tou_sll* list,
        int key,
        tou_sll_node** removed_nodes
);

int tou_sll_remove_keys(
        tou_sll* list,
        uint32_t* keys,
        void** vals,
        size_t val_size,
        int keys_count
);

void tou_sll_dump(
        tou_sll* list
);

void tou_free_sll(
        tou_sll* list,
        int should_free_values
);


#endif