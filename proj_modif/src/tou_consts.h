#ifndef TOU_CONSTS_H
#define TOU_CONSTS_H
#define TOU_DEFAULT_MSS (1500) // max payload size

#define TOU_DEFAULT_RECVWINDOW_SIZE (10)

#define TOU_DEFAULT_SENDWINDOW_SIZE 100
#define TOU_DEFAULT_RECV_WORKBUFFER_SIZE (TOU_DEFAULT_MSS * 3)

#define TOU_DEFAULT_RECV_INBUFFER_SIZE (TOU_DEFAULT_MSS * 3)

#define TOU_DEFAULT_SEND_WORKBUFFER_SIZE (TOU_DEFAULT_MSS * 3)

#define TOU_DEFAULT_SEND_OUTBUFFER_SIZE (TOU_DEFAULT_MSS * TOU_DEFAULT_SENDWINDOW_SIZE)

#define TOU_DEFAULT_RECV_CTRLBUFFER_SIZE ((TOU_LEN_PKT_ACK + sizeof(ack_id_t) * TOU_PKT_ACK_MAX_ACK_COUNT) * 5)

#define TOU_DEFAULT_ACK_TIMEOUT_MS (10)

#define TOU_DEFAULT_FAST_RETRANSMIT_ACK_COUNT (2)

// tou_retransmit_all(conn)
// tou_retransmit_expired(conn)
// tou_retransmit_n(conn, n)
// tou_retransmit_id(conn, id)
// tou_retransmit(conn, pkt)
#define TOU_DEFAULT_RETRANSMIT_METHOD(conn, expired_packet, dropped_id, ack_stack_count) tou_retransmit_n(conn, dropped_id)

#endif