// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2001-2022
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>

#define	MAXBUFSIZ	8096

static int errors;

int main(int argc, char *argv[])
{
	int s;
	struct in_addr gimr;
	struct ip_mreq simr;

	unsigned int i1, i2, i3, i4;
	struct hostent *hp, *gethostbyname();

	char sintf[20], gintf[20] = {0};
	unsigned char ttl;
	char loop = 0;
	unsigned int len = 0;

	if (argc != 2) {
		fprintf(stderr,
			"usage: %s interface_name (or i.i.i.i)\n", argv[0]);
		exit(1);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror("Error: can't open socket");
		exit(1);
	}

	printf("agrv sub 1 is %s\n", argv[1]);
	hp = gethostbyname(argv[1]);
	if (hp)
		memcpy(&simr.imr_interface.s_addr, hp->h_addr, hp->h_length);
	else if (sscanf(argv[1], "%u.%u.%u.%u", &i1, &i2, &i3, &i4) != 4) {
		fprintf(stderr, "Bad interface address\n");
		exit(1);
	} else
		simr.imr_interface.s_addr =
		    htonl((i1 << 24) | (i2 << 16) | (i3 << 8) | i4);
	strcpy(sintf, inet_ntoa(simr.imr_interface));

	/* verify socket options */
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF,
		       &simr.imr_interface.s_addr,
		       sizeof(simr.imr_interface.s_addr)) != 0) {
		perror("Error: Setting IP_MULTICAST_IF");
		errors++;
	} else
		printf("Set interface: %s for multicasting\n", sintf);

	len = sizeof(gimr);
	if (getsockopt
	    (s, IPPROTO_IP, IP_MULTICAST_IF, &gimr, (socklen_t *)&len) != 0) {
		perror("Getting IP_MULTICAST_IF");
		errors++;
	} else {
		strcpy(gintf, inet_ntoa(gimr));
		printf("Got multicasting socket interface: %s\n", gintf);
	}

	/* Verify that the multicastion for the interface was set */
	if (strcmp(sintf, gintf) != 0) {
		printf("Error: IP_MULTICAST_IF was not set\n");
		errors++;
	} else
		printf
		    ("Socket has been set for multicasting on interface: %s\n",
		     sintf);

	len = sizeof(ttl);
	if (getsockopt
	    (s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, (socklen_t *)&len) != 0) {
		perror("Error: Gettting IP_MULTICAST_TTL");
		errors++;
	} else
		printf("getsockopt: got ttl = %i\n", ttl);
	if (ttl != 1)
		printf("Error: IP_MULTICAST_TTL not default value, ttl = %i\n",
		       ttl);
	ttl = 10;		/* Set ttl to 10 */
	len = sizeof(ttl);
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL,
		       &ttl, sizeof(ttl)) != 0) {
		perror("Error: Setting IP_MULTICAST_TTL");
		errors++;
	} else
		printf("TTL set on multicast socket\n");
	if (getsockopt
	    (s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, (socklen_t *)&len) != 0) {
		perror("Error: Getting IP_MULTICAST_TTL");
		errors++;
	}
	if (ttl != 10) {
		printf("Error: IP_MULTICAST_TTL not set, ttl = %i\n", ttl);
		errors++;
	}

	len = sizeof(loop);
	if (getsockopt
	    (s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop,
	     (socklen_t *)&len) != 0) {
		perror("Error: Getting IP_MULTICAST_LOOP");
		errors++;
	} else
		printf("Got loopback setting\n");
	if (loop != 1) {
		printf("Error: IP_MULTICAST_LOOP not enabled, loop = %i\n",
		       loop);
		errors++;
	} else
		printf("IP_MULTICAST_LOOP is enabled\n");

	loop = 0;		/* Disable IP_MULTICAST_LOOP */
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(char)) !=
	    0) {
		errors++;
		perror("Error: Setting IP_MULTICAST_LOOP");
	} else
		printf("Multicast loopback disabled\n");
	if (getsockopt
	    (s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop,
	     (socklen_t *)&len) != 0) {
		perror("Error: Getting IP_MULTICAST_LOOP");
		errors++;
	} else
		printf("Got multicast loopback value\n");
	if (loop != 0) {
		printf("Error: IP_MULTICAST_LOOP not disabled, loop = %i\n",
		       loop);
		errors++;
	} else
		printf("IP_MULTICAST_LOOP disabled\n");

	close(s);
	if (errors)
		exit(1);
	exit(0);
}
