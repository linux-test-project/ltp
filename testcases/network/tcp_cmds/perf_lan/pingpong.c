/*
#
#
#                              Task Subprogram 
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
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include "test.h"
#include "usctest.h"


#define	MAXPACKET	4096	/* max packet size */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	64
#endif

int	pfd[2];
int	fdstr[10];
int	verbose;
int	count;
u_char	packet[MAXPACKET];
int	options;
int s;			/* Socket file descriptor */
struct hostent *hp;	/* Pointer to host info */
struct timezone tz;	/* leftover */

struct sockaddr whereto;/* Who to pingpong */
int datalen;		/* How much data */


char *hostname;
char hnamebuf[MAXHOSTNAMELEN];

int npackets=1;
int ntransmitted = 0;		/* sequence # for outbound packets = #sent */
int ident;

int nreceived = 0;		/* # of packets we got back */
int timing = 0;
void finish(int); 
int nwrite;
char *TCID = "perf_lan";
int TST_TOTAL = 1;
extern int Tst_count;

u_short in_cksum(u_short *, int);
int ck_packet(char *, int, struct sockaddr_in *);
int echopkt(int, int);


/*
 * 			M A I N
 */
int main(argc, argv)
char *argv[];
{
	struct sockaddr_in from;
	char **av = argv;
	struct sockaddr_in *to = (struct sockaddr_in *) &whereto;
	int rc = 0;
	struct protoent *proto;

	tst_resm (TINFO, "Starting pingpong - to send / receive packets from host \n");

        /* Get Host net address */
        tst_resm (TINFO, "Get host net address for sending packets \n");
	memset( (char *)&whereto, 0x00, sizeof(struct sockaddr) );
	to->sin_family = AF_INET;
	to->sin_addr.s_addr = inet_addr(av[1]);
	if (to->sin_addr.s_addr != -1) {
		strcpy(hnamebuf, av[1]);
		hostname = hnamebuf;
	} else {
		hp = gethostbyname(av[1]);
		if (hp) {
			to->sin_family = hp->h_addrtype;
			memcpy((caddr_t)&to->sin_addr, hp->h_addr, hp->h_length);
			hostname = hp->h_name;
		} else {
            tst_resm (TINFO, "%s: unknown host, couldn't get address\n",argv[0]);
			exit(1);
		}
	}



    /*  Determine Packet Size - either use what was passed in or default */
        tst_resm (TINFO, "Determine packet size \n");
	if( argc >= 3 )
		datalen = atoi( av[2] );
	else
		datalen = 64-8;
	if (datalen > MAXPACKET) {
		tst_resm (TINFO, "Pingpong: packet size too large\n");
		exit(1);
	}
	if (datalen >= sizeof(struct timeval))
		timing = 1;


        /* Set number of packets to be sent */
        tst_resm (TINFO, "Determine number of packets to send \n");
	if (argc >= 4)
		npackets = atoi(av[3]);


        /* Get PID of current process */
	ident = getpid() & 0xFFFF;


        /* Get network protocol to use (check /etc/protocol) */
	if ((proto = getprotobyname("icmp")) == NULL) {
		tst_resm (TINFO, "ICMP: unknown protocol\n");
		exit(10);
	}

 
        /* Create a socket endpoint for communications - returns a descriptor */
	if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0) {
		tst_resm (TINFO, "Pingpong: socket - could not create link \n");
		exit(5);
	}

	tst_resm (TINFO, "echoing %s: %d data bytes\n", hostname, datalen );

	setlinebuf( stdout );

        /* Setup traps */
	signal( SIGINT, finish );
	signal( SIGCLD, finish );

     
        /* Fork a child process to continue sending packets */
        tst_resm (TINFO, "Create a child process to continue to send packets \n");
	switch (fork()) {
	case -1:
		tst_resm (TINFO, "ERROR when forking a new process\n");
		exit(1);
	case 0:
                /* Child's work */
		ntransmitted=echopkt(datalen,npackets);
		tst_resm (TINFO, "%d packets transmitted, ",ntransmitted);
		sleep(10);
		break;
	default:
                tst_resm (TINFO, "Parent started - to  receive packets \n");
                /* Parent's work - receive packets back from child */
		for (;;) {
			int len = sizeof (packet);
			unsigned int fromlen = sizeof (from);
#ifdef __64BIT__
                        long cc;
#else
			int cc;
#endif

			/* Receive packet from socket */
			tst_resm (TINFO, "Receiving packet \n");
			fromlen = sizeof (from);
			if ( (cc=recvfrom(s, packet, len, 0, (struct sockaddr *)&from, &fromlen)) < 0) {
				tst_resm (TINFO, "ERROR in recvfrom\n");
			}
                        /* Verify contents of packet */
                        if ((rc = ck_packet (packet, cc, &from)) != 0) {
                                tst_resm (TINFO, "ERROR - network garbled packet\n");
			}
                        else {
				nreceived++;
			} 
		}
	}
	return 0;
}

int echopkt(datalen,npackets)
int datalen;
int npackets;
{
	int count=0;
	static u_char outpack[MAXPACKET];
	register struct icmp *icp = (struct icmp *) outpack;
        int i;
#ifdef __64BIT__
                        long cc;
#else
                        int cc;
#endif

	register struct timeval *tp = (struct timeval *) &outpack[8];
	register u_char *datap = &outpack[8+sizeof(struct timeval)];


        /* Setup the packet structure */
        tst_resm (TINFO, "Setup ICMP packet structure to send to host \n");
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_id = ident;		/* ID */

	cc = datalen+8;			/* skips ICMP portion */


        /* Add time stamp and user data */
        tst_resm (TINFO, "Add time stamp, user data, and check sum to packet. \n");
	if (timing)
		gettimeofday( tp, &tz );

	for( i=8; i<datalen; i++) {	/* skip 8 for time */
		*datap++ = i;
	}

	/* Compute ICMP checksum here */
	icp->icmp_cksum = in_cksum( (ushort *)icp, cc );

	/* cc = sendto(s, msg, len, flags, to, tolen) */
	ntransmitted=0;
	while (count < npackets) {
		count++;
                /* Send packet through socket created */
                tst_resm (TINFO, "Sending packet through created socket \n");
		i = sendto( s, outpack, cc, 0, &whereto, sizeof(struct sockaddr) );

		if( i < 0 || i != cc )  {
			if( i<0 )  perror("sendto");
			tst_resm (TINFO, "pingpong: wrote %s %d chars, ret=%d\n",hostname,cc,i);
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
u_short in_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr, tmp;
    register int sum = 0;
    register u_short answer = 0;

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
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    return(answer);
}


/*
 * 			T V S U B
 * 
 * Subtract 2 timeval structs:  out = out - in.
 * 
 * Out is assumed to be >= in.
 *tvsub( out, in )
 *register struct timeval *out, *in;
 *{
 *	if( (out->tv_usec -= in->tv_usec) < 0 )   {
 *		out->tv_sec--;
 *		out->tv_usec += 1000000;
 *	}
 *	out->tv_sec -= in->tv_sec;
 *}
 */

/*
 *			F I N I S H      
 *
 * Outputs packet information to confirm transmission and reception.
 */
void finish(int n)
{
	tst_resm (TINFO, "%d packets received, \n", nreceived );
	exit(0);
}


/*
 *			C K _ P A C K E T
 *
 * Checks contents of packet to verify information did not get destroyed
 */

int ck_packet (buf, cc, from)
char 	*buf;			/* pointer to start of IP header */
int	cc;			/* total size of received packet */
struct sockaddr_in *from; 	/* address of sender */
{
	u_char 	i;
	int 	iphdrlen;
	struct 	ip *ip = (struct ip *) buf;          /* pointer to IP header */
	register struct	icmp *icp;                   /* ptr to ICMP */
  	u_char *datap ;
	
	from->sin_addr.s_addr = ntohl(from->sin_addr.s_addr);

	iphdrlen = ip->ip_hl << 2;  /* Convert # 16-bit words to #bytes */
	cc -= iphdrlen; 
        icp = (struct icmp *) (buf + iphdrlen);
	datap = (u_char *)icp + sizeof(struct timeval) + 8;
	if (icp->icmp_type != ICMP_ECHOREPLY) {
		return(0);	 /* Not your packet 'cause not an echo */
	}
	if (icp->icmp_id != ident) {
		return(0);			/* Sent to us by someone else */
	}	

        /* Verify data in packet */
        tst_resm (TINFO, "Verify data in packet after returned from sender \n");
	if ( datalen > 118 ) {
		datalen=118;
	}
	tst_resm (TINFO, "Checking Data.\n");
        for( i=8; i<datalen; i++) {             /* skip 8 for time */
                if ( i !=  (*datap)) {               
                        tst_resm (TINFO, "Data cannot be validated. \n");
                }
                datap++; 
        }
	return(0);
}
