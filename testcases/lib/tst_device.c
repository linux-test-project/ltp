/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "old/old_device.h"

extern struct tst_test *tst_test;

static struct tst_test test = {
	.tid = "tst_device"
};

static void print_help(void)
{
	printf("\nUsage: tst_device acquire [size]\n");
	printf("   or: tst_device release /path/to/device\n\n");
}

static int acquire_device(int argc, char *argv[])
{
	unsigned int size = 0;
	const char *device;

	if (argc > 3)
		return 1;

	if (argc == 3) {
		size = atoi(argv[2]);

		if (!size) {
			fprintf(stderr, "ERROR: Invalid device size '%s'",
				argv[2]);
			return 1;
		}
	}

	device = tst_acquire_device__(size);

	if (!device)
		return 1;

	printf("%s", device);

	return 0;
}

static int release_device(int argc, char *argv[])
{
	if (argc != 3)
		return 1;

	return tst_release_device(argv[2]);
}

int main(int argc, char *argv[])
{
	/*
	 * Force messages to be printed from the new library i.e. tst_test.c
	 *
	 * The new library prints messages into stderr while the old one prints
	 * them into stdout. When messages are printed into stderr we can
	 * safely do:
	 *
	 * DEV=$(tst_device acquire)
	 */
	tst_test = &test;

	if (argc < 2)
		goto help;

	if (!strcmp(argv[1], "acquire")) {
		if (acquire_device(argc, argv))
			goto help;
	} else if (!strcmp(argv[1], "release")) {
		if (release_device(argc, argv))
			goto help;
	} else {
		fprintf(stderr, "ERROR: Invalid COMMAND '%s'\n", argv[1]);
		goto help;
	}

	return 0;
help:
	print_help();
	return 1;
}
