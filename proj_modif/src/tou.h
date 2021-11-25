#ifndef TOU_tou_H
#define TOU_tou_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "tou_socket.h"
#include "tou_conn.h"
#include "tou_packet.h"
#include "tou_rel.h"

// TODO check for other problems like packet->acked that wasn't reset properly for each fucking tou_sll existing

/*
    packet_type(char)
*/
#define TOU_LEN_BASEHEADER (0)

#define TOU_HANDSHAKE_ACK_DEFAULT_TIMEOUT (0)

#define TOU_DEFAULT_EXPECTED_ID (1)

#define TOU_READ_EXACT 0

// similar to tcp socket => bind => listen
tou_socket* tou_open_socket(
        const char* listen_ip,
        int listen_port
);

// simital to tcp connect
tou_conn* tou_connect(
        const char* target_ip,
        int target_port
);

// similar to tcp accept
tou_conn* tou_accept_conn(
        tou_socket* listen_sock
);

// set socket opt non-blocking behaviour
#define TOU_FLAG_NONBLOCKING_DATA (1)
#define TOU_FLAG_NONBLOCKING_DATA_ENABLE (1)
#define TOU_FLAG_NONBLOCKING_CTRL (1 << 1)
#define TOU_FLAG_NONBLOCKING_CTRL_ENABLE (1 << 2)

void tou_set_nonblocking(
        tou_conn* conn,
        char flags
);

// similar to recvfrom but for a tou_conn
// receive and process packet until at least n bytes are available
// first bytes in recv stream are copied to buffer argument
// bytes written to buffer are consumed
// modes :
//  exact mode      = TOU_READ_EXACT   : awaits for at least n bytes and writes exactly n bytes to buffer
//  at least mode   = N != 0           : awaits for at least n bytes and writes k bytes to buffer with : n <= k <= N
//                                       this means N must be less or equal to buffer size to avoid buffer overflow
// returns number of bytes written to buffer
int tou_read(
        tou_conn* conn,
        char* buffer,
        int n,
        int mode
);

int tou_retransmit(
        tou_conn* conn,
        tou_packet_dtp* pkt
);

int tou_retransmit_all(
        tou_conn* conn
);

int tou_retransmit_n(
        tou_conn* conn,
        int n
);

int tou_retransmit_expired(
        tou_conn* conn,
        long expire_time
);

int tou_retransmit_id(
        tou_conn* conn,
        uint32_t id
);

#endif