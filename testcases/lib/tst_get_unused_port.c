// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 */

#define TST_NO_DEFAULT_MAIN
#include <stdio.h>

#include "tst_safe_net.h"
#include "tst_test.h"

static void help(const char *fname)
{
	printf("usage: %s FAMILY TYPE\n", fname);
	printf("FAMILY := { ipv4 | ipv6 }\n");
	printf("TYPE := { stream | dgram }\n");
}

int main(int argc, char *argv[])
{
	int family = 0, type = 0;
	int opt;

	while ((opt = getopt(argc, argv, ":h")) != -1) {
		switch (opt) {
		case 'h':
			help(argv[0]);
			return 0;
		default:
			help(argv[0]);
			return 1;
		}
	}

	if (argc != 3) {
		help(argv[0]);
		return 1;
	}

	if (!strcmp(argv[1], "ipv4"))
		family = AF_INET;
	else if (!strcmp(argv[1], "ipv6"))
		family = AF_INET6;

	if (!strcmp(argv[2], "stream"))
		type = SOCK_STREAM;
	else if (!strcmp(argv[2], "dgram"))
		type = SOCK_DGRAM;

	if (!family || !type) {
		help(argv[0]);
		return 1;
	}

	printf("%d", ntohs(TST_GET_UNUSED_PORT(family, type)));
	return 0;
}
