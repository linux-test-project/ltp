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
#include <sys/vfs.h>

#include "test.h"

char *TCID = "smack_set_socket_labels";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);
static void set_socket_labels(char **);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		set_socket_labels(argv);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void set_socket_labels(char **argv)
{
	char *anin = "security.SMACK64IPIN";
	char *anout = "security.SMACK64IPOUT";
	char *annot = "security.SMACK64IPNOT";
	char *avin = "TheOne";
	char *avout = "TheOther";
	char *avnot = "TheBadValue";
	int sock;
	int rc;
	char buf[256];

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0)
		tst_brkm(TFAIL, NULL, "%s Socket creation failure", argv[0]);

	flistxattr(sock, buf, 256);
	if (strstr(buf, "security.SMACK64") == NULL)
		tst_brkm(TCONF, NULL, "smackfs not set.");

	rc = fsetxattr(sock, anin, avin, strlen(avin) + 1, 0);
	if (rc < 0) {
		tst_brkm(TFAIL, NULL, "%s fsetxattr of %s to %s failure",
			 argv[0], anin, avin);
	}

	rc = fsetxattr(sock, anout, avout, strlen(avout) + 1, 0);
	if (rc < 0) {
		tst_brkm(TFAIL, NULL, "%s fsetxattr of %s to %s failure",
			 argv[0], anout, avout);
	}

	rc = fsetxattr(sock, annot, avnot, strlen(avnot) + 1, 0);
	if (rc >= 0) {
		tst_brkm(TFAIL, NULL,
			 "%s fsetxattr of %s to %s succeeded in error",
			 argv[0], anout, avout);
	}

	tst_resm(TPASS, "Test %s success.", TCID);
}

static void cleanup(void)
{
}
