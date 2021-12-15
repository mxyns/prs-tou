all : serveur1-LesSemiCroustillants serveur2-LesSemiCroustillants serveur3-LesSemiCroustillants

serveur1-LesSemiCroustillants : reconf tou.c tou_handshake.c tou_io.c tou_rel.c tou_utils.c tou_window.c tou_datastructs.c tou_socket.c tou_packet.c tou_conn.c tou.h tou_handshake.h tou_utils.h tou_io.h tou_rel.h tou_window.h tou_datastructs.h tou_socket.h tou_packet.h tou_conn.h tou_consts.h
	make -f Makefile.1 serveur1-LesSemiCroustillants
	mkdir -p ../bin
	cp serveur1-LesSemiCroustillants ../bin/serveur1-LesSemiCroustillants

serveur2-LesSemiCroustillants : reconf tou.c tou_handshake.c tou_io.c tou_rel.c tou_utils.c tou_window.c tou_datastructs.c tou_socket.c tou_packet.c tou_conn.c tou.h tou_handshake.h tou_utils.h tou_io.h tou_rel.h tou_window.h tou_datastructs.h tou_socket.h tou_packet.h tou_conn.h tou_consts.h
	make -f Makefile.1 serveur2-LesSemiCroustillants
	mkdir -p ../bin
	cp serveur2-LesSemiCroustillants ../bin/serveur2-LesSemiCroustillants

serveur3-LesSemiCroustillants : reconf tou.c tou_handshake.c tou_io.c tou_rel.c tou_utils.c tou_window.c tou_datastructs.c tou_socket.c tou_packet.c tou_conn.c tou.h tou_handshake.h tou_utils.h tou_io.h tou_rel.h tou_window.h tou_datastructs.h tou_socket.h tou_packet.h tou_conn.h tou_consts.h
	make -f Makefile.1 serveur3-LesSemiCroustillants
	mkdir -p ../bin
	cp serveur3-LesSemiCroustillants ../bin/serveur3-LesSemiCroustillants

reconf : 
	aclocal
	autoconf
	autoheader
	automake --add-missing
	./configure

clean :
	make -f Makefile.1 clean
