/*
 * Copyright (C) 2007 Casey Schaufler <casey@schaufler-ca.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, version 2.
 *
 * Author:
 *	Casey Schaufler <casey@schaufler-ca.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#ifdef HAVE_LINUX_NETLINK_H
#include <linux/netlink.h>
#endif

#include <sys/xattr.h>

int
main(int argc, char *argv[])
{
	char message[256];
	char *anin = "security.SMACK64IPIN";
	char *anout = "security.SMACK64IPOUT";
	char *annot = "security.SMACK64IPNOT";
	char *avin = "TheOne";
	char *avout = "TheOther";
	char *avnot = "TheBadValue";
	int sock;
	int rc;

	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		sprintf(message, "%s Socket creation failure", argv[0]);
		perror(message);
		exit(1);
	}

	rc = fsetxattr(sock, anin, avin, strlen(avin) + 1, 0);
	if (rc < 0) {
		sprintf(message, "%s fsetxattr of %s to %s failure",
			argv[0], anin, avin);
		perror(message);
		exit(1);
	}

	rc = fsetxattr(sock, anout, avout, strlen(avout) + 1, 0);
	if (rc < 0) {
		sprintf(message, "%s fsetxattr of %s to %s failure",
			argv[0], anout, avout);
		perror(message);
		exit(1);
	}

	rc = fsetxattr(sock, annot, avnot, strlen(avnot) + 1, 0);
	if (rc >= 0) {
		sprintf(message, "%s fsetxattr of %s to %s succeeded in error",
			argv[0], anout, avout);
		perror(message);
		exit(1);
	}
	exit(0);
}