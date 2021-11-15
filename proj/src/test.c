#include "tou_datastructs.h"
#include "tou_packet.h"
#include <stdlib.h>

int tou_sll_remove_keys(
    tou_sll *list,
    uint32_t *keys,
    void *vals,
    int keys_count)
{
    tou_sll_node *prev = NULL;
    int removed = 0;
    {
        tou_sll_node *curr = (list)->head;
        while (curr != NULL && curr->used)
        {
            tou_packet_dtp *pkt = (tou_packet_dtp *)curr->val;
            if (pkt->acked)
            {
                continue;
            }
            for (int i = 0; i < keys_count; i++)
            {
                if (curr->key == keys[i])
                {
                    if (list->count == 0)
                    {
                        return -removed;
                    }
                    if (list->head == curr)
                    {
                        char err = 0;
                        tou_sll_pop(list, err);
                        if (err > 0)
                            return -removed;
                        vals[removed++] = curr->val;
                        break;
                    }
                    curr->key = 0;
                    curr->used = 0;
                    list->count--;
                    if (!(curr->next == ((void *)0)))
                    {
                        prev->next = curr->next;
                        curr->next = list->last->next;
                        list->last->next = curr;
                    }
                    if (curr == list->last)
                    {
                        list->last = prev;
                    }
                    vals[removed++] = curr->val;
                    break;
                }
            }
            prev = curr;
            ;
            curr = curr->next;
        }
    }
}