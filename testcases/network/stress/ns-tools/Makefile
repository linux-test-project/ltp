SCRIPTS=check_envval \
	get_ifname initialize_if set_ipv4addr add_ipv6addr \
	check_icmpv4_connectivity check_icmpv6_connectivity \
	check_netem check_setkey \
	create_file \
	find_portbundle \
	bg_tcp_traffic \
	killall_icmp_traffic killall_tcp_traffic killall_udp_traffic \
	output_ipsec_conf \
	ns-echoclient

BINS=ns-tcpserver ns-tcpclient ns-udpserver ns-udpclient \
     ns-icmpv4_sender ns-icmpv6_sender \
     ns-udpsender \
     ns-icmp_redirector \
     ns-mcast_receiver ns-igmp_querier \
     ns-mcast_join 

TARGETS=$(SCRIPTS) $(BINS)

#CC=gcc
#CFLAGS=-g -Wall
#CFLAGS=-O2

all:	scripts $(BINS)

ns-tcpserver:	ns-common.o ns-tcpserver.o
ns-tcpclient:	ns-common.o ns-tcpclient.o
ns-udpserver:	ns-common.o ns-udpserver.o
ns-udpclient:	ns-common.o ns-udpclient.o
ns-icmpv4_sender:	ns-common.o ns-icmpv4_sender.o
ns-icmpv6_sender:	ns-common.o ns-icmpv6_sender.o
ns-udpsender:	ns-common.o ns-udpsender.o
ns-icmp_redirector:	ns-common.o ns-icmp_redirector.o
ns-mcast_receiver:	ns-common.o ns-mcast_receiver.o
ns-igmp_querier:	ns-common.o ns-igmp_querier.o
ns-mcast_join:	ns-common.o ns-mcast_join.o

ns-common.o: 	ns-traffic.h ns-common.c 
ns-tcpserver.o:	ns-traffic.h ns-tcpserver.c
ns-tcpclient.o:	ns-traffic.h ns-tcpclient.c
ns-udpserver.o:	ns-traffic.h ns-udpserver.c
ns-udpclient.o:	ns-traffic.h ns-udpclient.c
ns-icmpv4_sender.o:	ns-traffic.h ns-icmpv4_sender.c
ns-icmpv6_sender.o:	ns-traffic.h ns-icmpv6_sender.c
ns-udpsender.o:	ns-traffic.h ns-udpsender.c
ns-icmp_redirector.o:	ns-traffic.h ns-icmp_redirector.c
ns-mcast_receiver.o:	ns-traffic.h ns-mcast.h ns-mcast_receiver.c
ns-igmp_querier.o:	ns-traffic.h ns-mcast.h ns-igmp_querier.c
ns-mcast_join.o:	ns-traffic.h ns-mcast.h ns-mcast_join.c

scripts:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done

clean:
	@set -e; for i in $(TARGETS); do rm -f ../../../bin/$$i ; done
	rm -f $(BINS) *.o
	@true
