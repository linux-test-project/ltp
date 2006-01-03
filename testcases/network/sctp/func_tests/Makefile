#
#  (C) Copyright IBM Corp. 2001, 2003
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

###########################################################################
# name of file	: Makefile						  #
###########################################################################
INCLUDES += -I../../../../include -I../include -I../testlib
LIBS += -L../../../../lib -L../lib -L../testlib

CFLAGS += -g $(INCLUDES) -DLTP

LOADLIBES += $(LIBS) -lltp -lsctputil -lsctp -lpthread

V6FLAGS = -DTEST_V6=1 $(CFLAGS)

SRCS = $(wildcard *.c)

V4_TARGETS = test_1_to_1_accept_close test_1_to_1_addrs test_1_to_1_connect \
             test_1_to_1_events test_1_to_1_initmsg_connect \
             test_1_to_1_nonblock test_1_to_1_recvfrom test_1_to_1_recvmsg \
             test_1_to_1_rtoinfo test_1_to_1_send test_1_to_1_sendmsg \
             test_1_to_1_sendto test_1_to_1_shutdown \
             test_1_to_1_socket_bind_listen test_1_to_1_sockopt \
             test_1_to_1_threads test_assoc_abort test_assoc_shutdown \
             test_autoclose test_basic test_connect test_fragments \
             test_getname test_inaddr_any test_peeloff test_recvmsg \
             test_sctp_sendrecvmsg test_sockopt test_tcp_style \
             test_timetolive test_1_to_1_connectx test_connectx

V6_TARGETS = test_basic_v6 test_fragments_v6 test_getname_v6 \
             test_inaddr_any_v6 test_peeloff_v6 test_sctp_sendrecvmsg_v6 \
             test_sockopt_v6 test_tcp_style_v6 test_timetolive_v6

all: $(V4_TARGETS) $(V6_TARGETS)

test_1_to_1_accept_close: test_1_to_1_accept_close.c
test_1_to_1_addrs: test_1_to_1_addrs.c
test_1_to_1_connect: test_1_to_1_connect.c
test_1_to_1_events: test_1_to_1_events.c
test_1_to_1_initmsg_connect: test_1_to_1_initmsg_connect.c
test_1_to_1_nonblock: test_1_to_1_nonblock.c
test_1_to_1_recvfrom: test_1_to_1_recvfrom.c
test_1_to_1_recvmsg: test_1_to_1_recvmsg.c
test_1_to_1_rtoinfo: test_1_to_1_rtoinfo.c
test_1_to_1_send: test_1_to_1_send.c
test_1_to_1_sendmsg: test_1_to_1_sendmsg.c
test_1_to_1_sendto: test_1_to_1_sendto.c
test_1_to_1_shutdown: test_1_to_1_shutdown.c
test_1_to_1_socket_bind_listen: test_1_to_1_socket_bind_listen.c
test_1_to_1_sockopt: test_1_to_1_sockopt.c
test_1_to_1_threads: test_1_to_1_threads.c
test_assoc_abort: test_assoc_abort.c
test_assoc_shutdown: test_assoc_shutdown.c
test_autoclose: test_autoclose.c
test_basic: test_basic.c
test_basic_v6: test_basic_v6.o
test_connect: test_connect.c
test_fragments: test_fragments.c
test_fragments_v6: test_fragments_v6.o
test_getname: test_getname.c
test_getname_v6: test_getname_v6.o
test_inaddr_any: test_inaddr_any.c
test_inaddr_any_v6: test_inaddr_any_v6.o
test_peeloff: test_peeloff.c
test_peeloff_v6: test_peeloff_v6.o
test_recvmsg: test_recvmsg.c
test_sctp_sendrecvmsg: test_sctp_sendrecvmsg.c
test_sctp_sendrecvmsg_v6: test_sctp_sendrecvmsg_v6.o
test_sockopt: test_sockopt.c
test_sockopt_v6: test_sockopt_v6.o
test_tcp_style: test_tcp_style.c
test_tcp_style_v6: test_tcp_style_v6.o
test_timetolive: test_timetolive.c
test_timetolive_v6: test_timetolive_v6.o
test_1_to_1_connectx: test_1_to_1_connectx.c
test_connectx: test_connectx.c

test_basic_v6.o: test_basic.c
	$(CC) -c $(V6FLAGS) -o $@ $^
test_fragments_v6.o: test_fragments.c
	$(CC) -c $(V6FLAGS) -o $@ $^
test_getname_v6.o: test_getname.c
	$(CC) -c $(V6FLAGS) -o $@ $^
test_inaddr_any_v6.o: test_inaddr_any.c
	$(CC) -c $(V6FLAGS) -o $@ $^
test_peeloff_v6.o: test_peeloff.c
	$(CC) -c $(V6FLAGS) -o $@ $^
test_sctp_sendrecvmsg_v6.o: test_sctp_sendrecvmsg.c
	$(CC) -c $(V6FLAGS) -o $@ $^
test_sockopt_v6.o: test_sockopt.c
	$(CC) -c $(V6FLAGS) -o $@ $^
test_tcp_style_v6.o: test_tcp_style.c
	$(CC) -c $(V6FLAGS) -o $@ $^
test_timetolive_v6.o: test_timetolive.c
	$(CC) -c $(V6FLAGS) -o $@ $^

v4test: ${V4_TARGETS}
	@for a in $^;                           \
	do                                      \
		echo "./$$a";                   \
		if ./$$a;                       \
		then                            \
			echo "$$a passes";      \
			echo "";                \
		else                            \
			echo "$$a fails";       \
			exit 1;                 \
		fi;                             \
	done

v6test: ${V6_TARGETS}
	@for a in $^;                           \
	do                                      \
		echo "./$$a";                   \
		if ./$$a;                       \
		then                            \
			echo "$$a passes";      \
			echo "";                \
		else                            \
			echo "$$a fails";       \
			exit 1;                 \
		fi;                             \
	done

install:
	ln -f $(V4_TARGETS) ../../../bin
	ln -f $(V6_TARGETS) ../../../bin

clean:
	rm -f $(V4_TARGETS)
	rm -f $(V6_TARGETS)
	rm -f ../../../bin/$(V6_TARGETS)
	rm -f *.o
