/*
#
#
#                              Task Subprogram 
#
#  SUBPROGRAM NAME: PINGPONG6.C
#
#  REQUIRED PARAMETERS:
#    Calling Procedure: pingpong6 HOST SIZE PACKETS
#       HOST - Current host
#       SIZE  - Size of each packet 
#       PACKETS  - the number of packets across the network
#
#  SETUP REQUIRED:
#       o  This task must be run as root.
#       o  TCP/IP must be configured before executing this task.
#
#  DESCRIPTION:
# 	Purpose:To generate lan traffic with ICMP6 echo packet
# 	Command: None
# 	Subcommand:None
# 	Design:
#		Create raw socket
#		spawn child process to send echo ICMP6 packet
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
#include <sys/time.h>
#include <sys/types.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/signal.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netdb.h>

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
extern	int errno;

int s;			/* Socket file descriptor */
struct addrinfo *hp;	/* Pointer to host info */
struct addrinfo hints;
struct timezone tz;	/* leftover */

struct sockaddr_in6 whereto;/* Who to pingpong */
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
/*
 * 			M A I N
 */
main(argc, argv)
char *argv[];
{
	struct sockaddr_in6 from;
	char **av = argv;
	int nrcv;
	int on = 1;
	int rc = 0;
	struct protoent *proto;
	int gai;

	printf ("Starting pingpong - to send / receive packets from host \n");

        /* Get Host net address */
        printf ("Get host net address for sending packets \n");
	memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_INET6;

        if ((gai=getaddrinfo(av[1], NULL, &hints, &hp))!=0)
                errx(2, "Unknown subject address %s: %s\n",av[1], gai_strerror(gai));
        if (!hp->ai_addr || hp->ai_addr->sa_family != AF_INET6)
                errx(2, "getaddrinfo failed");
	strcpy(hnamebuf, av[1]);
        hostname = hnamebuf;
	bzero( (char *)&whereto, sizeof(struct sockaddr) );
	memcpy(&whereto, hp->ai_addr, hp->ai_addrlen);
        
	/*  Determine Packet Size - either use what was passed in or default */
        printf ("Determine packet size \n");
	if( argc >= 3 )
		datalen = atoi( av[2] );
	else
		datalen = 64-8;
	if (datalen > MAXPACKET) {
		printf("Pingpong: packet size too large\n");
		exit(1);
	}
	if (datalen >= sizeof(struct timeval))
		timing = 1;


        /* Set number of packets to be sent */
        printf ("Determine number of packets to send \n");
	if (argc >= 4)
		npackets = atoi(av[3]);


        /* Get PID of current process */
	ident = getpid() & 0xFFFF;


        /* Get network protocol to use (check /etc/protocol) */
	if ((proto = getprotobyname("ipv6-icmp")) == NULL) {
		printf("ICMP: unknown protocol\n");
		exit(1);
	}

 
        /* Create a socket endpoint for communications - returns a descriptor */
	if ((s = socket(AF_INET6, SOCK_RAW, proto->p_proto)) < 0) {
		printf("Pingpong: socket - could not create link \n");
		exit(1);
	}

	printf("echoing %s: %d data bytes\n", hostname, datalen );

	setlinebuf( stdout );

        /* Setup traps */
	signal( SIGINT, finish );
	signal( SIGCLD, finish );

     
        /* Fork a child process to continue sending packets */
        printf ("Create a child process to continue to send packets \n");
	switch (fork()) {
	case -1:
		printf("ERROR when forking a new process\n");
		exit(1);
	case 0:
                /* Child's work */
		ntransmitted=echopkt(datalen,npackets);
		printf("%d packets transmitted, ",ntransmitted);
		sleep(10);
		break;
	default:
                printf ("Parent started - to  receive packets \n");
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
			printf("Receiving packet \n");
			fromlen = sizeof (from);
			if ( (cc=recvfrom(s, packet, len, 0, (struct sockaddr *)&from, &fromlen)) < 0) {
				printf("ERROR in recvfrom\n");
			}
                        /* Verify contents of packet */
                        if ((rc = ck_packet (packet, cc, &from)) != 0) {
                                printf("ERROR - network garbled packet\n");
			}
                        else {
				nreceived++;
			} 
		}
	}
}

echopkt(datalen,npackets)
int datalen;
int npackets;
{
	int count=0;
	static u_char outpack[MAXPACKET];
	register struct icmp6_hdr *icp = (struct icmp6_hdr *) outpack;
        int i;
#ifdef __64BIT__
                        long cc;
#else
                        int cc;
#endif

	register struct timeval *tp = (struct timeval *) &outpack[8];
	register u_char *datap = &outpack[8+sizeof(struct timeval)];


        /* Setup the packet structure */
        printf ("Setup ICMP packet structure to send to host \n");
	icp->icmp6_type = ICMP6_ECHO_REQUEST;
	icp->icmp6_code = 0;
	icp->icmp6_cksum = 0;
	icp->icmp6_id = ident;		/* ID */

	cc = datalen+8;			/* skips ICMP portion */


        /* Add time stamp and user data */
        printf ("Add time stamp, user data, and check sum to packet. \n");
	if (timing)
		gettimeofday( tp, &tz );

	for( i=8; i<datalen; i++) {	/* skip 8 for time */
		*datap++ = i;
	}

	/* Compute ICMP checksum here */
	icp->icmp6_cksum = in6_cksum( icp, cc );

	ntransmitted=0;
	while (count < npackets) {
		count++;
                /* Send packet through socket created */
                printf ("Sending packet through created socket \n");
		i = sendto( s, outpack, cc, 0, &whereto, sizeof(whereto) );

		if( i < 0 || i != cc )  {
			if( i<0 )  perror("sendto");
			printf("pingpong6: wrote %s %d chars, ret=%d\n",hostname,cc,i);
			fflush(stdout);
			}
		}
		/* sleep(30); */
		return(count);
}



/*
 *			I N 6_ C K S U M
 *
 * Checksum routine for Internet Protocol version 6 family headers (C Version)
 *
 */
in6_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr, tmp;
    register int sum = 0;
    register u_long answer = 0;

    /*
     * Our algorithm is simple, using a 128 bit accumulator (sum), we add
     * sequential 16 bit words to it, and at the end, fold back all the
     * carry bits from the top 64 bits into the lower 64 bits.
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
    /* add back carry outs from top 64 bits to low 64 bits */
    sum = (sum >> 64) + (sum & 0xffffffffffffffff); /* add hi 64 to low 64 */
    sum += (sum >> 64);         /* add carry */
    answer = ~sum;              /* truncate to 64 bits */
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
	printf("%d packets received, \n", nreceived );
	exit(0);
}


/*
 *			C K _ P A C K E T
 *
 * Checks contents of packet to verify information did not get destroyed
 */

ck_packet (buf, cc, from)
char 	*buf;			/* pointer to start of IP header */
int	cc;			/* total size of received packet */
struct sockaddr_in6 *from; 	/* address of sender */
{
	u_char 	i;
	int 	iphdrlen;
	struct 	ip6_hdr *ip6 = (struct ip6_hdr *) buf;          /* pointer to IP header */
	register struct	icmp6_hdr *icp;                   /* ptr to ICMP */
	struct timeval *tp = (struct timeval *) &buf[8];
  	u_char *datap ;
	
	/*from->sin6_addr.s6_addr32= ntohl(from->sin6_addr.s6_addr32);*/

	cc = ip6->ip6_plen << 2; /* Convert # 16-bit words to #bytes */ 
        icp = (struct icmp6 *) (buf + iphdrlen);
	datap = (u_char *)icp + sizeof(struct timeval) + 8;
	if (icp->icmp6_type != ICMP6_ECHO_REPLY) {
		return(0);	 /* Not your packet 'cause not an echo */
	}
	if (icp->icmp6_id != ident) {
		return(0);			/* Sent to us by someone else */
	}	

        /* Verify data in packet */
        printf ("Verify data in packet after returned from sender \n");
	if ( datalen > 118 ) {
		datalen=118;
	}
	printf ("Checking Data.\n");
        for( i=8; i<datalen; i++) {             /* skip 8 for time */
                if ( i !=  (*datap)) {               
                        printf ("Data cannot be validated. \n");
                }
                datap++; 
        }
	return(0);
}
