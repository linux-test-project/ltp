#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <netdb.h>

#define	MAXBUFSIZ	8096

static char buf[MAXBUFSIZ];
static int Num_Loops = 100;

int main(int argc, char *argv[])
{
	struct ip_mreq imr;
	struct sockaddr_in sin, mcast_out;
	int i = 0, s = 0, n = 0;
	unsigned i1, i2, i3, i4, g1, g2, g3, g4;
	struct hostent *hp, *gethostbyname();
	char myname[64];
	char ttl = 0;

	if (argc < 4) {
		fprintf(stderr,
			"usage: %s g.g.g.g interface_name (or i.i.i.i) port [ttl]\n",
			argv[0]);
		exit(1);
	}

	/* Get local host name */
	if (gethostname(myname, sizeof(myname) - 1) < 0) {
		perror("gethostname");
		exit(1);
	}

	/* set up multicast membership structure */
	if ((n = sscanf(argv[1], "%u.%u.%u.%u", &g1, &g2, &g3, &g4)) != 4) {
		fprintf(stderr, "bad group address\n");
		exit(1);
	}
	imr.imr_multiaddr.s_addr =
	    htonl((g1 << 24) | (g2 << 16) | (g3 << 8) | g4);

	if ((hp = gethostbyname(argv[2]))) {
		memcpy(&imr.imr_interface.s_addr, hp->h_addr, hp->h_length);
	} else
	    if ((n = sscanf(argv[2], "%u.%u.%u.%u", &i1, &i2, &i3, &i4)) != 4) {
		fprintf(stderr, "Bad interface address\n");
		exit(1);
	} else
		imr.imr_interface.s_addr =
		    htonl((i1 << 24) | (i2 << 16) | (i3 << 8) | i4);

	/* Set up socket structure for sendto */
	memset(&mcast_out, 0x00, sizeof(mcast_out));
	memset(&sin, 0x00, sizeof(sin));
	mcast_out.sin_family = sin.sin_family = AF_INET;
	mcast_out.sin_port = sin.sin_port = htons(atoi(argv[3]));

	mcast_out.sin_addr.s_addr = imr.imr_multiaddr.s_addr;

	/* Create socket */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("can not open socket");
		exit(1);
	}

	/* Set socket for multicasting */
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF,
		       &imr.imr_interface.s_addr,
		       sizeof(imr.imr_interface.s_addr)) != 0) {
		fprintf(stderr,
			"Error: unable to set socket option IP_MULTICAST_IF\n");
		exit(1);
	}

	/* With an even port number the loopback will be disabled */
/*      loop = atoi(argv[3])&1;
        if (setsockopt(s,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(char))!= 0) {
           fprintf (stderr,
                    "Error: unable to set socket option IP_MULTICAST_LOOP\n");
           exit (1);
        }
*/
	ttl = atoi(argv[4]);
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("can not set ttl");
		exit(1);
	}

	/* Send datagrams */
	for (i = 1; i < Num_Loops; i++) {
		sprintf(buf, "%s %d %lld", argv[2], i, (long long int)time(0));
		if ((n =
		     sendto(s, buf, sizeof(buf), 0,
			    (struct sockaddr *)&mcast_out,
			    sizeof(mcast_out))) < 0) {
			perror("setsockopt");
			exit(1);
		}
		sleep(1);
	}

	/* Tell recevier to close */
	sprintf(buf, "quit");
	if ((n = sendto(s, buf, sizeof(buf), 0, (struct sockaddr *)&mcast_out,
			sizeof(mcast_out))) < 0) {
		perror("setsockopt");
		exit(1);
	}

	close(s);
	exit(0);
}
