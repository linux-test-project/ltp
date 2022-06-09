// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2001-2022
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
	int s;
	struct in_addr simr, gimr;

	unsigned int i1, i2, i3, i4;
	struct hostent *hp, *gethostbyname();

	unsigned char ttl;
	char no_loop = 0, do_loop = 1;
	unsigned long len = 0;

	if (argc != 2) {
		fprintf(stderr,
			"usage: %s interface_name  (or i.i.i.i)\n", argv[0]);
		exit(1);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror("can't open socket");
		exit(1);
	}

	hp = gethostbyname(argv[1]);
	if (hp)
		memcpy(&simr.s_addr, hp->h_addr, hp->h_length);
	else if (sscanf(argv[1], "%u.%u.%u.%u", &i1, &i2, &i3, &i4) != 4) {
		fprintf(stderr, "Bad interface address\n");
		exit(1);
	} else
		simr.s_addr = htonl((i1 << 24) | (i2 << 16) | (i3 << 8) | i4);

	/* verify socket options error messages */
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &simr,
		       sizeof(simr)) != 0)
		perror("Setting IP_MULTICAST_IF"), exit(1);
	len = sizeof(gimr);
	if (getsockopt
	    (s, IPPROTO_IP, IP_MULTICAST_IF, &gimr, (socklen_t *)&len) != 0)
		perror("Getting IP_MULTICAST_IF"), exit(1);

	len = sizeof(ttl);
	if (getsockopt
	    (s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, (socklen_t *)&len) != 0)
		perror("Getting IP_MULTICAST_TTL"), exit(1);

	ttl = 10;		/* Set ttl to 10 */
/*		printf("setting ttl=10\n");*/
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) != 0)
		perror("Setting IP_MULTICAST_TTL"), exit(1);

	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &do_loop, sizeof(char))
	    != 0)
		perror("Setting IP_MULTICAST_LOOP"), exit(1);
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &no_loop, sizeof(char))
	    != 0)
		perror("Setting IP_MULTICAST_LOOP"), exit(1);
	len = sizeof(no_loop);
	if (getsockopt
	    (s, IPPROTO_IP, IP_MULTICAST_LOOP, &no_loop,
	     (socklen_t *)&len) != 0)
		perror("Getting IP_MULTICAST_LOOP"), exit(1);

	close(s);
	exit(0);
}
