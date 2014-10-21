#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>

#define	MAXBUFSIZ	8096

static char buf[MAXBUFSIZ];

int main(int argc, char *argv[])
{
	struct ip_mreq imr;
	struct sockaddr_in from_in, mcast_in;
	int s = 0, n = 0, one = 1;
	/*DHH 052697 Added for 64BIT Compatibility */
/*	6/2/97	Mario Gallegos changed from ifdef to ifndef (PORTING)	*/
#ifndef __64BIT__
	unsigned int len = 0;
#else
	unsigned long len = 0;
#endif
	unsigned i1, i2, i3, i4, g1, g2, g3, g4;
	struct hostent *hp, *gethostbyname();

	/*int loop=0; */

	if (argc < 4) {
		fprintf(stderr,
			"usage: %s g.g.g.g interface_name (or i.i.i.i) port\n",
			argv[0]);
		exit(1);
	}

	/* set up multicast membership structure */
	if ((n = sscanf(argv[1], "%u.%u.%u.%u", &g1, &g2, &g3, &g4)) != 4) {
		fprintf(stderr, "bad group address\n");
		exit(1);
	}
	imr.imr_multiaddr.s_addr =
	    htonl((g1 << 24) | (g2 << 16) | (g3 << 8) | g4);

	if ((hp = gethostbyname(argv[2])))
		memcpy(&imr.imr_interface.s_addr, hp->h_addr, hp->h_length);
	else if ((n = sscanf(argv[2], "%u.%u.%u.%u", &i1, &i2, &i3, &i4)) != 4) {
		fprintf(stderr, "Bad group interface address\n");
		exit(1);
	} else
		imr.imr_interface.s_addr =
		    htonl((i1 << 24) | (i2 << 16) | (i3 << 8) | i4);

	/* set up socket structure */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("can not open socket");
		exit(1);
	}

	memset(&mcast_in, 0x00, sizeof(mcast_in));
	mcast_in.sin_family = AF_INET;
	mcast_in.sin_port = htons(atoi(argv[3]));

	/* allow address reuse for binding multiple multicast applications to
	   the same group */
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
		perror("setsockopt SO_REUSEADDR failed");
		exit(1);
	}

	if (bind(s, (struct sockaddr *)&mcast_in, sizeof(mcast_in)) < 0) {
		perror("bind");
		exit(1);
	}

	/* Add interface to multicast group */
	if (setsockopt
	    (s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr,
	     sizeof(struct ip_mreq)) < 0) {
		perror("can not join group");
		exit(1);
	}
	printf("Joined group\n");

	/* Receive datagrams */
	len = sizeof(from_in);
	for (;;) {
		memset(&from_in, 0x00, sizeof(from_in));
		if ((n =
		     recvfrom(s, buf, sizeof(buf), 0,
			      (struct sockaddr *)&from_in, &len)) < 0) {
			perror("recvfrom");
			exit(1);
		}
		if (strcmp(buf, "quit") == 0)
			break;
		printf("%s\n", buf);
	}
	close(s);
	exit(0);
}
