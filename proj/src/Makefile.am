noinst_PROGRAMS=cserver \
                cclient \
                test_cbuff \
                test_sll \
                test_pserver \
                test_pclient \
                test_window \
                test_cblocks \
                test_gotos \
                test_time \
                #end 

AM_CPPFLAGS = -I$(srcdir)
LDADD = -lm

TOU_CORE = tou.c \
            tou_handshake.c \
            tou_io.c \
            tou_rel.c \
            tou_utils.c \
            tou_window.c \
            tou_datastructs.c \
            tou_socket.c \
            tou_packet.c \
            tou_conn.c \
            #end

noinst_HEADERS= tou.h \
                    tou_handshake.h \
                    tou_utils.h \
                    tou_io.h \
                    tou_rel.h \
                    tou_window.h \
                    tou_datastructs.h \
                    tou_socket.h \
                    tou_packet.h \
                    tou_conn.h \
                    #end

cserver_SOURCES= $(TOU_CORE) \
                server_classic.c \
                #end

cclient_SOURCES= $(TOU_CORE) \
                client_classic.c
                #end

test_cbuff_SOURCES = $(TOU_CORE) \
                test_cbuff.c \
                #end

test_sll_SOURCES = $(TOU_CORE) \
                test_sll.c \
                #end

test_pserver_SOURCES = $(TOU_CORE) \
                test_packet_server.c \
                #end
test_pclient_SOURCES = $(TOU_CORE) \
                test_packet_client.c \
                #end

test_window_SOURCES = $(TOU_CORE) \
                test_window.c \
                #end

test_cblocks_SOURCES = test_cblocks.c
test_gotos_SOURCES = test_gotos.c

test_time_SOURCES = test_time.c