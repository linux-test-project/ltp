// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Red Hat Inc.
 * Author: Chunyu Hu <chuhu@redhat.com>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

static void print_help(void)
{
	printf("Usage: tst_random <value1> [value2]\n");
	printf("       Generated random will be between value1 and value2.\n");
	printf("       If only value1 is specified, value2 will treated as 0.\n");
}

static int get_seed(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec;
}

static long rand_range(long min, long max)
{
	return random() % (max - min + 1) + min;
}

int main(int argc, char *argv[])
{
	int opt;
	long min = 0, max = 0;
	long rval = 0;
	char *end;

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

	if (argc != 2 && argc != 3) {
		print_help();
		return 1;
	}

	max = strtol(argv[1], &end, 10);
	if (argv[1][0] == '\0' || *end != '\0') {
		fprintf(stderr, "ERROR: Invalid range value1 '%s'\n\n",
			argv[optind]);
		print_help();
		return 1;
	}

	if (argc == 3) {
		min = strtol(argv[2], &end, 10);
		if (argv[2][0] == '\0' || *end != '\0') {
			fprintf(stderr, "ERROR: Invalid range value2 '%s'\n\n",
				argv[optind+1]);
			print_help();
			return 1;
		}
	}

	srandom(get_seed());
	rval = (min > max) ? rand_range(max, min) : rand_range(min, max);
	printf("%ld\n", rval);

	return 0;
}
