#include "tou.h"
#include "limits.h"
#include "tou_datastructs.h"
#include "tou_utils.h"
#include <stdlib.h>

// reads at max 'count' bytes from sock into cbuff
// count limited by available buffer space
int tou_cbuffer_read(
        tou_socket* sock,
        tou_cbuffer* cbuff,
        int count
) {
    // if asked for more than what can be stored
    if (count > cbuff->cap)
        return -1;

    // limit to available size
    count = MIN(count, cbuff->cap - cbuff->cnt);
    TOU_DEBUG(printf("[tou][tou_cbuffer_read] really gonna read %d\n", count));

    char buffer[count];

    int recv;
    if ((recv = recvfrom(sock->fd, buffer, count, 0, sock->peer_addr, &sock->peer_addr_len)) < 0) {
        TOU_DEBUG(printf("ERR = %d\n", recv));
        return -1;
    }
    TOU_DEBUG(printf("[tou][tou_cbuffer_read] received %d bytes\n", recv));

    int inserted = tou_cbuffer_insert(cbuff, buffer, recv);
    TOU_DEBUG(printf("[tou][tou_cbuffer_read] inserted %d bytes\n", inserted));
    if (inserted != recv) {
        TOU_DEBUG(printf("[tou][tou_cbuffer_read] %d bytes lost\n", recv - inserted));
        return inserted - recv;
    }

    return inserted;
}

// insert data maximum amount of data after cbuff tail
// doesnt check beforehand if enough space is available in buffer
// return written bytes count n with 0 <= n <= size 
int tou_cbuffer_insert(
        tou_cbuffer* cbuff,
        char* buffer,
        int size
) {

    /*
        toWrite = 5
        head < tail : bounds is first limit
        [ 0 0 0 0 1 2 3 4 0 0 ] h = 4 | t = 8 | c = 10
                  h       t c
        memcpy : start = tail, n = cap - tail
        r = toWrite - n = 5 - 2 = 3 | tail = 0
        now head > tail, do |
                            |
                            V
        head > tail : head is the limit
        [ 4 0 0 0 0 0 0 1 2 3 ]
        memcpy : start = tail, n = head - tail
    */
    // easy peasy
    if (cbuff->cnt == 0) {
        cbuff->head = 0;
        cbuff->cnt = MIN(cbuff->cnt + size, cbuff->cap);
        memcpy(cbuff->buffer, buffer, cbuff->cnt);
        return cbuff->cnt;
    }

    int head = cbuff->head;
    int tail = (cbuff->head + cbuff->cnt) % cbuff->cap;
    if (head == tail) {
        // buffer is full bc cnt != 0 so cnt = cap
        return -1;
    }
    int written = 0;
    if (head < tail) {
        written = MIN(cbuff->cap - tail, size);
        memcpy(cbuff->buffer + tail, buffer, written);
        cbuff->cnt = MIN(cbuff->cnt + written, cbuff->cap);

        if (written == size) return size;
    }

    int new_write = 0;
    if (size - written > 0) {
        tail = (cbuff->head + cbuff->cnt) % cbuff->cap;

        // as much as i can without going past head
        new_write = MIN(size - written, head - tail);

        memcpy(cbuff->buffer + tail, buffer + written, new_write);
        cbuff->cnt = MIN(cbuff->cnt + new_write, cbuff->cap);
    }

    return written + new_write;
}

// returns total popped bytes count
int tou_cbuffer_pop(
        tou_cbuffer* cbuff,
        char* buffer,
        int size
) {

    /*
        toRead = 5                          
        head < tail : bounds is tail      <-----------------x
        [ 0 0 0 0 1 2 3 4 0 0 ] h = 4 | t = 8 | c = 10      |
                  h       t c                               |
        memcpy : start = head, n = tail - head              |
        r = toRead - n = 5 - 4 = 3 | tail = 0               |
                                                            |
        tail < head : bounds is the limit                   |
        [ 4 0 0 0 0 0 0 1 2 3 ] h = 7 | t = 1 | c = 10      |
        memcpy : start = head, n = cap - head               |
        now head = 0 < tail --------------------------------x
    */
    // easy peasy
    if (cbuff->cnt == 0) {
        return 0;
    }

    int tail = (cbuff->head + cbuff->cnt) % cbuff->cap;

    int read = 0;
    if (cbuff->head >= tail) {
        read = MIN(cbuff->cap - cbuff->head, size);
        memcpy(buffer, cbuff->buffer + cbuff->head, read);
        cbuff->cnt = MAX(cbuff->cnt - read, 0);
        cbuff->head = (cbuff->head + read) % cbuff->cap;

        if (read == size) return size;
    } else TOU_DEBUG(printf("skipped first part\n"));

    int new_read = 0;
    if (size - read > 0) {
        tail = (cbuff->head + cbuff->cnt) % cbuff->cap;
        TOU_DEBUG(printf("tail at %d, %d past head at %d\n", tail, tail - cbuff->head, cbuff->head));

        // as much as i can without going past tail
        new_read = MIN(tail - cbuff->head, size - read);
        TOU_DEBUG(printf("gotta read %d\n", new_read));

        memcpy(buffer + read, cbuff->buffer + cbuff->head, new_read);

        cbuff->cnt = MAX(cbuff->cnt - new_read, 0);
        cbuff->head = (cbuff->head + new_read) % cbuff->cap;
    } else TOU_DEBUG(printf("skipped 2nd part\n"));

    return read + new_read;
}

char* tou_cbuffer_peek(
        tou_cbuffer* cbuff,
        int pos
) {

    if (pos > cbuff->cnt) { // no data
        return NULL;
    }

    return (cbuff->buffer + ((cbuff->head + pos) % cbuff->cap));
}


void tou_cbuffer_dump(
        tou_cbuffer* buffer
) {

    printf("tou_cbuffer {\n\tcap=%d\n\tcnt=%d\n}",
           buffer->cap,
           buffer->cnt
    );
    printf(" = [ ");
    for (int i = 0; i < buffer->cap; i++) {
        if (buffer->head == i)
            printf(" {%c} ", (char) buffer->buffer[i]);
        else
            printf(" %c ", (char) buffer->buffer[i]);
    }
    printf("]\n");
}

void tou_cbuffer_cdump(
        tou_cbuffer* cbuffer
) {

    printf("tou_cbuffer {\n\tcap=%d\n\tcnt=%d\n}",
           cbuffer->cap,
           cbuffer->cnt
    );
    printf(" = ");

    compact_print_buffer(cbuffer->buffer, cbuffer->cap);
}

tou_sll* tou_sll_new(
        int size
) {

    if (size < 1) return NULL;

    tou_sll* list = (tou_sll*) calloc(1, sizeof(tou_sll) + size * sizeof(tou_sll_node));
    tou_sll_node* curr = (tou_sll_node*) (list + 1);

    list->head = curr;
    list->cap = size;
    list->count = 0;
    list->last = NULL;

    for (tou_sll_node* next = curr + 1; next < ((tou_sll_node*) (list + 1)) + size; curr = next++) {
        curr->next = next;
    }

    curr->next = NULL;

    return list;
}


// private function used in tou_free_sll that finds node with smallest val ptr value
static inline tou_sll_node
*
tou_sll_find_first(
        tou_sll
* list
) {
tou_sll_node* curr = list->head;
tou_sll_node* min = list->head;
while (curr != NULL) {
min = curr->val < min->val ? curr : min;
curr = curr->next;
}

return
min;
}

void tou_free_sll(
        tou_sll* list,
        int should_free_values
) {
    switch (should_free_values) {
        case TOU_SLL_FREE_FIRST:
            free(tou_sll_find_first(list)->val);
            break;

        case TOU_SLL_FREE_ALL : TOU_SLL_ITER_ALL(list, if (curr->val != NULL) free(curr->val));
            break;

        case TOU_SLL_FREE_NONE:
        default:
            break;
    }

    free(list); // frees all nodes and list struct at once
}

int tou_sll_insert_overwrite(
        tou_sll* list,
        uint32_t key,
        void* value
) {
    if (TOU_SLL_ISFULL(list)) return -1;

    if (TOU_SLL_ISEMPTY(list)) { // insertion in head

        TOU_DEBUG(printf("list is empty\n"));
        list->last = list->head; // next free cell

        // insert data
        list->head->key = key;
        list->head->val = value;
        list->head->used = 1;
        list->count++;

        return 0;
    }

    tou_sll_node * node = list->last->next; // our free node
    tou_sll_node * previous = NULL;
    tou_sll_node * current = list->head;
    list->count++; // at this point we're guaranteed to insert at some point
    node->used = 1;

    while (current != node) {
        if (key < current->key) { // if can insert

            list->last->next = node->next; // set new free cell

            if (previous == NULL) { // if inserting before head
                list->head = node;
            } else {
                previous->next = node;
            }

            // add data
            node->next = current;
            node->key = key;
            node->val = value;

            return 0;
        }

        previous = current;
        current = current->next;
    }

    // insert after last
    node->key = key;
    node->val = value;

    list->last = node;

    return 0;
}

// insert a piece of data with given key(priority) and get the val ptr node->val
// returns node->val or NULL if cant insert bc TOU_SLL_ISFULL 
void* tou_sll_insert(
        tou_sll* list,
        uint32_t key
) {
    if (TOU_SLL_ISFULL(list)) return NULL;

    if (TOU_SLL_ISEMPTY(list)) { // insertion in head

        TOU_DEBUG(printf("list is empty\n"));
        list->last = list->head; // next free cell

        // insert data
        list->head->key = key;
        list->head->used = 1;
        list->count++;

        return list->head->val;
    }

    tou_sll_node * node = list->last->next; // our free node
    tou_sll_node * previous = NULL;
    tou_sll_node * current = list->head;
    list->count++; // at this point we're guaranteed to insert at some point
    node->used = 1;

    while (current != node) {
        if (key < current->key) { // if can insert

            list->last->next = node->next; // set new free cell

            if (previous == NULL) { // if inserting before head
                list->head = node;
            } else {
                previous->next = node;
            }

            // add data
            node->next = current;
            node->key = key;

            return node->val;
        }

        previous = current;
        current = current->next;
    }

    // insert after last
    node->key = key;
    list->last = node;

    return node->val;
}

// pop and return list head value
// node is marked as unused and put at the back of the line
// value stays referenced in tou_sll_node until overwritten by future insert 
void* tou_sll_pop(
        tou_sll* list,
        char* err
) {

    if (list->count == 0) {
        *err = 1;
        return NULL;
    }

    tou_sll_node * popped_head = list->head;

    popped_head->key = 0;
    popped_head->used = 0;
    list->count--;

    if (list->count == 0) { // empty list
        list->last = NULL; // list is empty now
    } else {
        list->head = popped_head->next;
        popped_head->next = list->last->next;
        list->last->next = popped_head;
    }

    *err = 0;
    return popped_head->val;
}

int tou_sll_remove_under(
        tou_sll* list,
        int key,
        tou_sll_node** removed_nodes
) {

    tou_sll_node * curr = list->head;

    char err;
    int n = 0;
    while (curr != NULL && curr->used && curr->key <= key) {
        TOU_DEBUG(
                printf("curr key=%d\n", curr->key);
                printf("popping\n");
        );
        tou_sll_head* head = list->head;
        tou_sll_pop(list, &err);
        TOU_DEBUG(printf("ok\n"));
        if (!err) {
            removed_nodes[n] = head;
            n++;
            TOU_DEBUG(printf("popped %d\n", n));
        }

        curr = list->head;
    }

    return n;
}

int tou_sll_remove_keys(
        tou_sll* list,
        uint32_t* keys,
        void** vals,
        size_t val_size,
        int keys_count
) {
    // TODO find min of ack_list and start looking for packets with key > min(ack_lis)

    int removed = 0;
    tou_sll_node * prev = NULL;
    tou_sll_node * curr = list->head;
    tou_sll_node * new_curr = NULL;

    while (curr != NULL && curr->used) {

        TOU_DEBUG(printf("node key=%d\n", curr->key));

        new_curr = curr;
        for (int i = 0; i < keys_count; i++) {
            TOU_DEBUG(printf("matching with key=%d\n", keys[i]));
            if (curr->key == keys[i]) { // if any key from key list matches current node's key
                TOU_DEBUG(printf("its a match\n"));
                // if list empty stop
                if (list->count == 0) {
                    TOU_DEBUG(printf("list empty stop\n"));
                    return -removed;
                }

                TOU_DEBUG(printf("clearing key=%d\n", curr->key));
                curr->key = 0;
                curr->used = 0;
                list->count--;

                if (list->head == curr) {
                    TOU_DEBUG(printf("removing head\n"));
                    if (list->count == 0) { // empty list
                        list->last = NULL; // list is empty now
                    } else {
                        list->head = curr->next;
                        curr->next = list->last->next;
                        list->last->next = curr;
                    }

                    removed++;
                    vals[MIN(keys_count, removed) - 1] = curr->val;
                    new_curr = list->head;
                    break;
                }

                if (curr == list->last) {
                    TOU_DEBUG(printf("was last\n"));
                    list->last = prev;
                } else if (!TOU_SLL_ISTAIL(curr)) {
                    TOU_DEBUG(printf("not tail\n"));
                    prev->next = curr->next;
                    curr->next = list->last->next;
                    list->last->next = curr;
                }

                new_curr = prev->next;

                removed++;
                vals[MIN(keys_count, removed) - 1] = curr->val;
                break;
            }
        }

        TOU_DEBUG(tou_sll_dump(list));

        if (new_curr == curr) {
            prev = curr;
            curr = curr->next;
        } else {
            curr = new_curr;
        }
    }
    return removed;
}

/*
                if (!tou_window_free_packet(conn->send_window, curr)) {
                    printf("[tou][tou_acknowledge_packet] couldn't free packet %d\n", pkt->packet_id);
                }
*/

void tou_sll_dump(
        tou_sll* list
) {

    printf("tou_sll {\n\tcap=%d\n\tcnt=%d\n}",
           list->cap,
           list->count
    );

    tou_sll_node * current = list->head;

    printf(" = %s:", list->count ? "used" : "free");
    while (current != NULL) {

        if (current == list->last)
            printf(" [{%ld|%d|%p}] \n =>   free:", current - (tou_sll_node*) (list + 1), (int) (current->key),
                   current->val);
        else if (current->used)
            printf(" {%ld|%d|%p} ->", current - (tou_sll_node*) (list + 1), (int) (current->key), current->val);
        else
            printf(" {%ld|X|%p} ->", current - (tou_sll_node*) (list + 1), current->val);
        current = current->next;
    }

    printf(" (nil)\n\n");
}