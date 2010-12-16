/*
#
#
#						      Task Subprogram
#
#  SUBPROGRAM NAME: PINGPONG.C
#
#  REQUIRED PARAMETERS:
#    Calling Procedure: pingpong HOST SIZE PACKETS
#       HOST - Current host
#       SIZE  - Size of each packet
#       PACKETS  - the number of packets across the network
#
#  SETUP REQUIRED:
#       o  This task must be run as root.
#       o  TCP/IP must be configured before executing this task.
#
#  DESCRIPTION:
# 	Purpose:To generate lan traffic with ICMP echo packet
# 	Command: None
# 	Subcommand:None
# 	Design:
#		Create raw socket
#		spawn child process to send echo ICMP packet
#		the child process build echo packet and send to network
#		repeat n times
#		the parent goes on to receive the reply
#		when finish print # of packets sent & # pf packets received
#
#
#===========================================================================
*/

#define PS2
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netdb.h>
#include "test.h"
#include "usctest.h"
#include "netdefs.h"

#define	MAXPACKET	4096	/* max packet size */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	64
#endif

#if INET6
char		*TCID = "perf_lan6";
#else
char		*TCID = "perf_lan";
#endif

int		TST_TOTAL = 1;

int		pfd[2];
int		fdstr[10];
int		verbose;
int		count;
uint8_t		packet[MAXPACKET];
int		options;
int		s;			/* Socket file descriptor */
struct hostent	*hp;			/* Pointer to host info */
struct timezone	tz;			/* leftover */

sai_t		whereto;		/* Who to pingpong */
int		datalen;		/* How much data */

char		*hostname;
char		hnamebuf[MAXHOSTNAMELEN];

int		npackets = 1;
int		ntransmitted = 0;	/* sequence number for outbound
					 * packets => amount sent */
int		ident;
int		nwrite;

int		nreceived = 0;		/* # of packets we got back */
int		timing = 0;

void		finish(int);
uint16_t	in_cksum(uint16_t*, int);
int		ck_packet(uint8_t*, size_t, sai_t*);
int		echopkt(int, int);

/*
 * 			M A I N
 */
int
main (int argc, char *argv[])
{
	sai_t from;
	int rc = 0;
	struct protoent *proto;

	tst_resm (TINFO, "Starting pingpong - to send / receive packets from "
			 "host");

	/* Get Host net address */
	tst_resm (TINFO, "Get host net address for sending packets");
	memset( (char *)&whereto, 0, sizeof(sa_t) );
#if INET6
	whereto.sin6_family = AFI;
	whereto.sin6_addr.s6_addr = inet_addr(argv[1]);
	if (whereto.sin6_addr.s6_addr != -1) {
#else
	whereto.sin_family = AFI;
	whereto.sin_addr.s_addr = inet_addr(argv[1]);
	if (whereto.sin_addr.s_addr != -1) {
#endif
		strcpy(hnamebuf, argv[1]);
		hostname = hnamebuf;
	} else {

		if ((hp = gethostbyname(argv[1])) != NULL) {
#if INET6
			whereto.sin6_family = hp->h_addrtype;
			memcpy((caddr_t) &whereto.sin6_addr, hp->h_addr, hp->h_length);
#else
			whereto.sin_family = hp->h_addrtype;
			memcpy((caddr_t) &whereto.sin_addr, hp->h_addr, hp->h_length);
#endif
			hostname = hp->h_name;
		} else {
			tst_resm (TBROK, "%s: unknown host, couldn't get "
					 "address", argv[0]);
			tst_exit();
		}

	}

	/*  Determine Packet Size - either use what was passed in or the default */
	tst_resm (TINFO, "Determining packet size");

	if (argc >= 3)
		datalen = atoi(argv[2]);
		if (datalen < 0) {
			tst_resm (TBROK, "datalen must be an integer.");
			tst_exit();
		}
	else
		datalen = 64;

	datalen -= 8;

	if (datalen > MAXPACKET) {
		tst_resm (TBROK, "packet size too large");
		tst_exit();
	}
	if (datalen >= sizeof(struct timeval))
		timing = 1;

        /* Set number of packets to be sent */
	tst_resm (TINFO, "Determining number of packets to send");
	if (argc >= 4)
		npackets = atoi(argv[3]);

        /* Get PID of current process */
	ident = getpid() & 0xFFFF;

	/* Get network protocol to use (check /etc/protocol) */
	if ((proto = getprotobyname(ICMP_PROTO)) == NULL) {
		tst_resm (TINFO, "unknown protocol: %s", ICMP_PROTO);
		tst_exit();
	}

	/* Create a socket endpoint for communications - returns a descriptor */
	if ((s = socket(AFI, SOCK_RAW, proto->p_proto)) < 0) {
		tst_resm (TINFO, "socket - could not create link");
		tst_exit();
	}

	tst_resm (TINFO, "echoing %s: %d data bytes", hostname, datalen);

	setlinebuf(stdout);

	/* Setup traps */
	signal(SIGINT, finish);
	signal(SIGCLD, finish);

	/* Fork a child process to continue sending packets */
	tst_resm (TINFO, "Create a child process to continue to send packets");
	switch (fork()) {
	case -1:
		tst_resm (TINFO, "ERROR when forking a new process");
		tst_exit();
	case 0:
		/* Child's work */
		ntransmitted = echopkt(datalen, npackets);
		tst_resm (TINFO, "%d packets transmitted", ntransmitted);
		sleep(10);
		break;
	default:

		tst_resm (TINFO, "Parent started - to  receive packets");
		/* Parent's work - receive packets back from child */

		size_t len;

		while (1) {

			len = sizeof (packet);
			size_t cc;
			socklen_t fromlen;

			/* Receive packet from socket */
			tst_resm (TINFO, "Receiving packet");
			if ((cc = recvfrom(s, packet, len, 0, (sa_t*) &from, &fromlen)) < 0) {
				tst_resm (TINFO, "ERROR - recvfrom");
			}
			/* Verify contents of packet */
			if ((rc = ck_packet (packet, cc, &from)) != 0) {
				tst_resm (TINFO, "ERROR - network garbled packet");
			} else {
				nreceived++;
			}

		}

	}

	tst_exit();

}

int
echopkt (int datalen, int npackets)
{
	int count = 0;
	static uint8_t outpack[MAXPACKET];
	register icmp_t *icp = (icmp_t *) outpack;
	int i;
	size_t cc;

	register u_char *datap = &outpack[8];

        /* Setup the packet structure */
        tst_resm (TINFO, "Setting up ICMP packet structure to send to host");

#if INET6
	icp->icmp6_type = IERQ;
	icp->icmp6_code = 0;
	icp->icmp6_id = ident;		/* ID */
#else
	icp->icmp_type = IERQ;
	icp->icmp_code = 0;
	icp->icmp_id = ident;		/* ID */
#endif

	cc = datalen + 8;		/* skips ICMP portion */

	for (i = 8; i < datalen; i++) {	/* skip 8 for time */
		*datap++ = i;
	}

	/* Compute ICMP checksum here */
#if INET6
	icp->icmp6_cksum = in_cksum((uint16_t*) icp, cc);
#else
	icp->icmp_cksum = in_cksum((uint16_t*) icp, cc);
#endif

	/* cc = sendto(s, msg, len, flags, to, tolen) */
	ntransmitted = 0;

	while (count < npackets) {
		count++;
		/* Send packet through socket created */
		tst_resm (TINFO, "Sending packet through created socket");
		i = sendto( s, outpack, cc, 0, (const sa_t*) &whereto, sizeof(whereto) );

		if (i < 0 || i != cc)  {
			if (i < 0)
				perror("sendto");
			tst_resm(TINFO, "wrote %s %d chars, ret=%d",
					hostname, cc, i);
			fflush(stdout);
		}
	}
	/* sleep(30); */
	return(count);

}

/*
 *			I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
uint16_t
in_cksum (uint16_t *addr, int len)
{
	register int nleft = len;
	register uint16_t *w = addr, tmp;
	register int sum = 0;
	register uint16_t answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		tmp = *(u_char *)w;
		sum += (tmp << 8);
	}
	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */

	return answer;

}

/*
 *			F I N I S H
 *
 * Outputs packet information to confirm transmission and reception.
 */
void
finish (int n)
{
	tst_resm (TINFO, "%d packets received", nreceived);
	exit(0);
}

/*
 *			C K _ P A C K E T
 *
 * Checks contents of packet to verify information did not get destroyed
 */

/*
 * buf	- pointer to start of IP header
 * cc	- total size of received packet
 * from - address of sender
 */
int
ck_packet (uint8_t *buf, size_t cc, sai_t *from)
{
	u_char 	i;
	int 	iphdrlen;
	struct 	ip *ip = (struct ip *) buf;	/* pointer to IP header */
	register icmp_t *icp;			/* ptr to ICMP */
  	u_char *datap ;

#if INET6
	from->sin6_addr.s6_addr = ntohl(from->sin6_addr.s6_addr);
#else
	from->sin_addr.s_addr = ntohl(from->sin_addr.s_addr);
#endif

	iphdrlen = ip->ip_hl << 2;		/* Convert # 16-bit words to
						 * number of bytes */
	cc -= iphdrlen;
	icp = (icmp_t*) (buf + iphdrlen);
	datap = (u_char *)icp + sizeof(struct timeval) + 8;
	if (icp->icmp_type != IERP) {
		return 0;			/* Not your packet because it's
						 * not an echo */
	}
	if (icp->icmp_id != ident) {
		return 0;			/* Sent to us by someone
						 * else */
	}

	/* Verify data in packet */
	tst_resm (TINFO, "Verify data in packet after returned from sender");
	if (datalen > 118) {
		datalen = 118;
	}
	tst_resm (TINFO, "Checking Data.");
	for (i = 8; i < datalen; i++) {		/* skip 8 for time */
		if (i !=  (*datap)) {
			tst_resm (TINFO, "Data cannot be validated.");
		}
		datap++;
	}

	return 0;

}