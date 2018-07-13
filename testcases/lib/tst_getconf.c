// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Mylène Josserand <mylene.josserand@bootlin.com>
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

static void print_help(void)
{
	printf("Usage: tst_getconf variable\n\n");
	printf("       variable: can be PAGESIZE/PAGE_SIZE");
	printf(" or _NPROCESSORS_ONLN (for the moment)\n\n");
	printf("example: tst_getconf PAGESIZE\n");
}

int main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, ":h")) != -1) {
		switch (opt) {
		case 'h':
			print_help();
			return 0;
		default:
			print_help();
			return 1;
		}
	}

	if (argc != 2) {
		print_help();
		return 1;
	}

	if (!strcmp(argv[optind], "_NPROCESSORS_ONLN")) {
		printf("%ld\n", sysconf(_SC_NPROCESSORS_ONLN));
	} else if (!strcmp(argv[optind], "PAGESIZE") ||
		   !strcmp(argv[optind], "PAGE_SIZE")) {
		printf("%ld\n", sysconf(_SC_PAGE_SIZE));
	} else {
		printf("tst_getconf: Unrecognized variable \'%s\'\n",
		       argv[optind]);
		return -1;
	}

	return 0;
}
