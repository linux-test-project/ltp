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
#include <string.h>
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
int s;			/* Socket file descriptor */
struct addrinfo *hp;	/* Pointer to host info */
struct addrinfo hints;

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
                fprintf(stderr, "Unknown subject address %s: %s\n",av[1], gai_strerror(gai));
        if (!hp->ai_addr || hp->ai_addr->sa_family != AF_INET6)
                fprintf(stderr, "getaddrinfo failed");
	strcpy(hnamebuf, av[1]);
        hostname = hnamebuf;
	memset( (char *)&whereto, 0x00, sizeof(struct sockaddr) );
	memcpy(&whereto, hp->ai_addr, hp->ai_addrlen);
        
	/*  Determine Packet Size - either use what was passed in or default */
        printf ("Determine packet size \n");
	if( argc >= 3 )
		datalen = atoi( av[2] ) - 8;  
	else
		datalen = 64-8;
	if (datalen > MAXPACKET) {
		printf("Pingpong: packet size too large\n");
		exit(1);
	}


        /* Set number of packets to be sent */
        printf ("Determine number of packets to send \n");
	if (argc >= 4)
		npackets = atoi(av[3]);


        /* Get PID of current process */
	ident = getpid() & 0xFFFF;


        /* Get network protocol to use (check /etc/protocol) */
	if ((proto = getprotobyname("ipv6-icmp")) == NULL) {
		printf("ICMP6: unknown protocol\n");
		exit(1);
	}

 
        /* Create a socket endpoint for communications - returns a descriptor */
	if ((s = socket(AF_INET6, SOCK_RAW, proto->p_proto)) < 0) {
		printf("Pingpong: socket - could not create link \n");
		exit(1);
	}

	printf("echoing %s: %d data bytes\n", hostname, datalen );
	printf("Total packet size is %d bytes\n",datalen+8);	

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
			fromlen = sizeof (from);
			if ( (cc=recvfrom(s, packet, len, 0, (struct sockaddr *)&from, &fromlen)) < 0) {
				printf("ERROR in recvfrom\n");
			}
                        /* Verify contents of packet */
                        if ((rc = ck_packet (packet, cc, &from)) == 0) 
				nreceived++;
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

	register u_char *datap = &outpack[8];


        /* Setup the packet structure */
        printf ("Setup ICMP packet structure to send to host \n");
	icp->icmp6_type = ICMP6_ECHO_REQUEST;
	icp->icmp6_code = 0;
	icp->icmp6_cksum = 0;
	icp->icmp6_id = ident;		/* ID */
	
	cc = datalen+8;			/* skips ICMP portion */

	for( i=0; i<datalen; i++) {	
		*datap++ = 6;
	}
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
u_char 	*buf;			/* pointer to start of IP header */
int	cc;			/* total size of received packet */
struct sockaddr_in6 *from; 	/* address of sender */
{
	int 	i;
	struct  icmp6_hdr icp_hdr;
	struct	icmp6_hdr *icp = (struct ip6_hdr *) buf;          /* pointer to IP header */
  	u_char *datap ;
	

	datap = (u_char *)icp + sizeof(icp_hdr);
	if (icp->icmp6_type != ICMP6_ECHO_REPLY) {
		return(1);	 /* Not your packet 'cause not an echo */
	}
	if (icp->icmp6_id != ident) {
		return(1);			/* Sent to us by someone else */
	}	
	printf("Receiving packet \n");
        /* Verify data in packet */

	printf ("Checking Data.\n");
        for( i=0; i<datalen; i++) {     
          if ( (*datap) != 6 ) {               
                 printf ("RVW: Data in [%d] is %d\n",i,(*datap));
		 printf ("Data cannot be validated. \n");
          }
          datap++;
	}
	return(0);
}
