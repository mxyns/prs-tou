#include "tou.h"
#include "tou_window.h"
#include "tou_packet.h"

int main() {

    tou_window* win = tou_make_window(TOU_DEFAULT_RECVWINDOW_SIZE, TOU_DEFAULT_MSS, TOU_DEFAULT_EXPECTED_ID);
    tou_sll_dump(win->list);
    tou_packet_dtp* pkt = tou_sll_insert(win->list, 1);
    tou_sll_dump(win->list);
    printf("pkt = %p\n\n", pkt);
    pkt->packet_id = 2;
    tou_sll_dump(win->list);
}