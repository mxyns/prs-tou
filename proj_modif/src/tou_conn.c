#include <stdlib.h>
#include "tou.h"
#include "tou_conn.h"

tou_conn* tou_make_conn(
    tou_socket* ctrl_sock,
    tou_socket* socket
) {

    tou_conn* conn = (tou_conn*) calloc(
                                            1, 
                                            sizeof(tou_conn) \
                                            + sizeof(tou_cbuffer) \
                                            + TOU_DEFAULT_RECV_WORKBUFFER_SIZE \
                                            + sizeof(tou_cbuffer) \
                                            + TOU_DEFAULT_RECV_INBUFFER_SIZE \
                                            + sizeof(tou_cbuffer) \
                                            + TOU_DEFAULT_SEND_WORKBUFFER_SIZE \
                                            + sizeof(tou_cbuffer) \
                                            + TOU_DEFAULT_SEND_OUTBUFFER_SIZE \
                                            + sizeof(tou_cbuffer) \
                                            + TOU_DEFAULT_RECV_CTRLBUFFER_SIZE \
                                            /* end */
                                        );
    conn->socket = socket;
    conn->ctrl_socket = ctrl_sock;
    conn->last_packet_id = 0;
    conn->recv_work_buffer = (tou_cbuffer*) (conn + 1);
    conn->recv_work_buffer->buffer = (char*) (conn->recv_work_buffer + 1);
    conn->recv_work_buffer->cap = TOU_DEFAULT_RECV_WORKBUFFER_SIZE;

    conn->in = (tou_cbuffer*) (conn->recv_work_buffer->buffer + conn->recv_work_buffer->cap);
    conn->in->buffer = (char*) (conn->in + 1);
    conn->in->cap = TOU_DEFAULT_RECV_INBUFFER_SIZE;

    conn->send_work_buffer = (tou_cbuffer*) (conn->in->buffer + TOU_DEFAULT_RECV_INBUFFER_SIZE);
    conn->send_work_buffer->buffer = (char*) (conn->send_work_buffer + 1);
    conn->send_work_buffer->cap = TOU_DEFAULT_SEND_WORKBUFFER_SIZE;
    
    conn->out = (tou_cbuffer*) (conn->send_work_buffer->buffer + TOU_DEFAULT_SEND_WORKBUFFER_SIZE);
    conn->out->buffer = (char*) (conn->out + 1);
    conn->out->cap = TOU_DEFAULT_SEND_OUTBUFFER_SIZE;

    conn->ctrl_buffer = (tou_cbuffer*) (conn->out->buffer + TOU_DEFAULT_SEND_OUTBUFFER_SIZE);
    conn->ctrl_buffer->buffer = (char*) (conn->ctrl_buffer + 1);
    conn->ctrl_buffer->cap = TOU_DEFAULT_RECV_CTRLBUFFER_SIZE;
    
    conn->recv_window = tou_make_window(TOU_DEFAULT_RECVWINDOW_SIZE, TOU_DEFAULT_MSS, TOU_DEFAULT_EXPECTED_ID);    
    conn->send_window = tou_make_window(TOU_DEFAULT_SENDWINDOW_SIZE, TOU_DEFAULT_MSS, TOU_DEFAULT_EXPECTED_ID);
    conn->send_window->expected = 1;

    return conn;
}

void tou_free_conn(
    tou_conn* conn
) {
    tou_free_socket(conn->socket);
    tou_free_socket(conn->ctrl_socket);

    tou_free_window(conn->recv_window);
    tou_free_window(conn->send_window);

    // frees 
    // - tou_conn struct
    // - tou_cbuffer recv_work_buffer 
    // - recv_work_buffer's underlying buffer
    // all together
    free(conn);
}
